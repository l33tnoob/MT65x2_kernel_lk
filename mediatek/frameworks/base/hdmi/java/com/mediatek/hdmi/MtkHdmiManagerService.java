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

package com.mediatek.hdmi;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.os.Binder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UEventObserver;
import android.os.UserHandle;
import android.provider.Settings;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.hdmi.IHDMINative;
import com.mediatek.common.hdmi.IMtkHdmiManager;
import com.mediatek.internal.R;
import com.mediatek.xlog.Xlog;

import java.io.FileDescriptor;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Arrays;

/**
 * HDMI manager service, used to provide API for APPs to manage HDMI
 */
public final class MtkHdmiManagerService extends IMtkHdmiManager.Stub {
    private static final String TAG = "MtkHdmiService";
    private final Context mContext;
    private final ContentResolver mContentResolver;
    private static final int MSG_INIT = 0;
    private static final int MSG_DEINIT = MSG_INIT + 1;
    private static final int MSG_CABLE_STATE = MSG_INIT + 2;
    private static final int AP_CFG_RDCL_FILE_HDCP_KEY_LID = 42;
    private static final String ACTION_IPO_BOOT = "android.intent.action.ACTION_BOOT_IPO";
    private static final String ACTION_IPO_SHUTDOWN = "android.intent.action.ACTION_SHUTDOWN_IPO";
    private static final String ACTION_CLEARMOTION_DIMMED = "com.mediatek.clearmotion.DIMMED_UPDATE";
    private static final String KEY_CLEARMOTION_DIMMED = "sys.display.clearMotion.dimmed";
    private static final int HDMI_ENABLE_STATUS_DEFAULT = 1;
    private static final int HDMI_VIDEO_RESOLUTION_DEFAULT = 100;
    private static final int HDMI_COLOR_SPACE_DEFAULT = 0;
    private static final int HDMI_DEEP_COLOR_DEFAULT = 1;
    private PowerManager.WakeLock mWakeLock = null;
    private HdmiHandler mHandler;
    private HandlerThread mHandlerThread;
    private IHDMINative mHdmiNative;
    private HdmiObserver mHdmiObserver;
    private boolean mHdmiEnabled;
    private int mHdmiVideoResolution;
    private int mHdmiColorSpace;
    private int mHdmiDeepColor;
    private boolean mCablePlugged;
    private int[] mEdid;
    private int[] mPreEdid;
    private boolean mInitialized = false;
    private boolean mIsSmartBookPluggedIn = false;

    private boolean mIsHdVideoPlaying = false;
    private boolean mNeedRestore = false;

    private class HdmiHandler extends Handler {
        public HdmiHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            log(TAG, "handleMessage: " + msg.what);
            if (null == mHandlerThread || !mHandlerThread.isAlive()
                    || mHandlerThread.isInterrupted()) {
                log(TAG, "handler thread is error");
                return;
            }
            switch (msg.what) {
            case MSG_INIT:
                initHdmi();
                mInitialized = true;
                break;
            case MSG_DEINIT:
                deinitHdmi();
                break;
            case MSG_CABLE_STATE:
                int state = (Integer) msg.obj;
                hdmiCableStateChanged(state);
                break;
            default:
                super.handleMessage(msg);
            }
        }

        private void deinitHdmi() {
            enableHdmiImpl(false);
            if (isSignalOutputting()) {
                handleCablePlugged(false);
            }
        }

