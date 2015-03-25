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
package com.mediatek.voicecommand.app;

import java.lang.ref.WeakReference;

import android.os.Handler;
import android.os.Message;
import android.util.Log;
import com.mediatek.common.voicecommand.IVoicePhoneDetection;

public class VoicePhoneDetection implements IVoicePhoneDetection {

    private int mNativeContext = 0;
    public static String TAG = "VoicePhoneDetection";
    private Handler mCurHandler;

    public VoicePhoneDetection(Handler handler, int type) {
        mCurHandler = handler;
        native_setup(new WeakReference<VoicePhoneDetection>(this), type);
    }

    static {
        System.loadLibrary("phonemotiondetector_jni");
        try {
            native_init();

        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    // private final Handler mCurHandler = new Handler() {
    // @Override
    // public void handleMessage(Message msg) {
    // if (mListener != null) {
    // mListener.onNotify(msg.what, msg.arg1, msg.arg2);
    // }
    // }
    // };

    private final native void native_setup(Object phonedetection_this, int type)
            throws RuntimeException;

    private static final native void native_init() throws NoSuchMethodException;

    private final native void native_finalize();

    private final native void _release();

    private native void startPhoneDetect() throws IllegalStateException;

    private native void stopPhoneDetect() throws IllegalStateException;

    public void startPhoneDetection() throws IllegalStateException {

        if (mNativeContext != 0) {
            Log.i(TAG, "startPhoneDetectInNative");
            startPhoneDetect();
        }
    }

    public void stopPhoneDetection() throws IllegalStateException {
        if (mNativeContext != 0) {
            Log.i(TAG, "stopPhoneDetectInNative");
            stopPhoneDetect();
        }
    }

    public void releaseSelf() {
        if (mNativeContext != 0) {
            Log.i(TAG, "releaseSelf");
            _release();
        }
    }

    /*
     * Called from native code when an interesting event happens. This method
     * just uses the EventHandler system to post the event back to the main app
     * thread. We use a weak reference to the original VoiceRecognition object
     * so that the native code is safe from the object disappearing from
     * underneath it. (This is the cookie passed to native_setup().)
     */
    private static void postEventFromNative(Object voicePhoneDetection_ref,
            int what, int arg1, int arg2, Object obj) {
        VoicePhoneDetection detection = (VoicePhoneDetection) ((WeakReference) voicePhoneDetection_ref)
                .get();

        Log.i(TAG, "Message from native what=" + what + " arg1=" + arg1
                + " arg2=" + arg2);

        if (detection.mCurHandler != null) {
            Message m = detection.mCurHandler.obtainMessage(what, arg1, arg2,
                    obj);
            detection.mCurHandler.dispatchMessage(m);
        } else {
            Log.e(TAG, "Message from native but handler is null!");
        }
    }
}
