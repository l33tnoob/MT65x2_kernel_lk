/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.voicecommand.adapter;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Arrays;

import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.IMessageDispatcher;
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.service.VoiceCommandManagerStub;

public class JNICommandAdapter implements IVoiceAdapter {

    private int mNativeContext = 0;

    /*
     * Do not change these values without updating their counterparts!
     */
    // The mode currently supported
    private static final int MODE_VOICE_UNKNOW = -1;
    private static final int NATIVE_MODE_VOICE_UI = 1;
    private static final int NATIVE_MODE_VOICE_TRAINING = 2;
    private static final int NATIVE_MODE_VOICE_RECOGNITION = 4;
    private static final int NATIVE_MODE_VOICE_CONTACTS = 8;

    // The return message notified from native counterpart @{
    private static final int NOTIFY_VOICE_ERROR = -1;
    private static final int NOTIFY_VOICE_UI = 0;
    private static final int NOTIFY_VOICE_TRAINING = 1;
    private static final int NOTIFY_VOICE_RECOGNITION = 2;
    private static final int NOTIFY_VOICE_CONTACTS = 3;
    // voice_mode to native counterpart @{
    private static final int VOICE_NORMAL_MODE = 1;
    private static final int VOICE_HEADSET_MODE = 2;
    // @}

    // The detail error when native counterpart notify returns for error @{
    // private final int NOTIFY_ERROR_UI = 0;
    // private final int NOTIFY_ERROR_VOICE_TRAINING = 1;
    // private final int NOTIFY_ERROR_VOICE_RECOGNITION = 2;
    // @}

    // The detail information when native counterpart notify returns for voice
    // training
    private static final int NOTIFY_VOICE_TRAINING_FINISHED = 0;
    // private final int NOTIFY_VOICE_TRAINING_NEED_MORE_VOICE = 1;
    // private final int NOTIFY_VOICE_TRAINING_TOO_NOISY = 2;
    // private final int NOTIFY_VOICE_TRAINING_DIFF_PSWD = 3;
    private static final int NOTIFY_VOICE_TRAINING_EXIST_PSWD = 5;
    private static final int NOTIFY_VOICE_TRAINING_TIMEOUT = 6;
    private static final int NOTIFY_VOICE_TRAINING_OK_CONFIDENCE = 100;
    private static final int NOTIFY_VOICE_HEADSET_PLUG = 100;

    private static final int NOTIFY_VOICE_CONTACTS_COMMANDARRAY = 0;
    private static final int NOTIFY_VOICE_CONTACTS_SPEECHDETECTED = 1;

    private IMessageDispatcher mUpDispatcher;

    private FileOutputStream mPatternFileStream;
    private FileOutputStream mPasswordFileStream;
    private FileOutputStream mFeatureFileStream;

    private String mPatternFilePath;
    private String mPasswordFilePath;
    private String mFeatureFilePath;

    private String mVoiceUiPatternPath;
    private String mVoiceUiModelPath;
    private int mVoiceUiLanguageId;

    private int mScreenOrientation;

    private int mCurMode;
    private int mHeadsetMode;

    private ActiveProcess mCurTopActiveProcess;
    private ArrayList<ActiveProcess> mActiveProcessList = new ArrayList<ActiveProcess>();

    private ActiveProcess mCurRecogProcess;
    private ActiveProcess mCurTrainingProcess;
    private ActiveProcess mCurContactsProcess;

//    private long mStartTime = -1L;
//    private long mEndTime = -1L;
    public JNICommandAdapter(IMessageDispatcher dispatcher) {
        // curHandler = handler;

        mUpDispatcher = dispatcher;
        mCurMode = MODE_VOICE_UNKNOW;
        mHeadsetMode = VOICE_NORMAL_MODE;
        native_setup(new WeakReference<JNICommandAdapter>(this));
    }

    private native void setVoicePasswordFile(FileDescriptor fd, long offset,
            long length) throws IllegalStateException, IllegalArgumentException;

    private native void setVoicePatternFile(FileDescriptor fd, long offset,
            long length) throws IllegalStateException, IllegalArgumentException;

    public native void setVoicePatternFile(String path)
            throws IllegalStateException;

    public native void setVoicePatternPath(String path)
            throws IllegalStateException;

    private native void setVoiceFeatureFile(FileDescriptor fd, long offset,
            long length) throws IllegalStateException, IllegalArgumentException;

    public native void setVoiceUBMFile(String path)
            throws IllegalStateException;

    public native void setModeIFile(String path) throws IllegalStateException;

    public native void setActiveAP(int apid, boolean isEnalbe)
            throws IllegalStateException;

    private native void setCommandId(int id) throws IllegalStateException;

    private native int getVoiceIntensity() throws IllegalStateException;

    private native void startCaptureVoice(int mode)
            throws IllegalStateException;

    private native void stopCaptureVoice(int mode) throws IllegalStateException;

    private native void startVoiceTraining() throws IllegalStateException;

    private native RecognitionResult startVoicePWRecognition()
            throws IllegalStateException;