        private void initHdmi() {
            loadHdmiSettings();
            enableHdmiImpl(mHdmiEnabled);
            handleCablePlugged(mCablePlugged);
        }

    }

    private void hdmiCableStateChanged(int state) {
        mCablePlugged = state == 1;
        Settings.System.putIntForUser(mContentResolver,
                Settings.System.HDMI_CABLE_PLUGGED, state,
                UserHandle.USER_CURRENT);
        if (mInitialized) {
            if (mIsHdVideoPlaying && mCablePlugged) {
                int type = getDisplayType();
                if (type != 1) {
                    String contentStr = mContext.getResources().getString(
                            R.string.hdmi_hdvideo_toast);
                    String hdmi = mContext.getResources().getString(
                            R.string.hdmi_replace_hdmi);
                    String mhl = mContext.getResources().getString(
                            R.string.hdmi_replace_mhl);
                    if (type == 2) {
                        contentStr = contentStr.replaceAll(hdmi, mhl);
                    }
                    log(TAG, "disable hdmi when play HD video");
                    Toast.makeText(mContext, contentStr, Toast.LENGTH_LONG)
                            .show();
                    mNeedRestore = true;
                    log(TAG, "mIsHdVideoPlaying: " + mIsHdVideoPlaying + " mNeedRestore: " + mNeedRestore);
                    enableHdmi(false);
                    return;
                }
            }
            handleCablePlugged(mCablePlugged);
        }
    }

    private void handleCablePlugged(boolean plugged) {
        updateClearMotionDimmed(plugged);
        if (plugged) {
            if (FeatureOption.MTK_MT8193_HDMI_SUPPORT
                    || FeatureOption.MTK_INTERNAL_HDMI_SUPPORT
                    || FeatureOption.MTK_INTERNAL_MHL_SUPPORT) {
                refreshEdid(plugged);
            }
            if (FeatureOption.MTK_MT8193_HDMI_SUPPORT
                    || FeatureOption.MTK_INTERNAL_HDMI_SUPPORT
                    || FeatureOption.MTK_INTERNAL_MHL_SUPPORT) {
                setColorAndDeepImpl(mHdmiColorSpace, mHdmiDeepColor);
            }
            initVideoResolution(mHdmiVideoResolution);
        } else {
            if (FeatureOption.MTK_MT8193_HDMI_SUPPORT
                    || FeatureOption.MTK_INTERNAL_HDMI_SUPPORT
                    || FeatureOption.MTK_INTERNAL_MHL_SUPPORT) {
                refreshEdid(plugged);
            }
        }
        // if smart book plug in, don't show hdmi settings notification
        boolean isShowNotification = plugged && !mIsSmartBookPluggedIn;
        handleNotification(isShowNotification);
        updateWakeLock(plugged, mHdmiEnabled);
    }

    private void updateClearMotionDimmed(boolean plugged) {
        if (FeatureOption.MTK_CLEARMOTION_SUPPORT) {
            SystemProperties.set(KEY_CLEARMOTION_DIMMED, plugged ? "1" : "0");
            mContext.sendBroadcastAsUser(new Intent(ACTION_CLEARMOTION_DIMMED), UserHandle.ALL);
        }
    }

    private void refreshEdid(boolean plugged) {
        if (plugged) {
            mEdid = getResolutionMask();
            if (mEdid != null) {
                for (int i = 0; i < mEdid.length; i++) {
                    log(TAG, String.format("mEdid[%d] = %d", i, mEdid[i]));
                }
            } else {
                log(TAG, "mEdid is null!");
            }

            if (mPreEdid != null) {
                for (int i = 0; i < mPreEdid.length; i++) {
                    log(TAG, String.format("mPreEdid[%d] = %d", i, mPreEdid[i]));
                }
            } else {
                log(TAG, "mPreEdid is null!");
            }
        } else {
            mPreEdid = mEdid;
            // mEdid = null;
        }
    }

    private void handleNotification(boolean plugged) {
        NotificationManager notificationManager = (NotificationManager) mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
        if (notificationManager == null) {
            Xlog.w(TAG, "Fail to get NotificationManager");
            return;
        }
        if (plugged) {
            log(TAG, "HDMI cable is pluged in, show notification now");
            Notification notification = new Notification();
            String titleStr = mContext.getResources().getString(
                    R.string.hdmi_notification_title);
            String contentStr = mContext.getResources().getString(
                    R.string.hdmi_notification_content);
            String hdmi = mContext.getResources().getString(
                    R.string.hdmi_replace_hdmi);
            String mhl = mContext.getResources().getString(
                    R.string.hdmi_replace_mhl);
            notification.icon = R.drawable.ic_hdmi_notification;
            if (2 == getDisplayType()) {
                titleStr = titleStr.replaceAll(hdmi, mhl);
                contentStr = contentStr.replaceAll(hdmi, mhl);
                notification.icon = R.drawable.ic_mhl_notification;
            }
            notification.tickerText = titleStr;
            notification.flags = Notification.FLAG_ONGOING_EVENT
                    | Notification.FLAG_NO_CLEAR
                    | Notification.FLAG_SHOW_LIGHTS;
            Intent intent = Intent
                    .makeRestartActivityTask(new ComponentName(
                            "com.android.settings",
                            "com.android.settings.HDMISettings"));
            PendingIntent pendingIntent = PendingIntent.getActivityAsUser(
                    mContext, 0, intent, 0, null, UserHandle.CURRENT);
            notification.setLatestEventInfo(mContext, titleStr, contentStr,
                    pendingIntent);
            notificationManager.notifyAsUser(null,
                    R.drawable.ic_hdmi_notification, notification,
                    UserHandle.CURRENT);
        } else {
            log(TAG, "HDMI cable is pluged out, clear notification now");
            notificationManager.cancelAsUser(null,
                    R.drawable.ic_hdmi_notification, UserHandle.CURRENT);
        }
    }

    private BroadcastReceiver mActionReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            log(TAG, "receive: " + action);
            if (Intent.ACTION_BOOT_COMPLETED.equals(action)
                    || ACTION_IPO_BOOT.equals(action)) {
                sendMsg(MSG_INIT);
            } else if (Intent.ACTION_SHUTDOWN.equals(action)
                    || ACTION_IPO_SHUTDOWN.equals(action)) {
                sendMsg(MSG_DEINIT);
            } else if (Intent.ACTION_USER_SWITCHED.equals(action)) {
                sendMsg(MSG_DEINIT);
                sendMsg(MSG_INIT);
            // { @ Smart book hdmi settings
            } else if (Intent.ACTION_SMARTBOOK_PLUG.equals(action)) {
                mIsSmartBookPluggedIn = intent.getBooleanExtra(Intent.EXTRA_SMARTBOOK_PLUG_STATE, false);
                Xlog.d(TAG, "smartbook plug:" + mIsSmartBookPluggedIn);
                // if smart book plug in or out, don't show hdmi settings notification
                handleNotification(false);
            }
            // @ }
        }

        private void sendMsg(int msgInit) {
            if (!mHandler.hasMessages(msgInit)) {
                mHandler.sendEmptyMessage(msgInit);
                log(TAG, "send msg: " + msgInit);
            }
        }

    };

    private ContentObserver mHdmiSettingsObserver = new ContentObserver(
            mHandler) {
        @Override
        public void onChange(boolean selfChange) {
            Xlog.d(TAG, "hdmiSettingsObserver onChanged: " + selfChange);
            mHdmiEnabled = Settings.System.getIntForUser(mContentResolver,
                    Settings.System.HDMI_ENABLE_STATUS, 1,
                    UserHandle.USER_CURRENT) == 1;
            updateWakeLock(mCablePlugged, mHdmiEnabled);
        }
    };

    public MtkHdmiManagerService(Context context) {
        log(TAG, "MtkHdmiManagerService constructor");
        mContext = context;
        mContentResolver = mContext.getContentResolver();
        initial();
    }

    private void initial() {
        if (null == mHdmiNative) {
            mHdmiNative = MediatekClassFactory
                    .createInstance(IHDMINative.class);
        }
        if (null == mHandlerThread || !mHandlerThread.isAlive()) {
            mHandlerThread = new HandlerThread("HdmiService");
            mHandlerThread.start();
            mHandler = new HdmiHandler(mHandlerThread.getLooper());
            IntentFilter filter = new IntentFilter();
            filter.addAction(Intent.ACTION_SHUTDOWN);
            filter.addAction(Intent.ACTION_BOOT_COMPLETED);
            filter.addAction(Intent.ACTION_USER_SWITCHED);
            filter.addAction(ACTION_IPO_SHUTDOWN);
            filter.addAction(ACTION_IPO_BOOT);
            // { @ Smart book hdmi settings
            if (FeatureOption.MTK_SMARTBOOK_SUPPORT) {
                filter.addAction(Intent.ACTION_SMARTBOOK_PLUG);
            }
            // @ }
            mContext.registerReceiverAsUser(mActionReceiver, UserHandle.ALL,
                    filter, null, mHandler);
        }
        if (null == mWakeLock) {
            PowerManager mPowerManager = (PowerManager) mContext
                    .getSystemService(Context.POWER_SERVICE);
            mWakeLock = mPowerManager.newWakeLock(
                    PowerManager.SCREEN_BRIGHT_WAKE_LOCK, "HDMI");
            mWakeLock.setReferenceCounted(false);
        }
        if (null == mHdmiObserver) {
            mHdmiObserver = new HdmiObserver(mContext);
            mHdmiObserver.startObserve();
        }
        if ((FeatureOption.MTK_MT8193_HDCP_SUPPORT)
                || (FeatureOption.MTK_HDMI_HDCP_SUPPORT)) {
            mHandler.post(new Runnable() {

                @Override
                public void run() {
                    if (FeatureOption.MTK_DRM_KEY_MNG_SUPPORT) {
                        log(TAG, "setDrmKey: " + setDrmKey());
                    } else {
                        log(TAG, "setHdcpKey: " + setHdcpKey());
                    }
                }
            });
        }
        observeSettings();
    }

    private void updateWakeLock(boolean plugged, boolean hdmiEnabled) {
        if (plugged && hdmiEnabled && mHdmiNative.isHdmiForceAwake()) {
            mWakeLock.acquire();
        } else {
            mWakeLock.release();
        }
    }

    private boolean setHdcpKey() {
        byte[] key = null;
        IBinder binder = ServiceManager.getService("NvRAMAgent");
        NvRAMAgent agent = NvRAMAgent.Stub.asInterface(binder);
        if (agent != null) {
            try {
                log(TAG, "Read HDCP key from nvram");
                key = agent.readFile(AP_CFG_RDCL_FILE_HDCP_KEY_LID);
                for (int i = 0; i < 287; i++) {
                    log(TAG, String.format("HDCP key[%d] = %d", i, key[i]));
                }
                if (null != key) {
                    return mHdmiNative.setHDCPKey(key);
                }
            } catch (RemoteException e) {
                Xlog.w(TAG, "NvRAMAgent read file fail");
            }
        }
        return false;
    }

    private boolean setDrmKey() {
        synchronized (mHdmiNative) {
            return mHdmiNative.setHDMIDRMKey();
        }
    }

    private void loadHdmiSettings() {
        mHdmiEnabled = Settings.System.getIntForUser(mContentResolver,
                Settings.System.HDMI_ENABLE_STATUS, HDMI_ENABLE_STATUS_DEFAULT,
                UserHandle.USER_CURRENT) == 1;
        mHdmiVideoResolution = Settings.System.getIntForUser(mContentResolver,
                Settings.System.HDMI_VIDEO_RESOLUTION,
                HDMI_VIDEO_RESOLUTION_DEFAULT, UserHandle.USER_CURRENT);
        mHdmiColorSpace = Settings.System.getIntForUser(mContentResolver,
                Settings.System.HDMI_COLOR_SPACE, HDMI_COLOR_SPACE_DEFAULT,
                UserHandle.USER_CURRENT);
        mHdmiDeepColor = Settings.System.getIntForUser(mContentResolver,
                Settings.System.HDMI_DEEP_COLOR, HDMI_DEEP_COLOR_DEFAULT,
                UserHandle.USER_CURRENT);
        mIsHdVideoPlaying = false;
        mNeedRestore = false;
    }

    private void observeSettings() {
        mContentResolver.registerContentObserver(
                Settings.System.getUriFor(Settings.System.HDMI_ENABLE_STATUS),
                false, mHdmiSettingsObserver, UserHandle.USER_ALL);
    }

    @Override
    protected void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
        mContext.enforceCallingOrSelfPermission(
                android.Manifest.permission.DUMP, TAG);
        pw.println("MTK HDMI MANAGER (dumpsys HDMI)");
        pw.println("HDMI mHdmiEnabled: " + mHdmiEnabled);
        pw.println("HDMI mHdmiVideoResolution: " + mHdmiVideoResolution);
        pw.println("HDMI mHdmiColorSpace: " + mHdmiColorSpace);
        pw.println("HDMI mHdmiDeepColor: " + mHdmiDeepColor);
        pw.println("HDMI mCablePlugged: " + mCablePlugged);
        pw.println("HDMI mEdid: " + Arrays.toString(mEdid));
        pw.println("HDMI mPreEdid: " + Arrays.toString(mPreEdid));
        pw.println("HDMI mInitialized: " + mInitialized);
        pw.println();
    }

    @Override
    public boolean enableHdmi(boolean enabled) {
        log(TAG, "enableHdmi: " + enabled);
        boolean ret = false;
        if (enabled == mHdmiEnabled) {
            log(TAG, "mHdmiEnabled is the same: " + enabled);
        } else {
            ret = enableHdmiImpl(enabled);
            if (ret) {
                final long ident = Binder.clearCallingIdentity();
                try {
                    mHdmiEnabled = enabled;
                    Settings.System.putIntForUser(mContentResolver,
                            Settings.System.HDMI_ENABLE_STATUS,
                            mHdmiEnabled ? 1 : 0, UserHandle.USER_CURRENT);
                } finally {
                    Binder.restoreCallingIdentity(ident);
                }
            }
        }
        return ret;
    }

    private boolean enableHdmiImpl(boolean enabled) {
        synchronized (mHdmiNative) {
            return mHdmiNative.enableHDMI(enabled);
        }
    }

    @Override
    public int[] getResolutionMask() {
        log(TAG, "getResolutionMask");
        synchronized (mHdmiNative) {
            return mHdmiNative.getEDID();
        }
    }

    @Override
    public boolean isSignalOutputting() {
        log(TAG, "isSignalOutputting");
        return mCablePlugged && mHdmiEnabled;
    }

    @Override
    public boolean setColorAndDeep(int color, int deep) {
        log(TAG, "setColorAndDeep: " + color + ", " + deep);
        boolean ret = setColorAndDeepImpl(color, deep);
        if (ret) {
            final long ident = Binder.clearCallingIdentity();
            try {
                mHdmiColorSpace = color;
                mHdmiDeepColor = deep;
                Settings.System.putIntForUser(mContentResolver,
                        Settings.System.HDMI_COLOR_SPACE, color,
                        UserHandle.USER_CURRENT);
                Settings.System.putIntForUser(mContentResolver,
                        Settings.System.HDMI_DEEP_COLOR, deep,
                        UserHandle.USER_CURRENT);
            } finally {
                Binder.restoreCallingIdentity(ident);
            }
        }
        return ret;
    }

    private boolean setColorAndDeepImpl(int color, int deep) {
        synchronized (mHdmiNative) {
            return mHdmiNative.setDeepColor(color, deep);
        }
    }

    @Override
    public boolean setVideoResolution(int resolution) {
        log(TAG, "setVideoResolution: " + resolution);
        boolean ret = false;
        int suitableResolution = resolution;
        if (resolution >= HdmiDef.AUTO) {
            suitableResolution = getSuitableResolution(resolution);
        }
        if (suitableResolution == mHdmiVideoResolution) {
            log(TAG, "setVideoResolution is the same");
        }
        int finalResolution = suitableResolution >= HdmiDef.AUTO ? (suitableResolution - HdmiDef.AUTO)
                : suitableResolution;
        log(TAG, "final video resolution: " + finalResolution);
        ret = setVideoResolutionImpl(finalResolution);
        if (ret) {
            final long ident = Binder.clearCallingIdentity();
            try {
                mHdmiVideoResolution = suitableResolution;
                Settings.System.putIntForUser(mContentResolver,
                        Settings.System.HDMI_VIDEO_RESOLUTION,
                        mHdmiVideoResolution, UserHandle.USER_CURRENT);
            } finally {
                Binder.restoreCallingIdentity(ident);
            }
        }
        return ret;
    }

    private void initVideoResolution(int resolution) {
        log(TAG, "initVideoResolution: " + resolution);
        if (isResolutionSupported(resolution)) {
            setVideoResolutionImpl(resolution);
        } else {
            int suitableResolution = getSuitableResolution(resolution);
            int finalResolution = suitableResolution >= HdmiDef.AUTO ? (suitableResolution - HdmiDef.AUTO)
                    : suitableResolution;
            log(TAG, "initVideoResolution final video resolution: "
                    + finalResolution);
            if (setVideoResolutionImpl(finalResolution)) {
                mHdmiVideoResolution = suitableResolution;
                Settings.System.putIntForUser(mContentResolver,
                        Settings.System.HDMI_VIDEO_RESOLUTION,
                        mHdmiVideoResolution, UserHandle.USER_CURRENT);
            }
        }
    }

    private boolean isResolutionSupported(int resolution) {
        log(TAG, "isResolutionSupported: " + resolution);
        if (resolution >= HdmiDef.AUTO) {
            return false;
        }
        int[] supportedResolutions = getSupportedResolutions();
        for (int res : supportedResolutions) {
            if (res == resolution) {
                log(TAG, "resolution is supported");
                return true;
            }
        }
        return false;
    }

    private boolean setVideoResolutionImpl(int resolution) {
        synchronized (mHdmiNative) {
            return mHdmiNative.setVideoConfig(resolution);
        }
    }

    private int getSuitableResolution(int videoResolution) {
        int[] supportedResolutions = getSupportedResolutions();
        ArrayList<Integer> resolutionList = new ArrayList<Integer>();
        for (int res : supportedResolutions) {
            resolutionList.add(res);
        }
        if (FeatureOption.MTK_MT8193_HDMI_SUPPORT) {
            if (needUpdate(videoResolution)) {
                log(TAG, "8193 upate resolution");
                if (mEdid != null) {
                    int edidTemp = mEdid[0] | mEdid[1];
                    int[] prefered = HdmiDef.getPreferedResolutions();
                    for (int res : prefered) {
                        int act = res;
                        if (res >= HdmiDef.AUTO) {
                            act = res - HdmiDef.AUTO;
                        }
                        if (0 != (edidTemp & HdmiDef.sResolutionMask[act])
                                && resolutionList.contains(act)) {
                            videoResolution = res;
                            break;
                        }
                    }
                }
            }
        } else if (FeatureOption.MTK_INTERNAL_HDMI_SUPPORT
                || FeatureOption.MTK_INTERNAL_MHL_SUPPORT) {
            if (needUpdate(videoResolution)) {
                log(TAG, "8135 upate resolution");
                if (mEdid != null) {
                    int edidTemp = mEdid[0] | mEdid[1];
                    int[] prefered = HdmiDef.getPreferedResolutions();
                    for (int res : prefered) {
                        int act = res;
                        if (res >= HdmiDef.AUTO) {
                            act = res - HdmiDef.AUTO;
                        }
                        if (0 != (edidTemp & HdmiDef.sResolutionMask[act])
                                && resolutionList.contains(act)) {
                            videoResolution = res;
                            break;
                        }
                    }
                }
            }
        } else {
            if (videoResolution >= HdmiDef.AUTO) {
                log(TAG, "upate resolution");
                videoResolution = HdmiDef.AUTO
                        + HdmiDef.RESOLUTION_1280X720P_60HZ;
            }
        }
        log(TAG, "suiteable video resolution: " + videoResolution);
        return videoResolution;
    }

    private boolean needUpdate(int videoResolution) {
        log(TAG, "needUpdate: " + videoResolution);
        boolean needUpdate = true;
        if (mPreEdid != null && Arrays.equals(mEdid, mPreEdid)) {
            needUpdate = false;
        }
        if (videoResolution >= HdmiDef.AUTO) {
            needUpdate = true;
        }
        return needUpdate;
    }

    @Override
    public int[] getSupportedResolutions() {
        log(TAG, "getSupportedResolutions");
        return getSupportedResolutionsImpl();
    }

    private int[] getSupportedResolutionsImpl() {
        if (null == mEdid) {
            return getDriverSupportedResolutions();
        }
        if (FeatureOption.MTK_INTERNAL_HDMI_SUPPORT
                || FeatureOption.MTK_INTERNAL_MHL_SUPPORT) {
            return getDriverSupportedResolutions();
        }
        int edidTemp = mEdid[0] | mEdid[1];
        int[] resolutions = getDriverSupportedResolutions();
        ArrayList<Integer> list = new ArrayList<Integer>();
        for (int res : resolutions) {
            try {
                int mask = HdmiDef.sResolutionMask[res];
                if ((edidTemp & mask) != 0) {
                    if (!list.contains(res)) {
                        list.add(res);
                    }
                }
            } catch (ArrayIndexOutOfBoundsException e) {
                Xlog.w(TAG, e.getMessage());
            }
        }
        resolutions = new int[list.size()];
        for (int i = 0; i < list.size(); i++) {
            resolutions[i] = list.get(i);
        }
        log(TAG, "getSupportedResolutionsImpl: " + Arrays.toString(resolutions));
        return resolutions;
    }

    private int[] getDriverSupportedResolutions() {
        return getDefaultSupportedResolutions();
    }

    private int[] getDefaultSupportedResolutions() {
        int[] resolutions = HdmiDef.getDefaultResolutions();
        log(TAG, "getDefaultSupportedResolutions: "
                + Arrays.toString(resolutions));
        return resolutions;
    }

    @Override
    public int getDisplayType() {
        log(TAG, "getDisplayType");
        int ret = 0;
        if (mHdmiNative != null) {
            ret = mHdmiNative.getDisplayType();
        }
        return ret;
    }

    @Override
    public void notifyHdVideoState(boolean playing) {
        log(TAG, "notifyHdVideoState: " + playing);
        synchronized (MtkHdmiManagerService.this) {
            if (mIsHdVideoPlaying == playing) {
                return;
            } else {
                log(TAG, "mIsHdVideoPlaying: " + mIsHdVideoPlaying
                        + " mNeedRestore: " + mNeedRestore);
                mIsHdVideoPlaying = playing;
                if (!mIsHdVideoPlaying) {
                    if (mNeedRestore) {
                        mNeedRestore = false;
                        enableHdmi(true);
                    }
                }
            }
        }
    }

    private static void log(String tag, Object obj) {
        if (Log.isLoggable(tag, Log.INFO)) {
            Xlog.i(tag, obj.toString());
        }
    }

    private class HdmiObserver extends UEventObserver {
        private static final String TAG = "HdmiObserver";

        private static final String HDMI_UEVENT_MATCH = "DEVPATH=/devices/virtual/switch/hdmi";
        private static final String HDMI_STATE_PATH = "/sys/class/switch/hdmi/state";
        private static final String HDMI_NAME_PATH = "/sys/class/switch/hdmi/name";

        // Monitor OTG and notify HDMI
        private static final int MSG_HDMI = 0;
        private static final int MSG_OTG = 1;
        private static final String OTG_UEVENT_MATCH = "DEVPATH=/devices/virtual/switch/otg_state";
        private static final String OTG_STATE_PATH = "/sys/class/switch/otg_state/state";
        private static final String OTG_NAME_PATH = "/sys/class/switch/otg_state/name";
        private String mOtgName;

        private int mHdmiState;
        private int mPrevHdmiState;
        private String mHdmiName;

        private final Context mContext;
        private final WakeLock mWakeLock;

        public HdmiObserver(Context context) {
            mContext = context;
            PowerManager pm = (PowerManager) context
                    .getSystemService(Context.POWER_SERVICE);
            mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                    "HdmiObserver");
            mWakeLock.setReferenceCounted(false);
            init();
        }

        public void startObserve() {
            startObserving(HDMI_UEVENT_MATCH);
            // Monitor OTG
            startObserving(OTG_UEVENT_MATCH);
        }

        public void stopObserve() {
            stopObserving();
        }

        @Override
        public void onUEvent(UEventObserver.UEvent event) {
            log(TAG, "HdmiObserver: onUEvent: " + event.toString());
            String name = event.get("SWITCH_NAME");
            int state = 0;
            try {
                state = Integer.parseInt(event.get("SWITCH_STATE"));
            } catch (NumberFormatException e) {
                Xlog.w(TAG,
                        "HdmiObserver: Could not parse switch state from event "
                                + event);
            }
            log(TAG, "HdmiObserver.onUEvent(), name=" + name + ", state="
                    + state);
            if (name.equals(mOtgName)) {
                updateOtgState(state);
            } else {
                update(name, state);
            }
        }

        private void init() {
            String newName = mHdmiName;
            int newState = mHdmiState;
            mPrevHdmiState = mHdmiState;
            newName = getContentFromFile(HDMI_NAME_PATH);
            try {
                newState = Integer
                        .parseInt(getContentFromFile(HDMI_STATE_PATH));
            } catch (NumberFormatException e) {
                Xlog.w(TAG, "HDMI state fail");
            }
            update(newName, newState);
            initOtgState();
        }

        private String getContentFromFile(String filePath) {
            char[] buffer = new char[1024];
            FileReader reader = null;
            String content = null;
            try {
                reader = new FileReader(filePath);
                int len = reader.read(buffer, 0, buffer.length);
                content = String.valueOf(buffer, 0, len).trim();
                log(TAG, filePath + " content is " + content);
            } catch (FileNotFoundException e) {
                Xlog.w(TAG, "can't find file " + filePath);
            } catch (IOException e) {
                Xlog.w(TAG, "IO exception when read file " + filePath);
            } catch (IndexOutOfBoundsException e) {
                Xlog.w(TAG, "index exception: " + e.getMessage());
            } finally {
                if (null != reader) {
                    try {
                        reader.close();
                    } catch (IOException e) {
                        Xlog.w(TAG, "close reader fail: " + e.getMessage());
                    }
                }
            }
            return content;
        }

        private synchronized void update(String newName, int newState) {
            log(TAG, "HDMIOberver.update(), oldState=" + mHdmiState
                    + ", newState=" + newState);
            // Retain only relevant bits
            int hdmiState = newState;
            int newOrOld = hdmiState | mHdmiState;
            int delay = 0;
            // reject all suspect transitions: only accept state changes from:
            // - a: 0 HDMI to 1 HDMI
            // - b: 1 HDMI to 0 HDMI

            /**
             * HDMI states HDMI_STATE_NO_DEVICE HDMI_STATE_ACTIVE
             * 
             * Following are for MT8193
             * 
             * HDMI_STATE_PLUGIN_ONLY HDMI_STATE_EDID_UPDATE
             * HDMI_STATE_CEC_UPDATE
             */
            if (FeatureOption.MTK_MT8193_HDMI_SUPPORT) {
                if ((mHdmiState == hdmiState) && (3 != mHdmiState)) {
                    return;
                }
            } else {
                if (mHdmiState == hdmiState
                        || ((newOrOld & (newOrOld - 1)) != 0)) {
                    return;
                }
            }
            mHdmiName = newName;
            mPrevHdmiState = mHdmiState;
            mHdmiState = hdmiState;
            mWakeLock.acquire();
            mHandler.sendMessageDelayed(mHandler.obtainMessage(MSG_HDMI,
                    mHdmiState, mPrevHdmiState, mHdmiName), delay);
        }

        private synchronized void sendIntents(int hdmiState, int prevHdmiState,
                String hdmiName) {
            int curHdmi = 1;
            // int curHDMI = 3;
            sendIntent(curHdmi, hdmiState, prevHdmiState, hdmiName);
        }

        private void sendIntent(int hdmi, int hdmiState, int prevHdmiState,
                String hdmiName) {
            if ((hdmiState & hdmi) != (prevHdmiState & hdmi)) {
                Intent intent = new Intent(Intent.ACTION_HDMI_PLUG);
                intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY);
                int state = 0;
                if ((hdmiState & hdmi) != 0) {
                    state = 1;
                }
                intent.putExtra("state", state);
                intent.putExtra("name", hdmiName);
                log(TAG, "HdmiObserver: Broadcast HDMI event, state: " + state
                        + " name: " + hdmiName);
                mContext.sendStickyBroadcastAsUser(intent, UserHandle.ALL);
                MtkHdmiManagerService.this.mHandler.obtainMessage(
                        MSG_CABLE_STATE, state).sendToTarget();
            }
        }

        private final Handler mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                case MSG_HDMI:
                    sendIntents(msg.arg1, msg.arg2, (String) msg.obj);
                    break;
                case MSG_OTG:
                    handleOtgStateChanged(msg.arg1);
                    break;
                default:
                    super.handleMessage(msg);
                }
                mWakeLock.release();
            }
        };

        private void initOtgState() {
            mOtgName = getContentFromFile(OTG_NAME_PATH);
            int otgState = 0;
            try {
                otgState = Integer.parseInt(getContentFromFile(OTG_STATE_PATH));
            } catch (NumberFormatException e) {
                Xlog.w(TAG, "HDMI state fail");
            }
            Xlog.i(TAG, "HDMIObserver.initOtgState(), state=" + otgState
                    + ", name=" + mOtgName);
            updateOtgState(otgState);
        }

        private void updateOtgState(int otgState) {
            Xlog.i(TAG, "HDMIObserver.updateOtgState(), otgState=" + otgState);
            mWakeLock.acquire();
            Message msg = mHandler.obtainMessage(MSG_OTG);
            msg.arg1 = otgState;
            mHandler.sendMessage(msg);
        }

        private void handleOtgStateChanged(int otgState) {
            Xlog.i(TAG, "HDMIObserver.handleOtgStateChanged(), otgState="
                    + otgState);
            try {
                boolean ret = mHdmiNative.notifyOtgState(otgState);
                Xlog.i(TAG, "notifyOtgState: " + ret);
            } catch (Exception e) {
                Xlog.e(TAG, "", e);
            }
        }
    }

    private static class HdmiDef {
        public static final int AUTO = 100;
        public static final int RESOLUTION_720X480P_60HZ = 0;
        public static final int RESOLUTION_720X576P_50HZ = 1;
        public static final int RESOLUTION_1280X720P_60HZ = 2;
        public static final int RESOLUTION_1280X720P_50HZ = 3;
        public static final int RESOLUTION_1920X1080I_60HZ = 4;
        public static final int RESOLUTION_1920X1080I_50HZ = 5;
        public static final int RESOLUTION_1920X1080P_30HZ = 6;
        public static final int RESOLUTION_1920X1080P_25HZ = 7;
        public static final int RESOLUTION_1920X1080P_24HZ = 8;
        public static final int RESOLUTION_1920X1080P_23HZ = 9;
        public static final int RESOLUTION_1920X1080P_29HZ = 10;
        public static final int RESOLUTION_1920X1080P_60HZ = 11;
        public static final int RESOLUTION_1920X1080P_50HZ = 12;
        public static final int RESOLUTION_1280X720P3D_60HZ = 13;
        public static final int RESOLUTION_1280X720P3D_50HZ = 14;
        public static final int RESOLUTION_1920X1080I3D_60HZ = 15;
        public static final int RESOLUTION_1920X1080I3D_50HZ = 16;
        public static final int RESOLUTION_1920X1080P3D_24HZ = 17;
        public static final int RESOLUTION_1920X1080P3D_23HZ = 18;

        public static final int SINK_480P = (1 << 0);
        public static final int SINK_720P60 = (1 << 1);
        public static final int SINK_1080I60 = (1 << 2);
        public static final int SINK_1080P60 = (1 << 3);
        public static final int SINK_480P_1440 = (1 << 4);
        public static final int SINK_480P_2880 = (1 << 5);
        public static final int SINK_480I = (1 << 6);
        public static final int SINK_480I_1440 = (1 << 7);
        public static final int SINK_480I_2880 = (1 << 8);
        public static final int SINK_1080P30 = (1 << 9);
        public static final int SINK_576P = (1 << 10);
        public static final int SINK_720P50 = (1 << 11);
        public static final int SINK_1080I50 = (1 << 12);
        public static final int SINK_1080P50 = (1 << 13);
        public static final int SINK_576P_1440 = (1 << 14);
        public static final int SINK_576P_2880 = (1 << 15);
        public static final int SINK_576I = (1 << 16);
        public static final int SINK_576I_1440 = (1 << 17);
        public static final int SINK_576I_2880 = (1 << 18);
        public static final int SINK_1080P25 = (1 << 19);
        public static final int SINK_1080P24 = (1 << 20);
        public static final int SINK_1080P23976 = (1 << 21);
        public static final int SINK_1080P2997 = (1 << 22);

        public static int[] sResolutionMask = new int[] { SINK_480P, SINK_576P,
                SINK_720P60, SINK_720P50, SINK_1080I60, SINK_1080I50,
                SINK_1080P30, SINK_1080P25, SINK_1080P24, SINK_1080P23976,
                SINK_1080P2997, SINK_1080P60, SINK_1080P50 };

        public static int[] getDefaultResolutions() {
            int[] resolutions;
            if (FeatureOption.MTK_MT8193_HDMI_SUPPORT) {
                resolutions = new int[] { RESOLUTION_1280X720P_60HZ,
                        RESOLUTION_1280X720P_50HZ, RESOLUTION_1920X1080P_24HZ,
                        RESOLUTION_1920X1080P_23HZ };
            } else if (FeatureOption.MTK_INTERNAL_HDMI_SUPPORT
                    || FeatureOption.MTK_INTERNAL_MHL_SUPPORT) {
                resolutions = new int[] { RESOLUTION_1920X1080P_60HZ,
                        RESOLUTION_1920X1080P_50HZ, RESOLUTION_1920X1080P_30HZ,
                        RESOLUTION_1920X1080P_25HZ, RESOLUTION_1920X1080P_24HZ,
                        RESOLUTION_1920X1080P_23HZ, RESOLUTION_1920X1080I_60HZ,
                        RESOLUTION_1920X1080I_50HZ, RESOLUTION_1280X720P_60HZ,
                        RESOLUTION_1280X720P_50HZ, RESOLUTION_720X480P_60HZ,
                        RESOLUTION_720X576P_50HZ };
            } else {
                resolutions = new int[] { RESOLUTION_1920X1080P_30HZ,
                        RESOLUTION_1280X720P_60HZ, RESOLUTION_720X480P_60HZ };
            }
            return resolutions;
        }

        public static int[] getPreferedResolutions() {
            int[] prefered = null;
            if (FeatureOption.MTK_MT8193_HDMI_SUPPORT) {
                prefered = new int[] { AUTO + RESOLUTION_1280X720P_60HZ,
                        AUTO + RESOLUTION_1280X720P_50HZ,
                        AUTO + RESOLUTION_720X480P_60HZ,
                        AUTO + RESOLUTION_720X576P_50HZ };
            } else if (FeatureOption.MTK_INTERNAL_HDMI_SUPPORT
                    || FeatureOption.MTK_INTERNAL_MHL_SUPPORT) {
                prefered = new int[] { AUTO + RESOLUTION_1920X1080P_60HZ,
                        AUTO + RESOLUTION_1920X1080P_50HZ,
                        AUTO + RESOLUTION_1920X1080P_30HZ,
                        AUTO + RESOLUTION_1920X1080P_25HZ,
                        AUTO + RESOLUTION_1920X1080P_24HZ,
                        AUTO + RESOLUTION_1920X1080P_23HZ,
                        AUTO + RESOLUTION_1920X1080I_60HZ,
                        AUTO + RESOLUTION_1920X1080I_50HZ,
                        AUTO + RESOLUTION_1280X720P_60HZ,
                        AUTO + RESOLUTION_1280X720P_50HZ,
                        AUTO + RESOLUTION_720X480P_60HZ,
                        AUTO + RESOLUTION_720X576P_50HZ };
            } else {
                prefered = new int[] { AUTO + RESOLUTION_1280X720P_60HZ,
                        AUTO + RESOLUTION_1920X1080P_30HZ,
                        AUTO + RESOLUTION_720X480P_60HZ };
            }
            return prefered;
        }
    }

}
