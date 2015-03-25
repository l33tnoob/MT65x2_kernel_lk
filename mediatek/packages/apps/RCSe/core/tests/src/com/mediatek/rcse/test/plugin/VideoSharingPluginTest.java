/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.test.plugin;

import android.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Instrumentation.ActivityMonitor;

import android.bluetooth.BluetoothHeadset;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothProfile.ServiceListener;

import android.os.IBinder;
import android.os.RemoteException;
import android.os.PowerManager.WakeLock;
import android.provider.MediaStore;
import android.test.ActivityInstrumentationTestCase2;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.RelativeLayout;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.hardware.Camera;

import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.phone.CallScreenDialog;
import com.mediatek.rcse.plugin.phone.ICallScreenHost;
import com.mediatek.rcse.plugin.phone.SharingPlugin;
import com.mediatek.rcse.plugin.phone.VideoSharingPlugin;
import com.mediatek.rcse.plugin.phone.RichcallProxyActivity;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.SessionState;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;
import com.orangelabs.rcs.service.api.client.media.IVideoEventListener;
import com.orangelabs.rcs.service.api.client.media.IVideoPlayer;
import com.orangelabs.rcs.service.api.client.media.IVideoRenderer;
import com.orangelabs.rcs.service.api.client.media.MediaCodec;
import com.orangelabs.rcs.service.api.client.media.video.VideoSurfaceView;
import com.orangelabs.rcs.service.api.client.richcall.IVideoSharingEventListener;
import com.orangelabs.rcs.service.api.client.richcall.IVideoSharingSession;
import com.orangelabs.rcs.service.api.client.richcall.RichCallApi;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Test case for CallLogExtention.
 */
public class VideoSharingPluginTest extends ActivityInstrumentationTestCase2<RichcallProxyActivity> {

    private static final String TAG = "VideoSharingPluginTest";
    private static final int CONTROLL_START = 1;
    private static final int CONTROLL_STOP = 2;
    private static final int STORED_VIDEO_STATUS_PLAY = 0;
    private static final long SLEEP_TIME = 200;
    private static final long TIME_OUT = 2000;
    private static final int CONNECTED = 2;
    private static final String NUMBER = "+34200000253";
    private static final String CONTACT_CAPABILITIES = "com.orangelabs.rcs.capability.CONTACT_CAPABILITIES";
    private static final String SHARE_STATUS = "mShareStatus";
    private static final int REQUEST_CODE_VIDEO = 12;
    private static final String SESSION_ID = "sessionId";
    private static final String MEDIA_TYPE = "mediatype";
    private static final String VIDEO_LIVE = "videolive";
    private static final String VIDEO_SHARING_INVITATION_ACTION = "com.orangelabs.rcs.richcall.VIDEO_SHARING_INVITATION";
    private static final String VIDEO_SHARING_START_ACTION = "com.mediatek.phone.plugin.VIDEO_SHARING_START_ACTION";
    private static boolean ALLOW_COMPILE = true;
    private ListAdapter mAdapter;
    private VideoSharingPlugin videoSharingPlugin;