    public native void setActiveLanguage(int id) throws IllegalStateException;

    private static final native void native_init() throws NoSuchMethodException;

    private final native void native_setup(Object voicerecognition_this)
            throws RuntimeException;

    private final native void setInputMode(int input_mode)
            throws IllegalStateException;

    public native void setContactProModePath(String modelpath, String contactsdbpath) throws IllegalStateException;

    private native void setContactName(String[] allContactsName)
            throws IllegalStateException;

    private native void setAPSelectedRlt(String selectedName)
            throws IllegalStateException;

    private native void setSearchRltCnts(int searchCnts)
            throws IllegalStateException;

    private native void setScreenOrientation(int screenOrientation)
            throws IllegalStateException;

    private final native void native_finalize();

    private final native void _release();

    static {
        System.loadLibrary("voicerecognition_jni");
        try {
            native_init();

        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    @Override
    public boolean isNativePrepared() {
        return mNativeContext == 0 ? false : true;
    }

    private final Handler curHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case NOTIFY_VOICE_ERROR:
                handleNotifyVoiceError(msg);
                break;
            case NOTIFY_VOICE_UI:
                handleNotifyVoiceUI(msg);
                break;
            case NOTIFY_VOICE_TRAINING:
                handleNotifyVoiceTraining(msg);
                break;
            case NOTIFY_VOICE_RECOGNITION:
                handleNotifyVoiceRecognition(msg);
                break;
            case NOTIFY_VOICE_CONTACTS:
                handleNotifyVoiceContacts(msg);
                break;
            default:
                break;
            }
        }
    };

    private void handleNotifyVoiceError(Message msg) {

        // Cureently we just notify something wrong in service

        if (mCurMode == MODE_VOICE_UNKNOW) {
            // Do nothing because app didn't request any business
            Log.i(VoiceCommandManagerStub.TAG,
                    "handleNotifyVoiceError mCurMode =  MODE_VOICE_UNKNOW");
        } else {
            VoiceMessage message = new VoiceMessage();
            message.mMainAction = mCurMode;
            switch (mCurMode) {
            case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
                if (mCurTopActiveProcess != null) {
                    message.mPkgName = mCurTopActiveProcess.mProcessName;
                    message.mSubAction = VoiceCommandListener.ACTION_VOICE_UI_NOTIFY;
                    stopVoiceUI(mCurTopActiveProcess.mProcessName,
                            mCurTopActiveProcess.mPid);
                }
                break;

            case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
                if (mCurTrainingProcess != null) {
                    message.mPkgName = mCurTrainingProcess.mProcessName;
                    message.mSubAction = VoiceCommandListener.ACTION_VOICE_TRAINING_NOTIFY;
                    stopVoiceTraining(false);
                }
                break;

            case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE:
                if (mCurRecogProcess != null) {
                    message.mPkgName = mCurRecogProcess.mProcessName;
                    message.mSubAction = VoiceCommandListener.ACTION_VOICE_RECOGNIZE_NOTIFY;
                    stopVoicePWRecognition();
                }
                break;

            case VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS:
                if (mCurContactsProcess != null) {
                    message.mPkgName = mCurContactsProcess.mProcessName;
                    message.mSubAction = VoiceCommandListener.ACTION_VOICE_CONTACTS_NOTIFY;
                    stopVoiceContacts();
                }
                break;
            default:
                break;
            }
            message.mExtraData = DataPackage
                    .packageErrorResult(VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE);
            mUpDispatcher.dispatchMessageUp(message);
        }
    }

    private void handleNotifyVoiceUI(Message msg) {
        if (mCurTopActiveProcess != null) {
            if (msg.arg1 == NOTIFY_VOICE_HEADSET_PLUG) {
                String processname = mCurTopActiveProcess.mProcessName;
                int pid = mCurTopActiveProcess.mPid;
                deleteActiveProcess(processname, pid);
                try {
                    setActiveAP(pid, false);
                    stopCaptureVoice(NATIVE_MODE_VOICE_UI);
                    mCurTopActiveProcess = null;
                    Log.i(VoiceCommandManagerStub.TAG,
                            "handleNotifyVoiceUI Headset stopCaptureVoice success");
                } catch (Exception e) {
                    Log.e(VoiceCommandManagerStub.TAG,
                            "stop capture voice Error" + e.getMessage());
                }
                startVoiceUI(mVoiceUiModelPath, mVoiceUiPatternPath,
                        processname, pid, mVoiceUiLanguageId);
            } else {
                VoiceMessage message = new VoiceMessage();
                message.mPkgName = mCurTopActiveProcess.mProcessName;
                message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_UI;
                message.mSubAction = VoiceCommandListener.ACTION_VOICE_UI_NOTIFY;
                // msg.arg1 is the command id
                message.mExtraData = DataPackage.packageResultInfo(
                        VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS,
                        msg.arg2, null);
                if (mUpDispatcher.dispatchMessageUp(message) != VoiceCommandListener.VOICE_NO_ERROR) {
                    stopVoiceUI(mCurTopActiveProcess.mProcessName,
                            mCurTopActiveProcess.mPid);
                }
            }
        }
    }

    private void handleNotifyVoiceTraining(Message msg) {
        if (mCurTrainingProcess != null) {
            VoiceMessage message = new VoiceMessage();
            message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING;
            message.mSubAction = VoiceCommandListener.ACTION_VOICE_TRAINING_NOTIFY;
            message.mPkgName = mCurTrainingProcess.mProcessName;
            if (msg.arg1 == NOTIFY_VOICE_TRAINING_FINISHED) {
                msg.arg2 = NOTIFY_VOICE_TRAINING_OK_CONFIDENCE;
                // Because the capture is finised ,we need to stop capture and
                // start
                // trainig
                stopVoiceTraining(true);
                try {
                    startVoiceTraining();
                } catch (Exception e) {
                    // Maybe the native training failed , but as
                    // discussed with native guy ,Service and Apps don't need to
                    // do
                    // anything
                    Log.i(VoiceCommandManagerStub.TAG,
                            "handleNotifyVoiceTraining startVoiceTraining Error "
                                    + e.getMessage());
                }

            } else if (msg.arg1 == NOTIFY_VOICE_TRAINING_EXIST_PSWD
                    || msg.arg1 == NOTIFY_VOICE_TRAINING_TIMEOUT
                    || msg.arg1 == NOTIFY_VOICE_HEADSET_PLUG) {
                stopVoiceTraining(false);
            }
            message.mExtraData = DataPackage.packageResultInfo(
                    VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, msg.arg1,
                    msg.arg2);
            if (mUpDispatcher.dispatchMessageUp(message) != VoiceCommandListener.VOICE_NO_ERROR) {
                stopVoiceTraining(false);
            }
        }
    }

    private void handleNotifyVoiceRecognition(Message msg) {
        if (mCurRecogProcess != null) {
            VoiceMessage message = new VoiceMessage();
            message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE;
            message.mSubAction = VoiceCommandListener.ACTION_VOICE_RECOGNIZE_NOTIFY;
            message.mPkgName = mCurRecogProcess.mProcessName;
            try {
                stopCaptureVoice(NATIVE_MODE_VOICE_RECOGNITION);
                if (msg.arg1 == NOTIFY_VOICE_HEADSET_PLUG) {
                    message.mExtraData = DataPackage.packageResultInfo(
                            VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS,
                            msg.arg1, 0);
                } else {
                    RecognitionResult result = startVoicePWRecognition();
                    Log.i(VoiceCommandManagerStub.TAG,
                            "handleNotifyVoiceRecognition msgid="
                                    + result.msgid + " voicecmdid="
                                    + result.voicecmdid);
                    message.mExtraData = DataPackage.packageResultInfo(
                            VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS,
                            result.msgid, result.voicecmdid);
                }

            } catch (Exception ex) {
                // Althoungh error happened ,we also think the training has been
                // finished , so do nothing just notify app training ok
                message.mExtraData = DataPackage
                        .packageErrorResult(VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE);
                Log.e(VoiceCommandManagerStub.TAG,
                        "handleNotifyVoiceRecognition Error " + mCurMode
                                + "exception = " + ex.getMessage());
            }
            mUpDispatcher.dispatchMessageUp(message);
            mCurMode = MODE_VOICE_UNKNOW;
        }
    }

    private void handleNotifyVoiceContacts(Message msg) {
//        mEndTime = System.currentTimeMillis();
//        Log.d(VoiceCommandManagerStub.TAG,
//                "JNICommandAdapter spends time : "
//                        + (mEndTime - mStartTime) + "ms");
        if (mCurContactsProcess != null) {
            VoiceMessage message = new VoiceMessage();
            String processname = mCurContactsProcess.mProcessName;
            int pid = mCurContactsProcess.mPid;

            if (msg.arg1 == NOTIFY_VOICE_HEADSET_PLUG) {
                try {
                    stopCaptureVoice(NATIVE_MODE_VOICE_CONTACTS);
                    mCurContactsProcess = null;
                    Log.i(VoiceCommandManagerStub.TAG,
                            "handleNotifyVoiceContacts Headset stopCaptureVoice success");
                } catch (Exception e) {
                    Log.e(VoiceCommandManagerStub.TAG,
                            "handleNotifyVoiceContacts stopcapturevoice Error"
                                    + e.getMessage());
                }
                startVoiceContacts(processname, pid, mScreenOrientation);
            } else {
                message.mPkgName = processname;
                message.pid = pid;
                message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS;
                // msg.obj is the command contacts name array
                String[] commandArray = (String[]) msg.obj;
                if (msg.arg2 == NOTIFY_VOICE_CONTACTS_SPEECHDETECTED) {
                    message.mSubAction = VoiceCommandListener.ACTION_VOICE_CONTACTS_SPEECHDETECTED;
                    message.mExtraData = DataPackage.packageSuccessResult();
                    Log.i(VoiceCommandManagerStub.TAG,
                            "handleNotifyVoiceContacts speech detected");
                } else if (msg.arg2 == NOTIFY_VOICE_CONTACTS_COMMANDARRAY) {
                    message.mSubAction = VoiceCommandListener.ACTION_VOICE_CONTACTS_NOTIFY;
                    Log.i(VoiceCommandManagerStub.TAG,
                            "handleNotifyVoiceContacts commandArray"
                                    + Arrays.toString(commandArray));
                    message.mExtraData = DataPackage.packageResultInfo(
                            VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS,
                            commandArray, null);
                }

                if (mUpDispatcher.dispatchMessageUp(message) != VoiceCommandListener.VOICE_NO_ERROR) {
                    stopVoiceContacts(message.mPkgName, message.pid);
                }
            }
        }
    }

    /*
     * Called from native code when an interesting event happens. This method
     * just uses the EventHandler system to post the event back to the main app
     * thread. We use a weak reference to the original VoiceRecognition object
     * so that the native code is safe from the object disappearing from
     * underneath it. (This is the cookie passed to native_setup().)
     */
    private static void postEventFromNative(Object jniCommandAdapter_ref,
            int what, int arg1, int arg2, Object obj) {

        JNICommandAdapter adapter = (JNICommandAdapter) ((WeakReference<?>) jniCommandAdapter_ref)
                .get();
        if (adapter == null) {
            return;
        }
        Log.i(VoiceCommandManagerStub.TAG, "Message from native what=" + what
                + " arg1=" + arg1 + " arg2=" + arg2);
        if (adapter.curHandler != null) {
            Object newObject = null;
            // If handle Notify VoiceContacts copy String[] from native
            if (what == NOTIFY_VOICE_CONTACTS) {
                String[] commandArray = null;
                String[] tempArray = (String[]) obj;
                if (tempArray != null) {
                    int length = tempArray.length;
                    commandArray = new String[length];
                    System.arraycopy(tempArray, 0, commandArray, 0, length);
                }
                newObject = commandArray;
            }
            // Send the message to the handler to deal with it later
            Message m = adapter.curHandler.obtainMessage(what, arg1, arg2,
                    newObject);
            adapter.curHandler.sendMessage(m);
        }
    }

    /*
     * Close the stream if used before and create a new file output stream
     * 
     * @param out
     * 
     * @param path
     * 
     * @return
     */
    private FileOutputStream resetFdStream(FileOutputStream out, String path) {

        stopFdStream(out);
        FileOutputStream outs = null;
        ConfigurationManager.makeDirForFile(path);

        try {
            outs = new FileOutputStream(path);
        } catch (FileNotFoundException e) {
            Log.e(VoiceCommandManagerStub.TAG, "ResetFdStream Error path =  "
                    + path);
        }
        return outs;
        // return null;
    }

    private boolean resetFdStream(String pwdfile, String patternfile,
            String featurefile) {

        if ((mPasswordFileStream = resetFdStream(mPasswordFileStream, pwdfile)) == null
                || (mPatternFileStream = resetFdStream(mPatternFileStream,
                        patternfile)) == null
                || (mFeatureFileStream = resetFdStream(mFeatureFileStream,
                        featurefile)) == null) {
            return false;
        }

        mPasswordFilePath = pwdfile;
        mPatternFilePath = patternfile;
        mFeatureFilePath = featurefile;
        return true;
    }

    /*
     * Service need to mantain the output stream for the native
     * 
     * @param outs
     * 
     * @return
     */
    private void stopFdStream(FileOutputStream outs) {

        if (outs != null) {
            try {
                outs.close();

            } catch (IOException e) {
                // TODO Auto-generated catch block
                outs = null;
                e.printStackTrace();
            }
        }
    }

    private void stopFdStream(boolean isdeletefile) {
        stopFdStream(mPasswordFileStream);
        stopFdStream(mPatternFileStream);
        stopFdStream(mFeatureFileStream);

        if (isdeletefile) {
            deleteFile(mPasswordFilePath);
            deleteFile(mPatternFilePath);
            deleteFile(mFeatureFilePath);
        }

        mPasswordFilePath = null;
        mPatternFilePath = null;
        mFeatureFilePath = null;
    }

    private boolean deleteFile(String path) {
        boolean isDeleted = false;
        try {
            File file = new File(path);
            if (file.isFile() && file.exists()) {
                isDeleted = file.delete();
            }
        } catch (Exception ex) {
            Log.e(VoiceCommandManagerStub.TAG, "deleteFile error " + path + " "
                    + ex.toString());
        }

        return isDeleted;
    }

    private void deleteCommandFiles(String pwdpath, String patternpath,
            String featurepath, int[] commandMask) {
        if (pwdpath == null || patternpath == null || featurepath == null
                || commandMask == null || commandMask.length != 2) {
            Log.e(VoiceCommandManagerStub.TAG, "deleteFile error ");
            return;
        }

        for (int i = 0; i < commandMask[1]; i++) {
            if ((commandMask[0] >> i & 1) == 0) {
                String filename = i + ".dat";
                deleteFile(pwdpath + filename);
                deleteFile(patternpath + filename);
                deleteFile(featurepath + filename);
            }
        }
    }

    @Override
    public int startVoiceTraining(String pwdpath, String patternpath,
            String featurepath, String umbpath, int commandid,
            int[] commandMask, String processname, int pid) {
        Log.i(VoiceCommandManagerStub.TAG, "startVoiceTraining ");
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            return errorid;
        }

        if (mCurMode != MODE_VOICE_UNKNOW) {
            Log.i(VoiceCommandManagerStub.TAG,
                    "startVoiceTraining: stop current mode = " + mCurMode);
            stopTopProcess();
            // errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALIDACTION;
        }
        deleteCommandFiles(pwdpath, patternpath, featurepath, commandMask);
        String filename = commandid + ".dat";
        if (resetFdStream(pwdpath + filename, patternpath + filename,
                featurepath + filename)) {
            try {
                setVoicePasswordFile(mPasswordFileStream.getFD(), 0, 0);
                setVoicePatternFile(mPatternFileStream.getFD(), 0, 0);
                setVoiceFeatureFile(mFeatureFileStream.getFD(), 0, 0);
                setVoicePatternFile(patternpath);
                setVoiceUBMFile(umbpath);
                setCommandId(commandid);
                setInputMode(mHeadsetMode);
                Log.i(VoiceCommandManagerStub.TAG,
                        "startVoiceTraining  pwdpath=" + pwdpath
                                + " patternpath=" + patternpath
                                + " featurepath=" + featurepath + " umbpath="
                                + umbpath + " commandid=" + commandid);
                startCaptureVoice(NATIVE_MODE_VOICE_TRAINING);
            } catch (Exception e) {
                // TODO Auto-generated catch block
                stopFdStream(true);
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                Log.e(VoiceCommandManagerStub.TAG, "startVoiceTraining Error "
                        + e.getMessage());
                // e.printStackTrace();
            }
            mCurTrainingProcess = new ActiveProcess(processname, pid);
            mCurMode = VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING;
        } else {
            Log.i(VoiceCommandManagerStub.TAG,
                    "startVoiceTraining Error because can't create the output stream "
                            + mCurMode);
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        }

        return errorid;
    }

    @Override
    public int resetVoiceTraining(String pwdpath, String patternpath,
            String featurepath, int commandid) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (mCurMode != VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING
                && mCurMode != VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE) {
            String filename = commandid + ".dat";
            deleteFile(pwdpath + filename);
            deleteFile(patternpath + filename);
            deleteFile(featurepath + filename);
        } else {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALIDACTION;
            Log.i(VoiceCommandManagerStub.TAG,
                    "resetVoiceTraining Error because the Native in other mode "
                            + mCurMode);
        }

        return errorid;
    }

    @Override
    public int stopVoiceTraining(String processname, int pid) {
        if (mCurTrainingProcess == null
                || !mCurTrainingProcess.mProcessName.equals(processname)
                || mCurTrainingProcess.mPid != pid) {
            return VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGALPROCESS;
        }

        stopVoiceTraining(false);

        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    private void stopVoiceTraining(boolean isfinished) {
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            return;
        }
        try {
            stopCaptureVoice(NATIVE_MODE_VOICE_TRAINING);
            Log.i(VoiceCommandManagerStub.TAG,
                    "stopVoiceTraining stopCaptureVoice success");
        } catch (Exception ex) {
            // Althoungh error happened ,we also think the training has been
            // finished , so do nothing just notify app training ok
            Log.e(VoiceCommandManagerStub.TAG, "stopVoiceTraining Error "
                    + mCurMode + "exception = " + ex.getMessage());
        }

        stopFdStream(!isfinished);
        mCurTrainingProcess = null;
        mCurMode = MODE_VOICE_UNKNOW;
    }

    @Override
    public int getNativeIntensity() {

        int intersity = 0;
        if (mCurMode == VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING
                || mCurMode == VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE
                || mCurMode == VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS) {
            try {
                intersity = getVoiceIntensity();
                // bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO,
                // intersity);
                Log.i(VoiceCommandManagerStub.TAG, "getNativeIntensity : "
                        + intersity);
            } catch (Exception e) {
                mCurMode = MODE_VOICE_UNKNOW;
                Log.e(VoiceCommandManagerStub.TAG, "getVoiceIntensity Error "
                        + e.getMessage());
            }
        }
        return intersity;
    }

    @Override
    public int startVoicePWRecognition(String patternpath, String ubmpath,
            String processname, int pid) {
        Log.i(VoiceCommandManagerStub.TAG, "startVoicePWRecognition ");
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            return errorid;
        }

        if (mCurMode != MODE_VOICE_UNKNOW) {
            // in other mode , we need to notify apps
            Log.i(VoiceCommandManagerStub.TAG,
                    "startVoicePWRecognition: stop current mode =" + mCurMode);
            stopTopProcess();
            // errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALIDACTION;
        }

        try {
            Log.i(VoiceCommandManagerStub.TAG,
                    "startVoicePWRecognition  startCaptureVoice");
            setVoicePatternFile(patternpath);
            setVoiceUBMFile(ubmpath);
            setInputMode(mHeadsetMode);
            Log.i(VoiceCommandManagerStub.TAG,
                    "startVoicePWRecognition  patternpath=" + patternpath
                            + " ubmpath=" + ubmpath);
            startCaptureVoice(NATIVE_MODE_VOICE_RECOGNITION);
            mCurMode = VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE;
            mCurRecogProcess = new ActiveProcess(processname, pid);
        } catch (Exception e) {
            mCurMode = MODE_VOICE_UNKNOW;
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            Log.e(VoiceCommandManagerStub.TAG, "startVoicePWRecognition Error "
                    + e.getMessage());
        }

        // wait the notification from natvie and decide to do
        return errorid;
    }

    @Override
    public int stopVoicePWRecognition(String processname, int pid) {
        if (mCurRecogProcess == null
                || !mCurRecogProcess.mProcessName.equals(processname)
                || mCurRecogProcess.mPid != pid) {
            return VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGALPROCESS;
        }

        return stopVoicePWRecognition();
    }

    private int stopVoicePWRecognition() {
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            return errorid;
        }
        if (mCurMode != VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE) {
            Log.i(VoiceCommandManagerStub.TAG,
                    "stopVoicePWRecognition Error because the Native in other mode "
                            + mCurMode);
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALIDACTION;
        } else {
            try {
                stopCaptureVoice(NATIVE_MODE_VOICE_RECOGNITION);
                Log.i(VoiceCommandManagerStub.TAG,
                        "stopVoicePWRecognition stopCaptureVoice success");
            } catch (Exception e) {
                // Although exception happened ,we also think
                // stopVoicePWRecognition success
                // errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                Log.e(VoiceCommandManagerStub.TAG,
                        "stopVoicePWRecognition Error " + e.getMessage());
            }
            mCurRecogProcess = null;
            mCurMode = MODE_VOICE_UNKNOW;
        }

        return errorid;
    }

    @Override
    public int startVoiceContacts(String processname, int pid, int screenOrientation) {
        Log.i(VoiceCommandManagerStub.TAG, "startVoiceContacts ");
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            return errorid;
        }

        if (mCurMode != MODE_VOICE_UNKNOW) {
            // in other mode , we need to notify apps
            Log.i(VoiceCommandManagerStub.TAG,
                    "startVoiceContacts: stop current mode =" + mCurMode);
            stopTopProcess();
        }

        try {
            mScreenOrientation = screenOrientation;
            setInputMode(mHeadsetMode);
            setScreenOrientation(screenOrientation);
            startCaptureVoice(NATIVE_MODE_VOICE_CONTACTS);
            Log.i(VoiceCommandManagerStub.TAG, "startVoiceContacts success");
//            mStartTime = System.currentTimeMillis();
        } catch (Exception e) {
            mCurMode = MODE_VOICE_UNKNOW;
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            Log.e(VoiceCommandManagerStub.TAG,
                    "startVoiceContacts Error " + e.getMessage());
        }
        mCurMode = VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS;
        mCurContactsProcess = new ActiveProcess(processname, pid);
        // wait the notification from natvie and decide to do
        return errorid;
    }

    @Override
    public int stopVoiceContacts(String processname, int pid) {
        if (mCurContactsProcess == null
                || !mCurContactsProcess.mProcessName.equals(processname)
                || mCurContactsProcess.mPid != pid) {
            return VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGALPROCESS;
        }

        return stopVoiceContacts();
    }

    private int stopVoiceContacts() {
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            return errorid;
        }
        if (mCurMode != VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS) {
            Log.i(VoiceCommandManagerStub.TAG,
                    "stopVoiceContacts Error because the Native in other mode "
                            + mCurMode);
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALIDACTION;
        } else {
            try {
                stopCaptureVoice(NATIVE_MODE_VOICE_CONTACTS);
                Log.i(VoiceCommandManagerStub.TAG,
                        "stopVoiceContacts stopCaptureVoice success");
            } catch (Exception e) {
                Log.e(VoiceCommandManagerStub.TAG, "stopVoiceContacts Error "
                        + e.getMessage());
            }
            mCurContactsProcess = null;
            mCurMode = MODE_VOICE_UNKNOW;
        }

        return errorid;
    }

    @Override
    public int sendContactsName(String modelpath, String contactsdbpath, String[] allContactsName) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
