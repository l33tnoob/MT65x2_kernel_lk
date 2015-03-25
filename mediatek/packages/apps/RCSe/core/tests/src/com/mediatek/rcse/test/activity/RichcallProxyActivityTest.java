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

package com.mediatek.rcse.test.activity;

import android.app.Activity;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;
import android.test.ActivityInstrumentationTestCase2;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import com.mediatek.rcse.api.CapabilityApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.plugin.phone.ICallScreenHost;
import com.mediatek.rcse.plugin.phone.ImageSharingPlugin;
import com.mediatek.rcse.plugin.phone.RichcallProxyActivity;
import com.mediatek.rcse.plugin.phone.SharingPlugin;
import com.mediatek.rcse.plugin.phone.VideoSharingPlugin;
import com.mediatek.rcse.plugin.phone.VideoSharingPlugin.WaitingProgressDialog;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;
import com.orangelabs.rcs.service.api.client.media.IMediaRenderer;
import com.orangelabs.rcs.service.api.client.media.IVideoRenderer;
import com.orangelabs.rcs.service.api.client.media.video.VideoRenderer;
import com.orangelabs.rcs.service.api.client.richcall.RichCallApi;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class RichcallProxyActivityTest extends
        ActivityInstrumentationTestCase2<RichcallProxyActivity> {

    private static final String TAG = "RichcallProxyActivityTest";
    private static final String IMAGE_SHARING_SELECTION = "com.mediatek.rcse.plugin.phone.IMAGE_SHARING_SELECTION";
    private static final String IMAGE_SHARING_START_ACTION =
            "com.mediatek.phone.plugin.IMAGE_SHARING_START_ACTION";
    private static final String VIDEO_SHARING_START_ACTION =
            "com.mediatek.phone.plugin.VIDEO_SHARING_START_ACTION";
    private static final String CONTACT_CAPABILITIES =
            "com.orangelabs.rcs.capability.CONTACT_CAPABILITIES";
    private static final String NUMBER = "+34200000253";
    private static final int CONNECTED = 2;
    static final int DISCONNECTED = 0;
    public static final int LAUNCH_TIME_OUT = 1000;
    private static final long SLEEP_TIME = 200;
    private static final long TIME_OUT = 10000;
    static final double SELECT_CONTACTS_PATIAL = 0.5;
    static final int MIN_NUMBER = 1;
    private static final int RESULT_CODE = 1;
    private boolean mIsReceived = false;
    private RichcallProxyActivity mActivity = null;
    private static final String SHARE_STATUS = "mShareStatus";
    private Context mContext;
    private static final int REQUEST_CODE_CAMERA = 10;
    private static final int REQUEST_CODE_GALLERY = 11;
    private static final int REQUEST_CODE_VIDEO = 12;

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, final Intent intent) {
            Logger.d(TAG, "BroadcastReceiver onReceive()");
            if (intent != null) {
                String action = intent.getAction();
                Logger.v(TAG, "action = " + action);
                if (IMAGE_SHARING_START_ACTION.equals(action)) {
                    mIsReceived = true;
                } else if (VIDEO_SHARING_START_ACTION.equals(action)) {
                    mIsReceived = true;
                } else {
                    Logger.w(TAG, "Unknown action");
                }
            } else {
                Logger.w(TAG, "intent is null");
            }
            
        }
    };

    public RichcallProxyActivityTest() {
        super(RichcallProxyActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getInstrumentation().getTargetContext();
    }

    /**
     * Use to test the select a image from media
     */
    public void testCase1_SelectPicture() throws Throwable {
        Logger.v(TAG, "testCase1_SelectPicture()");
        mActivity = getActivity();
        assertNotNull(mActivity);
        IntentFilter filter = new IntentFilter();
        filter.addAction(IMAGE_SHARING_START_ACTION);
        filter.addAction(VIDEO_SHARING_START_ACTION);
        Context context = getInstrumentation().getTargetContext();
        context.registerReceiver(mBroadcastReceiver, filter);

        Intent data = new Intent();
        Cursor cursor = null;
        int imageId = 0;
        try {
            cursor =
                    mActivity.getContentResolver().query(
                            MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
            cursor.moveToFirst();
            imageId = cursor.getInt(cursor.getColumnIndex(MediaStore.Images.ImageColumns._ID));
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        Uri uri = Uri.parse("content://media/external/images/media/" + imageId);
        Logger.d(TAG, "testCase1_SelectPicture() uri = " + uri + " imageId" + imageId);
        data.setData(uri);
        Class[] argsClazz = new Class[3];
        argsClazz[0] = int.class;
        argsClazz[1] = int.class;
        argsClazz[2] = data.getClass();
        final Method method = getPrivateMethod(mActivity.getClass(), "onActivityResult", argsClazz);
        assertNotNull(method);

        method.invoke(mActivity, REQUEST_CODE_GALLERY, RESULT_CODE, data);
        Logger.d(TAG, "mIsReceived is " + mIsReceived);
        long startTime = System.currentTimeMillis();
        while (!mIsReceived) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
        }
        Logger.d(TAG, "mIsReceived is " + mIsReceived);
        context.unregisterReceiver(mBroadcastReceiver);
    }
    
    /**
     * Use to test the select a video from media
     */
    public void testCase2_SelectVideo() throws Throwable {
        Logger.v(TAG, "testCase2_SelectVideo()");
        mActivity = getActivity();
        assertNotNull(mActivity);
        IntentFilter filter = new IntentFilter();
        filter.addAction(IMAGE_SHARING_START_ACTION);
        filter.addAction(VIDEO_SHARING_START_ACTION);
        Context context = getInstrumentation().getTargetContext();
        context.registerReceiver(mBroadcastReceiver, filter);

        Intent data = new Intent();
        Cursor cursor = null;
        int videoId = 0;
        try {
            cursor =
                    mActivity.getContentResolver().query(
                            MediaStore.Video.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
            cursor.moveToFirst();
            videoId = cursor.getInt(cursor.getColumnIndex(MediaStore.Video.VideoColumns._ID));
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        Uri uri = Uri.parse("content://media/external/video/media/" + videoId);
        Logger.d(TAG, "testCase1_SelectVideo() uri = " + uri + " videoId" + videoId);
        data.setData(uri);
        Class[] argsClazz = new Class[3];
        argsClazz[0] = int.class;
        argsClazz[1] = int.class;
        argsClazz[2] = data.getClass();
        final Method method = getPrivateMethod(mActivity.getClass(), "onActivityResult", argsClazz);
        assertNotNull(method);

        method.invoke(mActivity, REQUEST_CODE_GALLERY, RESULT_CODE, data);
        Logger.d(TAG, "mIsReceived is " + mIsReceived);
        long startTime = System.currentTimeMillis();
        while (!mIsReceived) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
        }
        Logger.d(TAG, "mIsReceived is " + mIsReceived);
        context.unregisterReceiver(mBroadcastReceiver);
    }
    
    /**
     * Use to test the video sharing dialog
     */
    public void testCase3_testVideoDialog() throws Throwable {
        Logger.v(TAG, "testCase3_testDialog()");
        mActivity = getActivity();
        assertNotNull(mActivity);
        VideoSharingPlugin videoSharingPlugin = new VideoSharingPlugin(mActivity);

        Field field = getPrivateField(VideoSharingPlugin.class, "mVideoSharingDialogManager");
        Object mVideoSharingDialogManager = field.get(videoSharingPlugin);
        videoSharingPlugin.setCallScreenHost(mCallScreenHost);

        Method showTerminatedByRemoteDialog =
                getPrivateMethod(mVideoSharingDialogManager.getClass(),
                        "showTerminatedByRemoteDialog");
        assertNotNull(showTerminatedByRemoteDialog);
        showTerminatedByRemoteDialog.invoke(mVideoSharingDialogManager);

        Method showTerminatedByNetworkDialog =
                getPrivateMethod(mVideoSharingDialogManager.getClass(),
                        "showTerminatedByNetworkDialog");
        assertNotNull(showTerminatedByNetworkDialog);
        showTerminatedByNetworkDialog.invoke(mVideoSharingDialogManager);

        Method showRejectedByRemoteDialog =
                getPrivateMethod(mVideoSharingDialogManager.getClass(),
                        "showRejectedByRemoteDialog");
        assertNotNull(showRejectedByRemoteDialog);
        showRejectedByRemoteDialog.invoke(mVideoSharingDialogManager);

        Method showTimeOutDialog =
                getPrivateMethod(mVideoSharingDialogManager.getClass(), "showTimeOutDialog");
        assertNotNull(showTimeOutDialog);
        showTimeOutDialog.invoke(mVideoSharingDialogManager);

        WaitingProgressDialog.newInstance(mActivity, mActivity);

        Method method = getPrivateMethod(videoSharingPlugin.getClass(), "destroy");
        method.invoke(videoSharingPlugin);
    }

    /**
     * Use to test the image sharing dialog
     */
    public void testCase4_testImageDialog() throws Throwable {
        Logger.v(TAG, "testCase4_testImageDialog()");
        mActivity = getActivity();
        assertNotNull(mActivity);
        ImageSharingPlugin imageSharingPlugin = new ImageSharingPlugin(mActivity);

        Field field = getPrivateField(ImageSharingPlugin.class, "mCallScreenDialogManager");
        Object mCallScreenDialogManager = field.get(imageSharingPlugin);
        imageSharingPlugin.setCallScreenHost(mCallScreenHost);

        Method showInvitationConfirmDialog =
                getPrivateMethod(mCallScreenDialogManager.getClass(), "showInvitationConfirmDialog");
        assertNotNull(showInvitationConfirmDialog);
        showInvitationConfirmDialog.invoke(mCallScreenDialogManager);

        Method showTimeOutDialog =
                getPrivateMethod(mCallScreenDialogManager.getClass(), "showTimeOutDialog");
        assertNotNull(showTimeOutDialog);
        showTimeOutDialog.invoke(mCallScreenDialogManager);

        Method showInitFailDialog =
                getPrivateMethod(mCallScreenDialogManager.getClass(), "showInitFailDialog");
        assertNotNull(showInitFailDialog);
        showInitFailDialog.invoke(mCallScreenDialogManager);

        Method showRejectedDialog =
                getPrivateMethod(mCallScreenDialogManager.getClass(), "showRejectedDialog");
        assertNotNull(showRejectedDialog);
        showRejectedDialog.invoke(mCallScreenDialogManager);

        Method showTerminatedDialog =
                getPrivateMethod(mCallScreenDialogManager.getClass(), "showTerminatedDialog");
        assertNotNull(showTerminatedDialog);
        showTerminatedDialog.invoke(mCallScreenDialogManager);

        Method showTerminateConfirmDialog =
                getPrivateMethod(mCallScreenDialogManager.getClass(), "showTerminateConfirmDialog");
        assertNotNull(showTerminateConfirmDialog);
        showTerminateConfirmDialog.invoke(mCallScreenDialogManager);

        Method showNoStorageDialog =
                getPrivateMethod(mCallScreenDialogManager.getClass(), "showNoStorageDialog");
        assertNotNull(showNoStorageDialog);
        showNoStorageDialog.invoke(mCallScreenDialogManager);

        Method showSelectImageDialog =
                getPrivateMethod(mCallScreenDialogManager.getClass(), "showSelectImageDialog");
        assertNotNull(showSelectImageDialog);
        showSelectImageDialog.invoke(mCallScreenDialogManager);

        Method method = getPrivateMethod(imageSharingPlugin.getClass(), "finishImageSharing");
        method.invoke(imageSharingPlugin);
    }

    /**
     * Use to test the VideoSharingPlugin function getCapability()
     */
    public void testCase5_getVideoCapability() throws Throwable {
        Logger.v(TAG, "testCase5_getVideoCapability()");
        mActivity = getActivity();
        assertNotNull(mActivity);
        PluginApiManager.initialize(mActivity);
        Field field = getPrivateField(PluginApiManager.class, "mIsRegistered");
        field.set(PluginApiManager.getInstance(), true);

        VideoSharingPlugin videoSharingPlugin = new VideoSharingPlugin(mActivity);
        field = getPrivateField(SharingPlugin.class, "mRichCallStatus");
        field.set(videoSharingPlugin, CONNECTED);

        RichCallApi richCallApi = new MokeRichCallApi(mActivity);
        field = getPrivateField(SharingPlugin.class, "mRichCallApi");
        field.set(videoSharingPlugin, richCallApi);

        RegistrationApi registrationApi = new RegistrationApi(mActivity);
        field = getPrivateField(PluginApiManager.class, "mRegistrationApi");
        field.set(PluginApiManager.getInstance(), registrationApi);

        Capabilities capabilities = new Capabilities();
        capabilities.setRcseContact(true);
        Intent intent = new Intent(CONTACT_CAPABILITIES);
        intent.putExtra("contact", NUMBER);
        intent.putExtra("capabilities", capabilities);
        field = getPrivateField(PluginApiManager.class, "mBroadcastReceiver");
        BroadcastReceiver broadcastReceiver =
                (BroadcastReceiver) field.get(PluginApiManager.getInstance());
        broadcastReceiver.onReceive(mActivity, intent);
        
        long startTime = System.currentTimeMillis();
        while (!videoSharingPlugin.getCapability(NUMBER)) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }
        Method method = getPrivateMethod(videoSharingPlugin.getClass(), "destroy");
        method.invoke(videoSharingPlugin);
    }
    
    /**
     * Use to test the ImageSharingPlugin function getCapability()
     */
    public void testCase6_getImageCapability() throws Throwable {
        Logger.v(TAG, "testCase6_getImageCapability()");
        mActivity = getActivity();
        assertNotNull(mActivity);
        PluginApiManager.initialize(mActivity);
        Field field = getPrivateField(PluginApiManager.class, "mIsRegistered");
        field.set(PluginApiManager.getInstance(), true);

        ImageSharingPlugin imageSharingPlugin = new ImageSharingPlugin(mActivity);
        field = getPrivateField(SharingPlugin.class, "mRichCallStatus");
        field.set(imageSharingPlugin, CONNECTED);

        RichCallApi richCallApi = new MokeRichCallApi(mActivity);
        field = getPrivateField(SharingPlugin.class, "mRichCallApi");
        field.set(imageSharingPlugin, richCallApi);

        RegistrationApi registrationApi = new RegistrationApi(mActivity);
        field = getPrivateField(PluginApiManager.class, "mRegistrationApi");
        field.set(PluginApiManager.getInstance(), registrationApi);

        imageSharingPlugin.setCallScreenHost(mCallScreenHost);

        long startTime = System.currentTimeMillis();
        while (!imageSharingPlugin.getCapability(NUMBER)) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }

        Method method = getPrivateMethod(imageSharingPlugin.getClass(), "finishImageSharing");
        method.invoke(imageSharingPlugin);
        
    }
    
    /**
     * Use to test the startVideoShare function
     */
    public void testCase7_startVideoShare() throws Throwable {
        Logger.v(TAG, "testCase7_startVideoShare()");
        mActivity = getActivity();
        assertNotNull(mActivity);
        PluginApiManager.initialize(mActivity);
        Field field = getPrivateField(PluginApiManager.class, "mIsRegistered");
        field.set(PluginApiManager.getInstance(), true);

        VideoSharingPlugin videoSharingPlugin = new VideoSharingPlugin(mActivity);
        videoSharingPlugin.setCallScreenHost(mCallScreenHost);

        field = getPrivateField(SharingPlugin.class, "mNumber");
        field.set(videoSharingPlugin, NUMBER);

        RichCallApi richCallApi = new MokeRichCallApi(mActivity);
        field = getPrivateField(SharingPlugin.class, "mRichCallApi");
        field.set(videoSharingPlugin, richCallApi);

        MokeCapabilityApi mokeCapabilityApi = new MokeCapabilityApi(mActivity);
        field = getPrivateField(PluginApiManager.class, "mCapabilitiesApi");
        field.set(PluginApiManager.getInstance(), mokeCapabilityApi);
        videoSharingPlugin.registerForCapabilityChange(NUMBER);

        field = getPrivateField(SharingPlugin.class, "mRichCallApiListener");
        assertNotNull(field);

        Object mRichCallApiListener = field.get(videoSharingPlugin);
        long startTime = System.currentTimeMillis();
        while (mRichCallApiListener == null) {
            mRichCallApiListener = field.get(videoSharingPlugin);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }

        Method handleApiConnected =
                getPrivateMethod(mRichCallApiListener.getClass(), "handleApiConnected");
        assertNotNull(handleApiConnected);
        handleApiConnected.invoke(mRichCallApiListener);

        field = getPrivateField(VideoSharingPlugin.class, "mOutgoingVideoFormat");
        field.set(videoSharingPlugin, "h264");

        field = getPrivateField(VideoSharingPlugin.class, SHARE_STATUS);
        field.setAccessible(true);
        field.set(videoSharingPlugin, 1);

        Method method = getPrivateMethod(VideoSharingPlugin.class, "startVideoShare");
        method.invoke(videoSharingPlugin);
        
        method = getPrivateMethod(videoSharingPlugin.getClass(), "destroy");
        method.invoke(videoSharingPlugin);
    }
    
    /**
     * Test cancel a file share
     * 
     * @throws Throwable
     */
    public void testCase8_ResultCancel() throws Throwable {
        Logger.d(TAG, "testCase8_ResultCancel() entry");
        mActivity = getActivity();
        assertNotNull(mActivity);
        Field filedCamera =
                Utils.getPrivateField(RichcallProxyActivity.class, "REQUEST_CODE_CAMERA");
        Method methodOnActivityResult =
                Utils.getPrivateMethod(RichcallProxyActivity.class, "onActivityResult", int.class,
                        int.class, Intent.class);
        methodOnActivityResult.invoke(mActivity, filedCamera.getInt(mActivity),
                Activity.RESULT_CANCELED, null);
        waitForActivityFinish();
        Logger.d(TAG, "testCase8_ResultCancel() exit");
    }

    /**
     * Test to share a file from Camera
     * 
     * @throws NoSuchMethodException
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InterruptedException
     */
    public void testCase9_RequestCodeCamera() throws NoSuchMethodException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException {
        Logger.d(TAG, "testCase9_RequestCodeCamera() entry");
        mActivity = getActivity();
        assertNotNull(mActivity);
        Method methodOnActivityResult =
                Utils.getPrivateMethod(RichcallProxyActivity.class, "onActivityResult", int.class,
                        int.class, Intent.class);
        Field filedCameraTempFileUri =
                Utils.getPrivateField(RichcallProxyActivity.class, "mCameraTempFileUri");
        filedCameraTempFileUri.set(mActivity, getFileUri());
        Field filedStartCamera =
                Utils.getPrivateField(RichcallProxyActivity.class, "REQUEST_CODE_CAMERA");
        methodOnActivityResult.invoke(mActivity, filedStartCamera.getInt(mActivity),
                Activity.RESULT_OK, null);
        waitForActivityFinish();
        Logger.d(TAG, "testCase9_RequestCodeCamera() exit");
    }

    /**
     * Test start a share from gallery
     * 
     * @throws NoSuchMethodException
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     */
    public void testCase10_startGallery() throws NoSuchMethodException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase10_startGallery() entry");
        mActivity = getShareActivity(RichcallProxyActivity.SELECT_TYPE_GALLERY);
        assertNotNull(mActivity);
        mActivity.finishActivity(REQUEST_CODE_GALLERY);
        Logger.d(TAG, "testCase10_startGallery() exit");
    }

    /**
     * Test start a share from Camera
     * 
     * @throws NoSuchMethodException
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     */
    public void testCase11_startCamera() throws NoSuchMethodException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase11_startCamera() entry");
        mActivity = getShareActivity(RichcallProxyActivity.SELECT_TYPE_CAMERA);
        assertNotNull(mActivity);
        mActivity.finishActivity(REQUEST_CODE_CAMERA);
        Logger.d(TAG, "testCase11_startCamera() exit");
    }

    /**
     * Test start a share from VideoGallery
     * 
     * @throws NoSuchMethodException
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     */
    public void testCase12_startVideoGallery() throws NoSuchMethodException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase12_startVideoGallery() entry");
        mActivity = getShareActivity(RichcallProxyActivity.SELECT_TYPE_VIDEO);
        assertNotNull(mActivity);
        mActivity.finishActivity(REQUEST_CODE_VIDEO);
        Logger.d(TAG, "testCase12_startVideoGallery() exit");
    }
    
    /**
     * Test to share a video file
     * 
     * @throws NoSuchMethodException
     * @throws NoSuchFieldException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InterruptedException
     */
    public void testCase13_RequestCodeVideo() throws NoSuchMethodException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException {
        Logger.d(TAG, "testCase13_RequestCodeVideo() entry");
        mActivity = getActivity();
        assertNotNull(mActivity);
        Method methodOnActivityResult =
                Utils.getPrivateMethod(RichcallProxyActivity.class, "onActivityResult", int.class,
                        int.class, Intent.class);
        Field filedVideo = Utils.getPrivateField(RichcallProxyActivity.class, "REQUEST_CODE_VIDEO");
        Intent intent = new Intent();
        intent.setData(getVideoUri());
        methodOnActivityResult.invoke(mActivity, filedVideo.getInt(mActivity), Activity.RESULT_OK,
                intent);
        waitForActivityFinish();
        Logger.d(TAG, "testCase13_RequestCodeVideo() exit");
    }

    private RichcallProxyActivity getShareActivity(String type) {
        Logger.d(TAG, "getShareActivity() entry");
        Intent intent = new Intent(IMAGE_SHARING_SELECTION);
        intent.putExtra(RichcallProxyActivity.SELECT_TYPE, type);
        setActivityIntent(intent);
        Logger.d(TAG, "getShareActivity() exit");
        return getActivity();
    }
    
    private void waitForActivityFinish() throws InterruptedException {
        Logger.d(TAG, "waitForActivityFinish() entry");
        mActivity = getActivity();
        assertNotNull(mActivity);
        long startTime = System.currentTimeMillis();
        while (!mActivity.isFinishing()) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("waitForActivityFinish() timeout");
            }
            Thread.sleep(SLEEP_TIME);
        }
        Logger.d(TAG, "waitForActivityFinish() exit");
    }

    private Uri getFileUri() {
        Cursor cursor = null;
        int imageId = 0;
        try {
            cursor =
                    mContext.getContentResolver().query(
                            MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
            cursor.moveToFirst();
            imageId = cursor.getInt(cursor.getColumnIndex(MediaStore.Images.ImageColumns._ID));
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        Uri uri = Uri.parse("content://media/external/images/media/" + imageId);
        Logger.d(TAG, "getFileUri() exit with uri is " + uri);
        return uri;
    }
    
    private Uri getVideoUri() {
        Cursor cursor = null;
        int videoId = 0;
        try {
            cursor =
                    mActivity.getContentResolver().query(
                            MediaStore.Video.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
            cursor.moveToFirst();
            videoId = cursor.getInt(cursor.getColumnIndex(MediaStore.Video.VideoColumns._ID));
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        Uri uri = Uri.parse("content://media/external/video/media/" + videoId);
        Logger.d(TAG, "getVideoUri() uri = " + uri + " videoId" + videoId);
        return uri;
    }

    private Field getPrivateField(Class clazz, String filedName)
            throws NoSuchFieldException {

        Field field = clazz.getDeclaredField(filedName);
        assertTrue(field != null);
        field.setAccessible(true);
        return field;

    }

    private Method getPrivateMethod(Class clazz, String methodName,
            Class<?>... parameterTypes) throws NoSuchMethodException {

        Method method = clazz.getDeclaredMethod(methodName, parameterTypes);
        assertTrue(method != null);
        method.setAccessible(true);
        return method;

    }
    
    
    private class MokeRichCallApi extends RichCallApi {

        public MokeRichCallApi(Context ctx) {
            super(ctx);
        }
        
        public String getVfAccountViaNumber(String number) throws ClientApiException {
            return number;
        }

        public IVideoRenderer createVideoRenderer(String format) throws ClientApiException {
            return new VideoRenderer();
        }
            
    }
    
    private class MokeCapabilityApi extends CapabilityApi {

        public MokeCapabilityApi(Context context) {
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
    
    private ICallScreenHost mCallScreenHost = new ICallScreenHost() {
        
        @Override
        public ViewGroup requestAreaForDisplay() {
            return new RelativeLayout(mContext);
        }
        
        @Override
        public void onStateChange(int arg0) {
            // TODO Auto-generated method stub
            
        }
        
        @Override
        public void onCapabilityChange(String arg0, boolean arg1) {
            // TODO Auto-generated method stub
            
        }

        @Override
        public Activity getCallScreenActivity() {
            return mActivity;
        }
    };

}