    public VideoSharingPluginTest() {
        super(RichcallProxyActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        videoSharingPlugin = new VideoSharingPlugin(getActivity());
    }

    @Override
    protected void tearDown() throws Exception {
        Method method = Utils.getPrivateMethod(VideoSharingPlugin.class, "destroy");
        method.invoke(videoSharingPlugin);
        super.tearDown();
    }

    public void testCase1_VideoShareNotSupported() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase1_VideoShareNotSupported() entry!");
        Field field = Utils.getPrivateField(VideoSharingPlugin.class, "mBluetoothProfileServiceListener");
        BluetoothProfile.ServiceListener listener = (ServiceListener) field.get(videoSharingPlugin);
        listener.onServiceConnected(0, null);
        listener.onServiceDisconnected(0);
        Method method = Utils.getPrivateMethod(VideoSharingPlugin.class, "isBluetoothAvailable");
        method.invoke(videoSharingPlugin);
        videoSharingPlugin.start(NUMBER);
        videoSharingPlugin.start(NUMBER);
        videoSharingPlugin.stop();
        videoSharingPlugin.getState();
        videoSharingPlugin = new MockVideoSharePlugin(getActivity());
        videoSharingPlugin.start(NUMBER);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 4);
        videoSharingPlugin.start(NUMBER);
        com.mediatek.rcse.plugin.phone.Utils.setInImageSharing(true);
        videoSharingPlugin.start(NUMBER);
    }

    public void testCase2_ControllButton() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException {
       
        if(ALLOW_COMPILE)return; Logger.d(TAG, "testCase2_ControllButton() entry!");
        Field areaField = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingDisplayArea");
        areaField.set(videoSharingPlugin, new RelativeLayout(getActivity()));
        Field playerField = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingVideoPlayer");
        playerField.set(videoSharingPlugin, new MockMediaPlayer());
        Method method = Utils
                .getPrivateMethod(VideoSharingPlugin.class, "showControllButton", int.class, boolean.class);
        method.invoke(videoSharingPlugin, CONTROLL_START, true);
        Field field = Utils.getPrivateField(VideoSharingPlugin.class, "mControllButton");
        Object controllButton = (ImageView) field.get(videoSharingPlugin);
        assertNotNull(controllButton);

        Field listenerField = Utils.getPrivateField(controllButton.getClass(), "mListener");
        OnClickListener listener = (OnClickListener) listenerField.get(controllButton);
        listener.onClick(null);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mControllButton");
        field.set(videoSharingPlugin, null);
        controllButton = (ImageView) field.get(videoSharingPlugin);
        assertNull(controllButton);

        method.invoke(videoSharingPlugin, CONTROLL_STOP, true);
        controllButton = (ImageView) field.get(videoSharingPlugin);
        assertNotNull(controllButton);

        listener.onClick(null);

        method.invoke(videoSharingPlugin, CONTROLL_STOP, false);
        controllButton = (ImageView) field.get(videoSharingPlugin);
        assertNotNull(controllButton);

        areaField.set(videoSharingPlugin, null);
        field.set(videoSharingPlugin, null);
        method.invoke(videoSharingPlugin, CONTROLL_STOP, true);
        method.invoke(videoSharingPlugin, CONTROLL_STOP, false);
    }

    public void testCase3_PlayingProgress() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, NoSuchMethodException, InvocationTargetException {}

    public void testCase4_ReceivedStatus() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException {}

    public void testCase5_seekToStoredVideo() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException {}

    public void testCase6_loadThumbnail() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException {}

    public void testCase7_VideoSharingDialogManager() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, NoSuchMethodException, InvocationTargetException, InterruptedException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase7_VideoSharingDialogManager() entry!");
        VideoSharingPlugin videoSharingPlugin = new VideoSharingPlugin(getActivity());
        Field field = Utils.getPrivateField(VideoSharingPlugin.class, "mVideoSharingDialogManager");
        Object dialogManager = field.get(videoSharingPlugin);
        assertNotNull(dialogManager);
        field = Utils.getPrivateField(dialogManager.getClass(), "mOnClickListener");
        DialogInterface.OnClickListener listener = (DialogInterface.OnClickListener) field.get(dialogManager);
        assertNotNull(listener);
        listener.onClick(null, -1);
        listener.onClick(null, -2);
        listener.onClick(null, 0);

        Method method = Utils.getPrivateMethod(dialogManager.getClass(), "showTerminatedByRemoteDialog");
        method.invoke(dialogManager);

        Method terminatedByNetWorkMethod = Utils.getPrivateMethod(dialogManager.getClass(),
                "showTerminatedByNetworkDialog");
        Method waitingInitializeConextProgressMethod = Utils.getPrivateMethod(dialogManager.getClass(),
                "showWaitingInitializeConextProgressDialog");
        terminatedByNetWorkMethod.invoke(dialogManager);
        waitingInitializeConextProgressMethod.invoke(dialogManager);

        videoSharingPlugin.setCallScreenHost(new MockCallScreenHost(getActivity()));
        WaitForNotNull(videoSharingPlugin.getCallScreenHost());
        terminatedByNetWorkMethod.invoke(dialogManager);

        method = Utils.getPrivateMethod(dialogManager.getClass(), "showRejectedByRemoteDialog");
        method.invoke(dialogManager);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 3);
        method.invoke(dialogManager);
        Thread.sleep(SLEEP_TIME);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIsVideoSharingSender");
        field.set(videoSharingPlugin, new AtomicBoolean(true));
        method.invoke(dialogManager);

        field = Utils.getPrivateField(dialogManager.getClass(), "mRejectDialogListener");
        listener = (DialogInterface.OnClickListener) field.get(dialogManager);
        listener.onClick(null, -1);
        listener.onClick(null, -2);
        listener.onClick(null, 0);

        method = Utils.getPrivateMethod(dialogManager.getClass(), "showTimeOutDialog");
        method.invoke(dialogManager);
        Thread.sleep(SLEEP_TIME);

        method = Utils.getPrivateMethod(dialogManager.getClass(), "showSelectVideoDialog");
        method.invoke(dialogManager);
        MockCallScreenDialog callScreenDialog = new MockCallScreenDialog(getActivity());
        field = Utils.getPrivateField(dialogManager.getClass(), "mSelectionDialog");
        field.set(dialogManager, callScreenDialog);

        method.invoke(dialogManager);
        assertNotNull(mAdapter);
        assertNotNull(mAdapter.getView(0, null, null));

        field = Utils.getPrivateField(dialogManager.getClass(), "mSingleChoiceListener");
        listener = (DialogInterface.OnClickListener) field.get(dialogManager);
        MockDialog dialog = new MockDialog(getActivity());
        dialog.show();
        listener.onClick(dialog, -2);
        dialog.show();
        listener.onClick(dialog, 0);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 2);
        dialog.show();
        listener.onClick(dialog, 0);
        dialog.show();
        ActivityMonitor monitor = new ActivityMonitor(RichcallProxyActivity.class.getName(), null, false);
        getInstrumentation().addMonitor(monitor);
        Activity activity = null;
        try {
            listener.onClick(dialog, 1);
            activity = (RichcallProxyActivity) monitor.waitForActivityWithTimeout(TIME_OUT);
            assertNotNull(activity);
        } finally {
            if (null != activity) {
                activity.finishActivity(REQUEST_CODE_VIDEO);
            }
        }

        method = Utils.getPrivateMethod(dialogManager.getClass(), "showInvitationDialog");
        method.invoke(dialogManager);
        videoSharingPlugin.setCallScreenHost(null);
        method.invoke(dialogManager);
        method = Utils.getPrivateMethod(dialogManager.getClass(), "createAndShowAlertDialog", String.class,
                DialogInterface.OnClickListener.class);
        method.invoke(dialogManager, null, null);
    }

    public void testCase8_getIncomingVideoSharingSession() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase8_getIncomingVideoSharingSession() entry!");
        Method method = Utils.getPrivateMethod(VideoSharingPlugin.class, "getIncomingVideoSharingSession");
        method.invoke(videoSharingPlugin);
        Field field = Utils.getPrivateField(SharingPlugin.class, "mRichCallApi");
        field.set(videoSharingPlugin, new MockRichCallApi(getActivity()));
        method.invoke(videoSharingPlugin);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIncomingSessionId");
        field.set(videoSharingPlugin, "");
        method.invoke(videoSharingPlugin);
    }

     /* private void getVideoCapability() throws Throwable {
        PluginApiManager.initialize(getActivity());
        Field field = Utils.getPrivateField(PluginApiManager.class, "mIsRegistered");
        field.set(PluginApiManager.getInstance(), true);


        field = Utils.getPrivateField(SharingPlugin.class, "mRichCallStatus");
        field.set(videoSharingPlugin, CONNECTED);

        RichCallApi richCallApi = new MockRichCallApi(getActivity());
        field = Utils.getPrivateField(SharingPlugin.class, "mRichCallApi");
        field.set(videoSharingPlugin, richCallApi);

        RegistrationApi registrationApi = new RegistrationApi(getActivity());
        field = Utils.getPrivateField(PluginApiManager.class, "mRegistrationApi");
        field.set(PluginApiManager.getInstance(), registrationApi);

        Capabilities capabilities = new Capabilities();
        capabilities.setRcseContact(true);
        Intent intent = new Intent(CONTACT_CAPABILITIES);
        intent.putExtra("contact", NUMBER);
        intent.putExtra("capabilities", capabilities);
        field = Utils.getPrivateField(PluginApiManager.class, "mBroadcastReceiver");
        BroadcastReceiver broadcastReceiver = (BroadcastReceiver) field.get(PluginApiManager.getInstance());
        broadcastReceiver.onReceive(getActivity(), intent);

        long startTime = System.currentTimeMillis();
        while (!videoSharingPlugin.getCapability(NUMBER)) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }
    }
    */
    
    /**
     * Use to test the VideoSharingPlugin function getCapability()
     */
   /*  public void testCase9_getVideoCapability() throws Throwable {
        if(ALLOW_COMPILE)return ; Logger.v(TAG, "testCase9_getVideoCapability()");
        getVideoCapability();
        Method method = Utils.getPrivateMethod(videoSharingPlugin.getClass(), "destroy");
        method.invoke(videoSharingPlugin);
    }
   */
    /**
     * Use to test the startVideoShare function
     */
   /* public void testCase10_startVideoShare() throws Throwable {
        if(ALLOW_COMPILE)return ; Logger.v(TAG, "testCase10_startVideoShare()");
        Method method = Utils.getPrivateMethod(VideoSharingPlugin.class, "startByNetwork", int.class);
        method.invoke(videoSharingPlugin, 16);
        method.invoke(videoSharingPlugin, 2);
        method.invoke(videoSharingPlugin, 10);
        method.invoke(videoSharingPlugin, 0);
        method.invoke(videoSharingPlugin, -1);

        Field field = Utils.getPrivateField(VideoSharingPlugin.class, "mIncomingVideoSharingSession");
        field.set(videoSharingPlugin, new MockVideoSharingSession());
        method.invoke(videoSharingPlugin, 2);
        field = Utils.getPrivateField(SharingPlugin.class, "mRichCallApi");
        field.set(videoSharingPlugin, new MockRichCallApi(getActivity()));
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 1);
        method.invoke(videoSharingPlugin, 2);
    }
*/
 /*   public void testCase11_onClick() {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase11_onClick() entry!");
        videoSharingPlugin.onClick(new MockDialog(getActivity()), 0);
        videoSharingPlugin.onClick(new MockDialog(getActivity()), -1);
        videoSharingPlugin.onClick(new MockDialog(getActivity()), -2);
    }
*/
    public void testCase12_handleSecondReceiveInvitation() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException, RemoteException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase12_handleSecondReceiveInvitation() entry!");
        videoSharingPlugin.setCallScreenHost(new MockCallScreenHost(getActivity()));
        Field field = Utils.getPrivateField(VideoSharingPlugin.class, "mIncomingVideoSharingSession");
        field.set(videoSharingPlugin, new MockVideoSharingSession());
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingDisplayArea");
        field.set(videoSharingPlugin, new RelativeLayout(getActivity()));
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mSurfaceHolder");
        field.set(videoSharingPlugin, new MockSurfaceHolder());
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingLocalVideoSurfaceView");
        field.set(videoSharingPlugin, new VideoSurfaceView(getActivity()));
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingRemoteVideoRenderer");
        field.set(videoSharingPlugin, new MockMediaRenderer());
        Method method = Utils.getPrivateMethod(VideoSharingPlugin.class, "handleFirstReceiveInvitation");
        method.invoke(videoSharingPlugin);

        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "handleSecondReceiveInvitation");
        method.invoke(videoSharingPlugin);
        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "handleUserAcceptVideoSharing");
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIsVideoSharingSender");
        field.set(videoSharingPlugin, new AtomicBoolean(true));
        method.invoke(videoSharingPlugin);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIsVideoSharingReceiver");
        field.set(videoSharingPlugin, new AtomicBoolean(true));
        method.invoke(videoSharingPlugin);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingRemoteVideoSurfaceViewOnReceiveDataListener");
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mVideoSurfaceViewListener");
        Bitmap map = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.alert_dark_frame);

    }

    public void testCase13_handleUserDeclineVideoSharing() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase13_handleUserDeclineVideoSharing() entry!");
        Field field = Utils.getPrivateField(SharingPlugin.class, "mWakeLock");
        WakeLock wakeLock = (WakeLock) field.get(videoSharingPlugin);
        wakeLock.acquire();
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 3);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIncomingVideoSharingSession");
        field.set(videoSharingPlugin, new MockVideoSharingSession());
        Method method = Utils.getPrivateMethod(VideoSharingPlugin.class, "handleUserDeclineVideoSharing");
        method.invoke(videoSharingPlugin);
    }

    public void testCase14_is3GMobileNetwork() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase13_handleUserDeclineVideoSharing() entry!");
        Method method = Utils.getPrivateMethod(VideoSharingPlugin.class, "is3GMobileNetwork");
        method.invoke(videoSharingPlugin);
    }

    public void testCase15_handleVideoSharingInvitation() throws Throwable {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase15_handleVideoSharingInvitation() entry!");
        videoSharingPlugin = new MockVideoSharePlugin(getActivity());
        Method method = Utils.getPrivateMethod(VideoSharingPlugin.class, "handleVideoSharingInvitation", Context.class,
                Intent.class);
        Field field = Utils.getPrivateField(SharingPlugin.class, "mRichCallApi");
        field.set(videoSharingPlugin, new MockRichCallApi(getActivity()));
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingVideoSharingSession");
        field.set(videoSharingPlugin, new MockVideoSharingSession());
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIncomingSessionId");
        field.set(videoSharingPlugin, "");
        Intent intent = new Intent();
        intent.putExtra(SESSION_ID, "");
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 1);
        method.invoke(videoSharingPlugin, getActivity(), intent);

        field = Utils.getPrivateField(SharingPlugin.class, "mNumber");
        field.set(videoSharingPlugin, NUMBER);
        //getVideoCapability();
        intent.putExtra(MEDIA_TYPE, VIDEO_LIVE);
        method.invoke(videoSharingPlugin, getActivity(), intent);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingVideoSharingSession");
        field.set(videoSharingPlugin, null);
        method.invoke(videoSharingPlugin, getActivity(), intent);

        intent.putExtra(MEDIA_TYPE, "");
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 1);
        videoSharingPlugin.setCallScreenHost(new MockCallScreenHost(getActivity()));
        method.invoke(videoSharingPlugin, getActivity(), intent);
    }

    public void testCase16_handleVideoSharingInvitation() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException, NoSuchMethodException, InvocationTargetException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase16_handleVideoSharingInvitation() entry!");
        Field field = Utils.getPrivateField(VideoSharingPlugin.class, "mVideoSharingInvitationReceiver");
        Object receiver = field.get(videoSharingPlugin);
        Method method = Utils.getPrivateMethod(receiver.getClass(), "onReceive", Context.class, Intent.class);
        method.invoke(receiver, getActivity(), null);

        Intent intent = new Intent();
        method.invoke(receiver, getActivity(), intent);

        intent.setAction(VIDEO_SHARING_INVITATION_ACTION);
        method.invoke(receiver, getActivity(), intent);

        intent.setAction(VIDEO_SHARING_START_ACTION);
        method.invoke(receiver, getActivity(), intent);

        intent.setAction(intent.ACTION_HEADSET_PLUG);
        method.invoke(receiver, getActivity(), intent);

        intent.setAction(BluetoothHeadset.ACTION_CONNECTION_STATE_CHANGED);
        method.invoke(receiver, getActivity(), intent);
    }

    public void testCase17_switchCamera() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase17_switchCamera() entry!");
        Method method = Utils.getPrivateMethod(VideoSharingPlugin.class, "switchCamera");
        method.invoke(videoSharingPlugin);
        Field field = Utils.getPrivateField(VideoSharingPlugin.class, "mOpenedCameraId");
        field.set(videoSharingPlugin, 1);
        method.invoke(videoSharingPlugin);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mCamerNumber");
        field.set(videoSharingPlugin, 1);
        method.invoke(videoSharingPlugin);
    }

    public void testCase18_destroyOutgoingViewOnly() throws IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, NoSuchMethodException, NoSuchFieldException, InterruptedException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase18_destroyOutgoingViewOnly() entry!");
        Method method = Utils.getPrivateMethod(VideoSharingPlugin.class, "destroyOutgoingViewOnly");
        Field field = Utils.getPrivateField(VideoSharingPlugin.class, "mIncomingDisplayArea");
        field.set(videoSharingPlugin, new MockViewGroup(getActivity()));
        method.invoke(videoSharingPlugin);
        Thread.sleep(SLEEP_TIME);
        field.set(videoSharingPlugin, null);
        method.invoke(videoSharingPlugin);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIncomingRemoteVideoSurfaceView");
        field.set(videoSharingPlugin, new VideoSurfaceView(getActivity()));
        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "removeSurfaceViewListener");
        method.invoke(videoSharingPlugin);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingVideoSharingSession");
        field.set(videoSharingPlugin, new MockVideoSharingSession());
        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "cancelSession");
        method.invoke(videoSharingPlugin);
        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "showWaitRemoteAcceptMessage");
        method.invoke(videoSharingPlugin);
        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "showPlayingProgress", boolean.class);
        method.invoke(videoSharingPlugin, true);
        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "updatePlayingProgress", long.class);
        method.invoke(videoSharingPlugin, 0);
        Thread.sleep(SLEEP_TIME);
        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "showRemoteAcceptMessage");
        method.invoke(videoSharingPlugin);
        Thread.sleep(SLEEP_TIME);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 4);
        method.invoke(videoSharingPlugin);
        Thread.sleep(SLEEP_TIME);
    }

    public void testCase19_showReceiverLocalView() throws IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, NoSuchMethodException, NoSuchFieldException, RemoteException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase19_showReceiverLocalView() entry!");
        Method localMethod = Utils.getPrivateMethod(VideoSharingPlugin.class, "showReceiverLocalView");
        Method remoteMethod = Utils.getPrivateMethod(VideoSharingPlugin.class, "showReceiverRemoteView");
        Field field = Utils.getPrivateField(SharingPlugin.class, "mRichCallApi");
        field.set(videoSharingPlugin, new MockRichCallApi(getActivity()));
        videoSharingPlugin.setCallScreenHost(new MockCallScreenHost(getActivity()));
        Field outgoingDisplayAreaField = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingDisplayArea");
        outgoingDisplayAreaField.set(videoSharingPlugin, new RelativeLayout(getActivity()));
        localMethod.invoke(videoSharingPlugin);
        MockViewGroup groupView = new MockViewGroup(getActivity());
        Field incomingDisplayAreaField = Utils.getPrivateField(VideoSharingPlugin.class, "mIncomingDisplayArea");
        incomingDisplayAreaField.set(videoSharingPlugin, groupView);
        localMethod.invoke(videoSharingPlugin);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mVideoSurfaceViewListener");
        Bitmap map = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.alert_dark_frame);


        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIncomingRemoteVideoSurfaceView");
        field.set(videoSharingPlugin, new VideoSurfaceView(getActivity()));
        remoteMethod.invoke(videoSharingPlugin);
        groupView.removeAllViews();
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 4);
        remoteMethod.invoke(videoSharingPlugin);

        outgoingDisplayAreaField.set(videoSharingPlugin, null);
        incomingDisplayAreaField.set(videoSharingPlugin, null);
        localMethod.invoke(videoSharingPlugin);
    }

    public void testCase20_initializeReceiverRemoteView() throws IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, NoSuchMethodException, NoSuchFieldException, RemoteException,
            InterruptedException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase20_initializeReceiverRemoteView() entry!");
        Method method = Utils.getPrivateMethod(VideoSharingPlugin.class, "initializeReceiverRemoteView");
        Field field = Utils.getPrivateField(SharingPlugin.class, "mRichCallApi");
        field.set(videoSharingPlugin, new MockRichCallApi(getActivity()));
        MockViewGroup groupView = new MockViewGroup(getActivity());
        Field incomingDisplayAreaField = Utils.getPrivateField(VideoSharingPlugin.class, "mIncomingDisplayArea");
        incomingDisplayAreaField.set(videoSharingPlugin, groupView);
        method.invoke(videoSharingPlugin);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mVideoSurfaceViewListener");
        Bitmap map = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.alert_dark_frame);
        Thread.sleep(SLEEP_TIME);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 4);
        Thread.sleep(SLEEP_TIME);
        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "showPlayingProgress", boolean.class);
        method.invoke(videoSharingPlugin, true);
        Thread.sleep(SLEEP_TIME);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIncomingRemoteVideoSurfaceViewOnReceiveDataListener");
        Thread.sleep(SLEEP_TIME);
        Field statusField = Utils.getPrivateField(VideoSharingPlugin.class, "mStoredVideoStatusView");
        statusField.set(videoSharingPlugin, null);
        Thread.sleep(SLEEP_TIME);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 0);
        Thread.sleep(SLEEP_TIME);
        Thread.sleep(SLEEP_TIME);
    }

    public void testCase21_showSenderStoredView() throws IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, NoSuchMethodException, NoSuchFieldException, RemoteException,
            InterruptedException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase21_showSenderStoredView() entry!");
        Method method = Utils.getPrivateMethod(VideoSharingPlugin.class, "showSenderStoredView");
        Field outgoingDisplayAreaField = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingDisplayArea");
        outgoingDisplayAreaField.set(videoSharingPlugin, new RelativeLayout(getActivity()));
        Field field = Utils.getPrivateField(SharingPlugin.class, "mRichCallApi");
        field.set(videoSharingPlugin, new MockRichCallApi(getActivity()));
        videoSharingPlugin.setCallScreenHost(new MockCallScreenHost(getActivity()));
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingVideoPlayer");
        field.set(videoSharingPlugin, new MockMediaPlayer());
        method.invoke(videoSharingPlugin);
        String videoPath = Utils.getFilePath(getActivity(), MediaStore.Video.Media.EXTERNAL_CONTENT_URI);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mVideoFileName");
        field.set(videoSharingPlugin, videoPath);
        method.invoke(videoSharingPlugin);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mVideoSurfaceViewListener");
        Bitmap map = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.alert_dark_frame);

        Method playingProgressMethod = Utils.getPrivateMethod(VideoSharingPlugin.class, "showPlayingProgress",
                boolean.class);
        playingProgressMethod.invoke(videoSharingPlugin, true);

        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "showSenderLocalView");
        method.invoke(videoSharingPlugin);
        Thread.sleep(SLEEP_TIME);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIncomingDisplayArea");
        field.set(videoSharingPlugin, new MockViewGroup(getActivity()));
        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "removeIncomingVideoSharingVews");
        method.invoke(videoSharingPlugin);
        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "startCameraPreview");
        method.invoke(videoSharingPlugin);
        videoSharingPlugin.surfaceDestroyed(null);
    }

    public void testCase22_surfaceChanged() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase22_surfaceChanged() entry!");
        Field field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 2);
        videoSharingPlugin.surfaceChanged(null, 0, 0, 0);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mCameraPreviewRunning");
        field.set(videoSharingPlugin, true);
        videoSharingPlugin.surfaceCreated(null);
        videoSharingPlugin.surfaceChanged(null, 0, 0, 0);
    }

    public void testCase23_addPreviewCallback() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase23_addPreviewCallback() entry!");
        Method method = Utils.getPrivateMethod(VideoSharingPlugin.class, "addPreviewCallback");
        method.invoke(videoSharingPlugin);
        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "onCamreaPreviewFrame", byte[].class, Camera.class);
        method.invoke(videoSharingPlugin, new byte[1], null);
        Field playerField = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingVideoPlayer");
        playerField.set(videoSharingPlugin, new MockMediaPlayer());
        method.invoke(videoSharingPlugin, new byte[1], null);
    }

    public void testCase24_IVideoSharingEventListener() throws IllegalArgumentException, IllegalAccessException,
            RemoteException, NoSuchFieldException, InterruptedException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase24_IVideoSharingEventListener() entry!");
        videoSharingPlugin = new MockVideoSharePlugin(getActivity());
        Field field = Utils.getPrivateField(VideoSharingPlugin.class, "mIncomingSessionEventListener");
        IVideoSharingEventListener listener = (IVideoSharingEventListener) field.get(videoSharingPlugin);
        listener.handleSessionStarted();
        listener.handleSessionAborted(0);
        listener.handleSessionTerminatedByRemote();
        listener.handleSharingError(4);
        listener.handleSharingError(3);
        listener.handleSharingError(2);
        listener.handleSharingError(1);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 3);
        listener.handleSessionAborted(0);

        field.set(videoSharingPlugin, 4);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mBluetoothHeadset");
        field.set(videoSharingPlugin, null);
        listener.handleSessionStarted();
        Thread.sleep(SLEEP_TIME);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingSessionEventListener");
        listener = (IVideoSharingEventListener) field.get(videoSharingPlugin);
        listener.handleSessionStarted();
        listener.handleSessionAborted(0);
        listener.handleSessionTerminatedByRemote();
        listener.handleSharingError(4);
        listener.handleSharingError(3);
        listener.handleSharingError(2);
        listener.handleSharingError(10);
        listener.handleSharingError(0);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingVideoSharingSession");
        field.set(videoSharingPlugin, new MockVideoSharingSession());
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIncomingVideoSharingSession");
        field.set(videoSharingPlugin, new MockVideoSharingSession());
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 5);
        listener.handleSessionStarted();

        ((MockVideoSharePlugin) videoSharingPlugin).is3GSupport = false;
        videoSharingPlugin.setCallScreenHost(new MockCallScreenHost(getActivity()));
        listener.handleSessionStarted();
        Thread.sleep(SLEEP_TIME);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIsVideoSharingReceiver");
        field.set(videoSharingPlugin, new AtomicBoolean(true));
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 3);
        listener.handleSessionTerminatedByRemote();
        listener.handleSessionAborted(0);
        listener.handleSharingError(2);
        listener.handleSharingError(10);
        listener.handleSharingError(0);
        Thread.sleep(SLEEP_TIME);
    }

    public void testCase25_handleStartVideoSharingFailed() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException, InterruptedException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase25_handleStartVideoSharingFailed() entry!");
        Method method = Utils.getPrivateMethod(VideoSharingPlugin.class, "handleStartVideoSharingFailed");
        method.invoke(videoSharingPlugin);

        Field field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        assertTrue(field.get(videoSharingPlugin).equals(0));
        field.set(videoSharingPlugin, 3);
        method.invoke(videoSharingPlugin);
        assertTrue(field.get(videoSharingPlugin).equals(2));

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIsVideoSharingSender");
        field.set(videoSharingPlugin, new AtomicBoolean(true));
        method.invoke(videoSharingPlugin);

        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "startOutgoingVideoShareSession");
        method.invoke(videoSharingPlugin);
        Thread.sleep(SLEEP_TIME);
        field = Utils.getPrivateField(SharingPlugin.class, "mRichCallApi");
        field.set(videoSharingPlugin, new MockRichCallApi(getActivity()));
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingVideoPlayer");
        method.invoke(videoSharingPlugin);
        Thread.sleep(SLEEP_TIME);
        field.set(videoSharingPlugin, new MockMediaPlayer());
        method.invoke(videoSharingPlugin);
        Thread.sleep(SLEEP_TIME);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 1);
        method.invoke(videoSharingPlugin);
        Thread.sleep(SLEEP_TIME);

        field = Utils.getPrivateField(SharingPlugin.class, "mNumber");
        field.set(videoSharingPlugin, NUMBER);
        method.invoke(videoSharingPlugin);
        Thread.sleep(SLEEP_TIME);
    }

    public void testCase26_dismissDialog() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase26_dismissAllDialogs() entry!");
        Method method = Utils.getPrivateMethod(VideoSharingPlugin.class, "dismissDialog");
        method.invoke(videoSharingPlugin);
        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "alreadyOnGoing");
        method.invoke(videoSharingPlugin);

        Field field = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingDisplayArea");
        field.set(videoSharingPlugin, new RelativeLayout(getActivity()));
        field = Utils.getPrivateField(SharingPlugin.class, "mRichCallApi");
        field.set(videoSharingPlugin, new MockRichCallApi(getActivity()));
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mOutgoingVideoPlayer");
        field.set(videoSharingPlugin, new MockMediaPlayer());
        videoSharingPlugin.setCallScreenHost(new MockCallScreenHost(getActivity()));
        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "showLocalView");
        method.invoke(videoSharingPlugin);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIsVideoSharingSender");
        field.set(videoSharingPlugin, new AtomicBoolean(true));
        method.invoke(videoSharingPlugin);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIsVideoSharingSender");
        field.set(videoSharingPlugin, new AtomicBoolean(false));
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIsVideoSharingReceiver");
        field.set(videoSharingPlugin, new AtomicBoolean(true));
        method.invoke(videoSharingPlugin);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIsVideoSharingSender");
        field.set(videoSharingPlugin, new AtomicBoolean(true));
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 1);
        method.invoke(videoSharingPlugin);

        field = Utils.getPrivateField(SharingPlugin.class, "mRichCallStatus");
        field.set(videoSharingPlugin, 2);
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIsVideoSharingSender");
        field.set(videoSharingPlugin, new AtomicBoolean(false));
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mIsVideoSharingReceiver");
        field.set(videoSharingPlugin, new AtomicBoolean(true));
        method.invoke(videoSharingPlugin);
    }

    public void testCase27_mVideoSharingCallBack() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException {
        if(ALLOW_COMPILE)return ;  Logger.d(TAG, "testCase27_mVideoSharingCallBack() entry!");
        Field field = Utils.getPrivateField(VideoSharingPlugin.class, "mVideoSharingCallBack");
        Object sharingPlugin = field.get(videoSharingPlugin);
        Method method = Utils.getPrivateMethod(sharingPlugin.getClass(), "onApiConnected");
        method.invoke(sharingPlugin);
        method = Utils.getPrivateMethod(sharingPlugin.getClass(), "onFinishSharing");
        method.invoke(sharingPlugin);
        method = Utils.getPrivateMethod(sharingPlugin.getClass(), "displayStoredVideoUI", boolean.class);
        method.invoke(sharingPlugin, true);
        method.invoke(sharingPlugin, false);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mGetIncomingSessionWhenApiConnected");
        field.set(videoSharingPlugin, new AtomicBoolean(true));
        field = Utils.getPrivateField(VideoSharingPlugin.class, "mStartOutgoingSessionWhenApiConnected");
        field.set(videoSharingPlugin, new AtomicBoolean(true));
        method = Utils.getPrivateMethod(sharingPlugin.getClass(), "onApiConnected");
        method.invoke(sharingPlugin);

        field = Utils.getPrivateField(VideoSharingPlugin.class, "mShareStatus");
        field.set(videoSharingPlugin, 4);
        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "showReceivedStatus", int.class);
        method.invoke(videoSharingPlugin, STORED_VIDEO_STATUS_PLAY);
        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "showControllButton", int.class, boolean.class);
        method.invoke(videoSharingPlugin, CONTROLL_START, true);
        method = Utils.getPrivateMethod(VideoSharingPlugin.class, "showPlayingProgress", boolean.class);
        method.invoke(videoSharingPlugin, true);
        method = Utils.getPrivateMethod(sharingPlugin.getClass(), "displayStoredVideoUI", boolean.class);
        method.invoke(sharingPlugin, true);
        method.invoke(sharingPlugin, false);
    }

    private void WaitForNotNull(Object object) throws InterruptedException {
        long startTime = System.currentTimeMillis();
        do {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        } while (object == null);
    }

    private class MockVideoSharePlugin extends VideoSharingPlugin {

        public boolean is3GSupport = true;
        public MockVideoSharePlugin(Context context) {
            super(context);
        }

        @Override
        protected void imageShareNotSupported() {
        }
        
		@Override
        public boolean isImageShareSupported(String number) {
            return false;
        }

        @Override
        protected boolean is3GMobileNetwork() {
            if (is3GSupport) {
                return true;
            } else {
                return false;
            }
        }

      
        @Override
        protected String getVodafoneAccount(String normalNumber) {
            Logger.d(TAG, "getVodafoneAccount()");
            return NUMBER;
        }

        @Override
        public boolean isVideoShareSupported(String number) {
            if (number != null) {
                return true;
            }
            return false;
        }
    }

    private class MockMediaPlayer implements IVideoPlayer {

		@Override
		public IBinder asBinder() {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public void open(String remoteHost, int remotePort)
				throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void close() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void start() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void stop() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public int getLocalRtpPort() throws RemoteException {
			// TODO Auto-generated method stub
			return 0;
		}

		@Override
		public void addListener(IVideoEventListener listener)
				throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void removeAllListeners() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public MediaCodec[] getSupportedVideoCodecs() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public MediaCodec getVideoCodec() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public void setVideoCodec(MediaCodec mediaCodec) throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void setOrientationHeaderId(int headerId) throws RemoteException {
			// TODO Auto-generated method stub
			
		}

       

       

    }

    private class MockViewGroup extends ViewGroup {

        public MockViewGroup(Context context) {
            super(context);
            // TODO Auto-generated constructor stub
        }

        @Override
        protected void onLayout(boolean changed, int l, int t, int r, int b) {
            // TODO Auto-generated method stub

        }
    }

    private class MockCallScreenHost implements ICallScreenHost {
        private static final String TAG = "MockCallScreenHost";
        private Activity mActivity;

        public MockCallScreenHost(Activity activity) {
            mActivity = activity;
        }

        @Override
        public Activity getCallScreenActivity() {
           Logger.d(TAG, "getCallScreenActivity() entry!");
            return mActivity;
        }

        @Override
        public void onCapabilityChange(String number, boolean isSupport) {
            // TODO Auto-generated method stub

        }

        @Override
        public void onStateChange(int state) {
            // TODO Auto-generated method stub

        }

        @Override
        public ViewGroup requestAreaForDisplay() {
            return (ViewGroup) getActivity().getCurrentFocus();
        }
    }

    class MockCallScreenDialog extends CallScreenDialog {
        private static final String TAG = "MockCallScreenDialog";

        public MockCallScreenDialog(Context context) {
            super(context);
        }

        public void setSingleChoiceItems(ListAdapter adapter, int checkedItem,
                android.content.DialogInterface.OnClickListener listener) {
            Logger.v(TAG, "setSingleChoiceItems");
            mAdapter = adapter;
            super.setSingleChoiceItems(adapter, checkedItem, listener);
        }
    }

    class MockRichCallApi extends RichCallApi {

        public MockRichCallApi(Context ctx) {
            super(ctx);
        }

        public IVideoSharingSession getVideoSharingSession(String id) {
            if (id == null) {
                return null;
            } else {
                return new MockVideoSharingSession();
            }
        }

        public IVideoRenderer createVideoRenderer(String format) {
            return new MockMediaRenderer();
        }

        public IVideoSharingSession initiateVideoSharing(String contact, String file, IVideoPlayer player) {
            if (player == null) {
                return null;
            } else {
                return new MockVideoSharingSession();
            }
        }

        public String getVfAccountViaNumber(String number) throws ClientApiException {
            Logger.d(TAG, "getVfAccountViaNumber(), number = " + number);
            return number;
        }

        public IVideoSharingSession initiateLiveVideoSharing(String contact, IVideoPlayer player) {
            return new MockVideoSharingSession();
        }
    }

    class MockVideoSharingSession implements IVideoSharingSession {

        @Override
        public void acceptSession() throws RemoteException {
            // TODO Auto-generated method stub

        }

        @Override
        public void addSessionListener(IVideoSharingEventListener listener) throws RemoteException {
            // TODO Auto-generated method stub

        }

        @Override
        public int getSessionDirection()
        {
        	return 0;
        }

        @Override
        public void cancelSession() throws RemoteException {
            // TODO Auto-generated method stub

        }

        @Override
        public String getRemoteContact() throws RemoteException {
            // TODO Auto-generated method stub
            return null;
        }

        @Override
        public String getSessionID() throws RemoteException {
            // TODO Auto-generated method stub
            return null;
        }

        @Override
        public int getSessionState() throws RemoteException {
            return SessionState.ESTABLISHED;
        }

        @Override
        public void rejectSession() throws RemoteException {
            // TODO Auto-generated method stub

        }

        @Override
        public void removeSessionListener(IVideoSharingEventListener listener) throws RemoteException {
            // TODO Auto-generated method stub

        }

      

        @Override
        public IBinder asBinder() {
            // TODO Auto-generated method stub
            return null;
        }


		@Override
		public void setVideoRenderer(IVideoRenderer renderer)
				throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public IVideoRenderer getVideoRenderer() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public void setVideoPlayer(IVideoPlayer player) throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public IVideoPlayer getVideoPlayer() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}
    }

    private class MockCapabilityApi extends CapabilityApi {

        public MockCapabilityApi(Context context) {
            super(context);
        }

        public Capabilities getMyCapabilities() {
            Logger.v(TAG, "MokeCapabilityApi:getMyCapabilities() entry");
            Capabilities capabilities = new Capabilities();
            capabilities.setVideoSharingSupport(true);
            return capabilities;
        }

        public Capabilities getContactCapabilities(String contact) {
            Logger.v(TAG, "MokeCapabilityApi:getContactCapabilities() entry, the contact is " + contact);
            return getMyCapabilities();
        }
    }

    private class MockDialog extends AlertDialog {

        private static final String TAG = "MockDialog";
        protected MockDialog(Context context) {
            super(context);
        }

        @Override
        public void dismiss() {
            if(ALLOW_COMPILE)return ;  Logger.d(TAG, "dismiss() entry, do nothing!");
        }
    }

    private class MockSurfaceHolder implements SurfaceHolder {

        @Override
        public void addCallback(Callback callback) {
            // TODO Auto-generated method stub

        }

        @Override
        public Surface getSurface() {
            // TODO Auto-generated method stub
            return null;
        }

        @Override
        public Rect getSurfaceFrame() {
            // TODO Auto-generated method stub
            return null;
        }

        @Override
        public boolean isCreating() {
            // TODO Auto-generated method stub
            return false;
        }

        @Override
        public Canvas lockCanvas() {
            // TODO Auto-generated method stub
            return null;
        }

        @Override
        public Canvas lockCanvas(Rect dirty) {
            // TODO Auto-generated method stub
            return null;
        }

        @Override
        public void removeCallback(Callback callback) {
            // TODO Auto-generated method stub

        }

        @Override
        public void setFixedSize(int width, int height) {
            // TODO Auto-generated method stub

        }

        @Override
        public void setFormat(int format) {
            // TODO Auto-generated method stub

        }

        @Override
        public void setKeepScreenOn(boolean screenOn) {
            // TODO Auto-generated method stub

        }

        @Override
        public void setSizeFromLayout() {
            // TODO Auto-generated method stub

        }

        @Override
        public void setType(int type) {
            // TODO Auto-generated method stub

        }

        @Override
        public void unlockCanvasAndPost(Canvas canvas) {
            // TODO Auto-generated method stub

        }
    }

    private class MockMediaRenderer implements IVideoRenderer {

		@Override
		public IBinder asBinder() {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public void open(String remoteHost, int remotePort)
				throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void close() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void start() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void stop() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public int getLocalRtpPort() throws RemoteException {
			// TODO Auto-generated method stub
			return 0;
		}

		@Override
		public void addListener(IVideoEventListener listener)
				throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void removeAllListeners() throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public MediaCodec[] getSupportedVideoCodecs() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public MediaCodec getVideoCodec() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public void setVideoCodec(MediaCodec mediaCodec) throws RemoteException {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void setOrientationHeaderId(int headerId) throws RemoteException {
			// TODO Auto-generated method stub
			
		}
    }
        
}