//        Log.i(VoiceCommandManagerStub.TAG,
//                "sendVoiceContacts allContactsName : "
//                        + Arrays.toString(allContactsName));
        try {
            Log.i(VoiceCommandManagerStub.TAG, "sendVoiceContacts modelpath=" + modelpath +
                    ", contactsdbpath=" + contactsdbpath + ", contactscount=" + allContactsName.length);
            setContactProModePath(modelpath, contactsdbpath);
            setContactName(allContactsName);
            Log.i(VoiceCommandManagerStub.TAG,
                    "sendVoiceContacts success");
        } catch (Exception e) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            Log.e(VoiceCommandManagerStub.TAG,
                    "sendVoiceContacts Error " + e.getMessage());
        }
        return errorid;
    }

    @Override
    public int sendContactsSelected(String selectedName) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Log.i(VoiceCommandManagerStub.TAG,
                "sendContactsSelected selectedName : " + selectedName);
        if (mCurMode == VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS) {
            try {
                setAPSelectedRlt(selectedName);
                Log.i(VoiceCommandManagerStub.TAG,
                        "sendContactsSelected success");
            } catch (Exception e) {
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                Log.e(VoiceCommandManagerStub.TAG,
                        "sendContactsSelected Error " + e.getMessage());
            }
        } else {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALIDACTION;
            Log.i(VoiceCommandManagerStub.TAG,
                    "sendContactsSelected Error because the Native in other mode "
                            + mCurMode);
        }
        return errorid;
    }

    @Override
    public int sendContactsSearchCnt(int searchCnt) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Log.i(VoiceCommandManagerStub.TAG, "sendContactsSearchCnt searchCnt : "
                + searchCnt);
        try {
            setSearchRltCnts(searchCnt);
            Log.i(VoiceCommandManagerStub.TAG, "sendContactsSearchCnt success");
        } catch (Exception e) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            Log.e(VoiceCommandManagerStub.TAG, "sendContactsSearchCnt Error "
                    + e.getMessage());
        }
        return errorid;
    }

    @Override
    public int sendContactsOrientation(int screenOrientation) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Log.i(VoiceCommandManagerStub.TAG,
                "sendContactsOrientation screenOrientation : " + screenOrientation);
        if (mCurMode == VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS) {
            try {
                setScreenOrientation(screenOrientation);
                Log.i(VoiceCommandManagerStub.TAG,
                        "sendContactsOrientation success");
            } catch (Exception e) {
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                Log.e(VoiceCommandManagerStub.TAG,
                        "sendContactsOrientation Error " + e.getMessage());
            }
        } else {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALIDACTION;
            Log.i(VoiceCommandManagerStub.TAG,
                    "sendContactsOrientation Error because the Native in other mode "
                            + mCurMode);
        }
        return errorid;
    }

    @Override
    public int startVoiceUI(String modelpath, String patternpath,
            String processname, int pid, int languageid) {

        Log.i(VoiceCommandManagerStub.TAG, "startVoiceUI ");
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            return errorid;
        }

        if (mCurMode != MODE_VOICE_UNKNOW
                && mCurMode != VoiceCommandListener.ACTION_MAIN_VOICE_UI) {
            // Because the native is in training or recognition mode
            Log.i(VoiceCommandManagerStub.TAG,
                    "startVoiceUI: stop current mode =" + mCurMode);
            stopTopProcess();
            // errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALIDACTION;
        } else if (mCurMode == VoiceCommandListener.ACTION_MAIN_VOICE_UI
                && mCurTopActiveProcess != null) {
            try {
                setActiveAP(mCurTopActiveProcess.mPid, false);
                stopCaptureVoice(NATIVE_MODE_VOICE_UI);
                mCurTopActiveProcess = null;
            } catch (Exception e) {
                Log.e(VoiceCommandManagerStub.TAG,
                        "startVoiceUI Error when stop capture voice first"
                                + e.getMessage());
                // Do nothing because we need to continue to start the voice
                // ui
            }
        }

        if (addActiveProcess(processname, pid)) {

            mCurMode = VoiceCommandListener.ACTION_MAIN_VOICE_UI;
            try {
                mVoiceUiPatternPath = patternpath;
                mVoiceUiModelPath = modelpath;
                mVoiceUiLanguageId = languageid;
                setInputMode(mHeadsetMode);
                setModeIFile(modelpath);
                setVoicePatternPath(patternpath);
                setActiveAP(pid, true);
                setActiveLanguage((0x01 << (languageid - 1)) & 0xFF);
                Log.i(VoiceCommandManagerStub.TAG, "startVoiceUI  modelpath="
                        + modelpath + " patternpath=" + patternpath
                        + " languageid = "
                        + ((0x01 << (languageid - 1)) & 0xFF));
                startCaptureVoice(NATIVE_MODE_VOICE_UI);
                Log.i(VoiceCommandManagerStub.TAG,
                        "startVoiceUI startCaptureVoice success");
            } catch (Exception e) {
                mCurMode = MODE_VOICE_UNKNOW;
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                Log.e(VoiceCommandManagerStub.TAG,
                        "startVoiceUI Error " + e.getMessage());
            }
        }
        return errorid;
    }

    @Override
    public int stopVoiceUI(String processname, int pid) {

        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            return errorid;
        }
        if (mCurMode != VoiceCommandListener.ACTION_MAIN_VOICE_UI) {
            Log.i(VoiceCommandManagerStub.TAG,
                    "stopVoiceUI Error because the Native in other mode "
                            + mCurMode);
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALIDACTION;
        } else {
            deleteActiveProcess(processname, pid);
            int size = mActiveProcessList.size();
            if (size > 0) {
                // Other process are waitting the command id
                // We don't support multi processes use the voice ui function
                // now, this case won't happen at this time
                mCurTopActiveProcess = mActiveProcessList.get(size - 1);
            } else {
                // No process started the voice ui
                mCurMode = MODE_VOICE_UNKNOW;
                mCurTopActiveProcess = null;
                try {
                    mVoiceUiPatternPath = null;
                    mVoiceUiModelPath = null;
                    mVoiceUiLanguageId = -1;
                    setActiveAP(pid, false);
                    stopCaptureVoice(NATIVE_MODE_VOICE_UI);
                    Log.i(VoiceCommandManagerStub.TAG,
                            "stopVoiceUI stopCaptureVoice success");
                } catch (Exception e) {
                    // Although exception happened ,we also think stopping voice
                    // ui success
                    // errorid =
                    // VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                    Log.e(VoiceCommandManagerStub.TAG,
                            "stopVoiceUI Error " + e.getMessage());
                }
            }
        }
        return errorid;
    }

    @Override
    public void stopCurMode(String processname, int pid) {
        switch (mCurMode) {
        case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
            stopVoiceUI(processname, pid);
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE:
            stopVoicePWRecognition(processname, pid);
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
            stopVoiceTraining(processname, pid);
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS:
            stopVoiceContacts(processname, pid);
            break;
        default:
            break;
        }
    }

    private boolean addActiveProcess(String processname, int pid) {

        if (mCurTopActiveProcess != null
                && mCurTopActiveProcess.mProcessName.equals(processname)
                && mCurTopActiveProcess.mPid == pid) {
            return false;
        }

        int size = mActiveProcessList.size() - 1;

        ActiveProcess activeProcess = null;

        for (int i = 0; i < size; i++) {

            activeProcess = mActiveProcessList.get(i);

            if (activeProcess.mProcessName.equals(processname)) {
                mActiveProcessList.remove(activeProcess);
                if (activeProcess.mPid != pid) {
                    // Will this case happen?
                    // Means Apps died but service didn't know the event
                    activeProcess = null;
                }
                break;
            }
        }

        if (activeProcess == null) {
            activeProcess = new ActiveProcess(processname, pid);
        }

        mCurTopActiveProcess = activeProcess;
        mActiveProcessList.add(activeProcess);

        return true;
    }

    private void stopTopProcess() {
        switch (mCurMode) {
        case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
            if (mCurTopActiveProcess != null) {
                stopVoiceUI(mCurTopActiveProcess.mProcessName,
                        mCurTopActiveProcess.mPid);
            }
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE:
            stopVoicePWRecognition();
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
            stopVoiceTraining(false);
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS:
            if (mCurContactsProcess != null) {
                stopVoiceContacts();
            }
            break;
        default:
            break;
        }
    }

    private void deleteActiveProcess(String processname, int pid) {
        int size = mActiveProcessList.size();
        for (int i = (size - 1); i >= 0; i--) {
            ActiveProcess activeProcess = mActiveProcessList.get(i);
            if (activeProcess.mProcessName.equals(processname)
                    && activeProcess.mPid == pid) {
                mActiveProcessList.remove(activeProcess);
            }
        }
    }

    private class ActiveProcess {
        ActiveProcess(String processname, int pid) {
            mProcessName = processname;
            mPid = pid;
        }

        String mProcessName;
        int mPid;
    }

    @Override
    public void release() {
        // TODO Auto-generated method stub
        switch (mCurMode) {
        case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
            stopCaptureVoice(NATIVE_MODE_VOICE_UI);
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE:
            stopCaptureVoice(NATIVE_MODE_VOICE_RECOGNITION);
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
            stopCaptureVoice(NATIVE_MODE_VOICE_TRAINING);
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS:
            stopCaptureVoice(NATIVE_MODE_VOICE_CONTACTS);
            break;
        default:
            break;
        }

        _release();
    }

    @Override
    public void setCurHeadsetMode(boolean isPlugin) {
        // TODO Auto-generated method stub
        if (isPlugin) {
            Log.i(VoiceCommandManagerStub.TAG, "handleHeadSetPlugEvent in");
            mHeadsetMode = VOICE_HEADSET_MODE;
        } else {
            Log.i(VoiceCommandManagerStub.TAG, "handleHeadSetPlugEvent out");
            mHeadsetMode = VOICE_NORMAL_MODE;
        }

        if (mCurMode == MODE_VOICE_UNKNOW) {
            return;
        }
        Message message = new Message();
        switch (mCurMode) {
        case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
            message.what = NOTIFY_VOICE_UI;
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNIZE:
            message.what = NOTIFY_VOICE_RECOGNITION;
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
            message.what = NOTIFY_VOICE_TRAINING;
            break;
        case VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS:
            message.what = NOTIFY_VOICE_CONTACTS;
            break;
        default:
            break;
        }
        message.arg1 = NOTIFY_VOICE_HEADSET_PLUG;
        curHandler.sendMessage(message);
    }
}
