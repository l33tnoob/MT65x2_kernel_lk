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

import android.app.Activity;
import android.app.AlertDialog;

import android.database.Cursor;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.provider.MediaStore;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;

import com.android.internal.app.AlertController;
import com.mediatek.rcse.plugin.phone.CallScreenDialog;
import com.mediatek.rcse.plugin.phone.Constants;
import com.mediatek.rcse.plugin.phone.ICallScreenHost;
import com.mediatek.rcse.plugin.phone.ImageSharingPlugin;
import com.mediatek.rcse.plugin.phone.RichcallProxyActivity;
import com.mediatek.rcse.plugin.phone.SharingPlugin;
import com.mediatek.rcse.plugin.phone.ImageSharingPlugin.CompressDialog;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.core.ims.service.richcall.ContentSharingError;
import com.orangelabs.rcs.platform.file.FileDescription;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.ClientApiListener;
import com.orangelabs.rcs.service.api.client.richcall.IImageSharingEventListener;
import com.orangelabs.rcs.service.api.client.richcall.IImageSharingSession;
import com.orangelabs.rcs.service.api.client.richcall.RichCallApi;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

/**
 * Test case for ImageSharingPluginTest.
 */
public class ImageSharingPluginTest extends ActivityInstrumentationTestCase2<RichcallProxyActivity> {

    private static final String TAG = "ImageSharingPluginTest";
    static final String ACTION = "com.mediatek.rcse.plugin.phone.IMAGE_SHARING_SELECTION";
    private static final int TIME_OUT = 3000;
    private static final int SLEEP_TIME = 500;
    private static final int WAIT_TIME = 5000;
    private static final String METHOD_START = "start";
    private static final String PHONE_NUMBER = "13911111111";
    private final static String MOCK_NMUBER = "1234567890";
    private final static String MOCK_VF_ACCOUNT = "+34200000251";
    private ImageSharingPlugin mImageSharingPlugin;
    private ViewGroup mImageSharingView = null;
    private static final String IMAGE_SHARING_INVITATION_ACTION = "com.orangelabs.rcs.richcall.IMAGE_SHARING_INVITATION";
    private static final String IMAGE_SHARING_START_ACTION = "com.mediatek.phone.plugin.IMAGE_SHARING_START_ACTION";
    private static final String IMAGE_SHARING_UNKNOWN_ACTION = "unknow action";
    private static final String IMAGE_NAME = "filename";
    private static final String IMAGE_SIZE = "filesize";
    private static final String MOCK_IMAGE_NAME = "test.jpg";
    private static final String CONTACT = "contact";
    private static final String CONTACT_DISPLAYNAME = "contactDisplayname";
    private static final String SESSION_ID = "sessionId";
    private Activity mActivity;
    private static final int DISCONNECTED = 0;
    private static final int CONNECTING = 1;
    private static final int CONNECTED = 2;
    private IImageSharingSession mMockIncomingImageSharingSession = new MockIncomingImageSharingSession();
    private boolean mIsOnStateChange = false;
    private boolean mIsRejectSession = false;
    private boolean mInitiateImageSharing = false;
    private boolean mIsAddSessionListener = false;
    private static final long MOCK_ZERO_SIZE = 0L;
    private static final long MOCK_CURRENT_SIZE = 10L;
    private static final long MOCK_TOTAL_SIZE = 100L;
    private static final String MOCK_ORI_NAME = "mock ori name";
    private static final int WAIT_DIALOG_TIME = 2000;
    private static final int MOCK_MAX_IS_SIZE = 10000;
    private static final int MOCK_MAX_IS_SIZE_ZERO = 0;
    
    public ImageSharingPluginTest() {
        super(RichcallProxyActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mActivity = getActivity();
    }
    
    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    /**
     * Test the method of getAppTitle()
     * 
     * @throws Throwable
     */
    public void testCase1_ThumbnailsTest() throws Throwable {
        Cursor cursor = null;
        String imageName = null;
        try {
            cursor = this.mActivity.getContentResolver().query(
                    MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
            cursor.moveToFirst();
            imageName = cursor
                    .getString(cursor.getColumnIndex(MediaStore.Images.ImageColumns.DATA));
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        Logger.d(TAG, "testCase1_ThumbnailsTest(), imageName = " + imageName);
        final String name = imageName;
        mImageSharingPlugin = new ImageSharingPlugin(mActivity);
        mImageSharingPlugin.setCallScreenHost(mCallScreenHost);

        this.runTestOnUiThread(new Runnable() {

            public void run() {
                try {
                    Method method = Utils.getPrivateMethod(ImageSharingPlugin.class,
                            "startImageSharing", String.class);
                    method.invoke(mImageSharingPlugin, name);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        Thread.sleep(SLEEP_TIME);
        waitForImageNotNull();
        waitForHaveImage();
    }

    public void testCase2_ImageShareNotSupported() {
        MockImageSharePlugin imageSharingPlugin = new MockImageSharePlugin(mActivity);
        try {
            Method method = ImageSharingPlugin.class.getDeclaredMethod(METHOD_START, String.class);
            method.invoke(imageSharingPlugin, PHONE_NUMBER);
        } catch (Exception e) {
            e.printStackTrace();
        }
        assertFalse(imageSharingPlugin.imageShareSupported);
    }

    public void testCase3_Vibrate() {
        Logger.d(TAG, "testCase3_Vibrate");
        com.mediatek.rcse.plugin.phone.Utils.resetVibratedStatus();
        assertEquals(false, com.mediatek.rcse.plugin.phone.Utils.isVibrated());
        com.mediatek.rcse.plugin.phone.Utils.vibrate(getInstrumentation().getTargetContext(), 2000);
        assertEquals(true, com.mediatek.rcse.plugin.phone.Utils.isVibrated());
        com.mediatek.rcse.plugin.phone.Utils.resetVibratedStatus();
    }

    public void testCase4_Invitation() throws InterruptedException, IllegalArgumentException,
            NoSuchFieldException, IllegalAccessException {
        Logger.d(TAG, "testCase4_ISInvitation");
        com.mediatek.rcse.plugin.phone.Utils.resetVibratedStatus();
        MockImageSharePlugin imageSharingPlugin = new MockImageSharePlugin(mActivity);
        imageSharingPlugin.setCallScreenHost(mCallScreenHost);
        // Wait Plugin created
        wait_pluginCreated(imageSharingPlugin);
        Intent intent = new Intent();
        intent.setAction(IMAGE_SHARING_INVITATION_ACTION);
        intent.putExtra(CONTACT, PHONE_NUMBER);
        intent.putExtra(CONTACT_DISPLAYNAME, "test");
        intent.putExtra(SESSION_ID, "123456");
        intent.putExtra(IMAGE_NAME, "test.jpg");
        intent.putExtra(IMAGE_SIZE, 102L);
        Logger.d(TAG, "testCase4_ISInvitation send broadcast");
        getInstrumentation().getTargetContext().sendBroadcast(intent);
        // Should wait the broadcast to be received
        wait_invitation();
        assertEquals(true, com.mediatek.rcse.plugin.phone.Utils.isVibrated());
    }
    
    /**
     * This is to test acceptImageSharingSession() function
     */
    public void testCase5_AcceptImageSharingSession() throws Throwable {
    	Logger.d(TAG, "testCase5_AcceptImageSharingSession() entry");
    	ImageSharingPlugin imageSharingPlugin = new ImageSharingPlugin(mActivity);
    	imageSharingPlugin.setCallScreenHost(mCallScreenHost);
    	imageSharingPlugin.registerForCapabilityChange(PHONE_NUMBER);
    	wait_pluginCreated(imageSharingPlugin);
 	
        Field mRichCallStatusField = Utils.getPrivateField(ImageSharingPlugin.class.getSuperclass(), "mRichCallStatus");
        mRichCallStatusField.set(imageSharingPlugin, CONNECTED);
        
        Field mIncomingImageSharingSession = Utils.getPrivateField(ImageSharingPlugin.class, "mIncomingImageSharingSession");
        mIncomingImageSharingSession.set(imageSharingPlugin, mMockIncomingImageSharingSession);
        
        // Test filesize normal
        com.mediatek.rcse.plugin.phone.Utils.setInImageSharing(false);
        Method acceptImageSharingSession = Utils.getPrivateMethod(ImageSharingPlugin.class, "acceptImageSharingSession");
        assertNotNull(acceptImageSharingSession);
        acceptImageSharingSession.invoke(imageSharingPlugin);
        assertTrue(com.mediatek.rcse.plugin.phone.Utils.isInImageSharing());
        
        // Test filesize too large
        mIncomingImageSharingSession.set(imageSharingPlugin, new MockIncomingImageSharingSession2());
        acceptImageSharingSession.invoke(imageSharingPlugin);
        assertFalse(com.mediatek.rcse.plugin.phone.Utils.isInImageSharing());
    }
    
    /**
     * This is to test acceptImageSharing() function
     */
    public void testCase6_AcceptImageSharing() throws Throwable {
    	Logger.d(TAG, "testCase6_AcceptImageSharing() entry");
    	final ImageSharingPlugin imageSharingPlugin = new ImageSharingPlugin(mActivity);
    	imageSharingPlugin.setCallScreenHost(mCallScreenHost);
    	imageSharingPlugin.registerForCapabilityChange(PHONE_NUMBER);
    	wait_pluginCreated(imageSharingPlugin);
    	
    	mIsOnStateChange = false;
    	runTestOnUiThread(new Runnable() {
			@Override
			public void run() {
				Method acceptImageSharing = null;
				try {
					acceptImageSharing = Utils.getPrivateMethod(ImageSharingPlugin.class, "acceptImageSharing");
					assertNotNull(acceptImageSharing);
			        acceptImageSharing.invoke(imageSharingPlugin);
				} catch (NoSuchMethodException e) {
					e.printStackTrace();
				} catch (IllegalArgumentException e) {
					e.printStackTrace();
				} catch (IllegalAccessException e) {
					e.printStackTrace();
				} catch (InvocationTargetException e) {
					e.printStackTrace();
				}
			}
		});
        
        long startTime = System.currentTimeMillis();
        while (mIsOnStateChange != true) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("testCase6_AcceptImageSharing() timeout"
                        + (System.currentTimeMillis() - startTime));
            }
        }
        assertTrue(mIsOnStateChange);
        mIsOnStateChange = false;
    }
    
    /**
     * This is to test updateProgressBar() function
     */
    public void testCase7_UpdateProgressBar() throws Throwable {
    	Logger.d(TAG, "testCase7_UpdateProgressBar() entry");
    	final ImageSharingPlugin imageSharingPlugin = new ImageSharingPlugin(mActivity);
    	imageSharingPlugin.setCallScreenHost(mCallScreenHost);
    	imageSharingPlugin.registerForCapabilityChange(PHONE_NUMBER);
    	wait_pluginCreated(imageSharingPlugin);
    	
        runTestOnUiThread(new Runnable() {
			@Override
			public void run() {
				Method updateProgressBar = null;
				try {
					Field mImageSharingView = Utils.getPrivateField(ImageSharingPlugin.class, "mImageSharingView");
					mImageSharingView.set(imageSharingPlugin, null);
					assertNull(mImageSharingView.get(imageSharingPlugin));
					updateProgressBar = Utils.getPrivateMethod(ImageSharingPlugin.class, "updateProgressBar", long.class, long.class);
					updateProgressBar.invoke(imageSharingPlugin, MOCK_CURRENT_SIZE, MOCK_TOTAL_SIZE);
					assertNotNull(mImageSharingView.get(imageSharingPlugin));
					mImageSharingView.set(imageSharingPlugin, null);
					updateProgressBar.invoke(imageSharingPlugin, MOCK_ZERO_SIZE, MOCK_TOTAL_SIZE);
					assertNotNull(mImageSharingView.get(imageSharingPlugin));
				} catch (IllegalArgumentException e) {
					e.printStackTrace();
					fail();
				} catch (IllegalAccessException e) {
					e.printStackTrace();
					fail();
				} catch (InvocationTargetException e) {
					e.printStackTrace();
					fail();
				} catch (NoSuchFieldException e) {
					e.printStackTrace();
					fail();
				} catch (NoSuchMethodException e) {
					e.printStackTrace();
					fail();
				}
			}
		});  
    }
    
    /**
     * This is to test prepareStartImageSharing() function
     */
    public void testCase8_PrepareStartImageSharing() throws Throwable {
    	Logger.d(TAG, "testCase8_PrepareStartImageSharing() entry");
    	String filePath = getFilePath();
    	ImageSharingPlugin imageSharingPlugin = new ImageSharingPlugin(mActivity);
    	imageSharingPlugin.setCallScreenHost(mCallScreenHost);
    	imageSharingPlugin.registerForCapabilityChange(PHONE_NUMBER);
    	wait_pluginCreated(imageSharingPlugin);
        Field mRichCallStatusField = Utils.getPrivateField(ImageSharingPlugin.class.getSuperclass(), "mRichCallStatus");
        mRichCallStatusField.set(imageSharingPlugin, CONNECTED);
    	
    	RcsSettings.createInstance(getInstrumentation().getTargetContext());
        RcsSettings rcsSettings = RcsSettings.getInstance();
        String isEnabledCompressingImageFromDB = rcsSettings.readParameter(RcsSettingsData.RCSE_COMPRESSING_IMAGE);
        String restoreRemindCompressFlag = rcsSettings.readParameter(RcsSettingsData.COMPRESS_IMAGE_HINT);
    	
        // isEnabledCompressingImageFromDB == true
        rcsSettings.writeParameter(RcsSettingsData.RCSE_COMPRESSING_IMAGE, "true");
    	Field mOutgoingSessionEventListener = Utils.getPrivateField(ImageSharingPlugin.class, "mOutgoingSessionEventListener");
    	mOutgoingSessionEventListener.set(imageSharingPlugin, null);
    	Method prepareStartImageSharing = Utils.getPrivateMethod(ImageSharingPlugin.class, "prepareStartImageSharing", String.class);
        assertNotNull(prepareStartImageSharing);
        prepareStartImageSharing.invoke(imageSharingPlugin, filePath);
        long startTime = System.currentTimeMillis();
        while (mOutgoingSessionEventListener.get(imageSharingPlugin) == null) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("testCase8_PrepareStartImageSharing() timeout");
            }
        }
        assertNotNull(mOutgoingSessionEventListener.get(imageSharingPlugin));
        
        // isEnabledCompressingImageFromDB == false
        rcsSettings.writeParameter(RcsSettingsData.RCSE_COMPRESSING_IMAGE, "false");

        // restoreRemindCompressFlag == false
        mOutgoingSessionEventListener.set(imageSharingPlugin, null);
        rcsSettings.writeParameter(RcsSettingsData.COMPRESS_IMAGE_HINT, "false");
        prepareStartImageSharing.invoke(imageSharingPlugin, filePath);
        startTime = System.currentTimeMillis();
        while (mOutgoingSessionEventListener.get(imageSharingPlugin) == null) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("testCase8_PrepareStartImageSharing() timeout");
            }
        }
        assertNotNull(mOutgoingSessionEventListener.get(imageSharingPlugin));
        
        rcsSettings.writeParameter(RcsSettingsData.COMPRESS_IMAGE_HINT, "true");
        prepareStartImageSharing.invoke(imageSharingPlugin, filePath);
        Activity activity = mCallScreenHost.getCallScreenActivity();
        startTime = System.currentTimeMillis();
        while (null == activity.getFragmentManager().findFragmentByTag("CompressDialog")) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("testCase8_PrepareStartImageSharing() timeout");
            }
        }
        
        // clear up
        rcsSettings.writeParameter(RcsSettingsData.RCSE_COMPRESSING_IMAGE, isEnabledCompressingImageFromDB);
        rcsSettings.writeParameter(RcsSettingsData.COMPRESS_IMAGE_HINT, restoreRemindCompressFlag);
    }
    
    /**
     * This is to test CallScreenDialogManager
     */
    public void testCase9_CallScreenDialogManager() throws Throwable {
    	Logger.d(TAG, "testCase9_CallScreenDialogManager() entry");
    	final ImageSharingPlugin imageSharingPlugin = new ImageSharingPlugin(mActivity);
    	imageSharingPlugin.setCallScreenHost(mCallScreenHost);
    	imageSharingPlugin.registerForCapabilityChange(PHONE_NUMBER);
    	wait_pluginCreated(imageSharingPlugin);
    	
    	Field mIncomingImageName = Utils.getPrivateField(ImageSharingPlugin.class, "mIncomingImageName");
    	mIncomingImageName.set(imageSharingPlugin, MOCK_IMAGE_NAME);
    	
    	Field mCallScreenDialogManager = Utils.getPrivateField(ImageSharingPlugin.class, "mCallScreenDialogManager");
    	Object dialogManager = mCallScreenDialogManager.get(imageSharingPlugin);
    	
    	// showInvitationConfirmDialog case 1
    	Utils.getPrivateField(ImageSharingPlugin.class, "mMaxImageSharingSize").set(imageSharingPlugin, MOCK_MAX_IS_SIZE);
    	waitDialogShow(dialogManager, "showInvitationConfirmDialog");
    	clickDialogButtonBoth(dialogManager);
    	
    	// showInvitationConfirmDialog case 2
    	Utils.getPrivateField(ImageSharingPlugin.class, "mIncomingImageSize").set(imageSharingPlugin, Long.MAX_VALUE);
    	Utils.getPrivateField(ImageSharingPlugin.class, "mMaxImageSharingSize").set(imageSharingPlugin, MOCK_MAX_IS_SIZE);
    	waitDialogShow(dialogManager, "showInvitationConfirmDialog");
    	clickDialogButtonPositive(dialogManager);
    	
    	waitDialogShow(dialogManager, "showTimeOutDialog");
    	clickDialogButtonPositive(dialogManager);
    	
    	waitDialogShow(dialogManager, "showInitFailDialog");
    	clickDialogButtonPositive(dialogManager);
    	
    	waitDialogShow(dialogManager, "showRejectedDialog");
    	clickDialogButtonPositive(dialogManager);
    	
    	waitDialogShow(dialogManager, "showTerminatedDialog");
    	clickDialogButtonPositive(dialogManager);
    	
    	waitDialogShow(dialogManager, "showTerminateConfirmDialog");
    	clickDialogButtonBoth(dialogManager);
    	
    	waitDialogShow(dialogManager, "showNoStorageDialog");
    	clickDialogButtonPositive(dialogManager);
    	
    	waitDialogShow(dialogManager, "showSelectImageDialog");
    	
    	imageSharingPlugin.dismissDialog();
    }
    
    private void clickDialogButtonBoth(Object dialogManager) throws NoSuchFieldException, IllegalArgumentException, IllegalAccessException, InterruptedException {
    	Field mDialogs = Utils.getPrivateField(dialogManager.getClass(), "mDialogs");
    	List<CallScreenDialog> dialogs = (List<CallScreenDialog>) mDialogs.get(dialogManager);
    	assertEquals(1, dialogs.size());
    	CallScreenDialog invitationConfirmDialog = dialogs.get(0);
    	Field mAlertDialog = Utils.getPrivateField(CallScreenDialog.class, "mAlertDialog");
    	final AlertDialog alertDialog = (AlertDialog) mAlertDialog.get(invitationConfirmDialog);
    	assertNotNull(alertDialog);
    	Field mBuilderField = Utils.getPrivateField(CallScreenDialog.class, "mBuilder");
    	final AlertDialog.Builder mBuilder = (AlertDialog.Builder) mBuilderField.get(invitationConfirmDialog);
    	final Field PField = Utils.getPrivateField(mBuilder.getClass(), "P");
    	assertNotNull(PField);
    	final AlertController.AlertParams P = (AlertController.AlertParams) PField.get(mBuilder);
    	assertNotNull(P.mPositiveButtonListener);
    	assertNotNull(P.mNegativeButtonListener);
    	Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(new Runnable() {
			@Override
			public void run() {
				P.mPositiveButtonListener.onClick(null, DialogInterface.BUTTON_POSITIVE);
	        	P.mNegativeButtonListener.onClick(null, DialogInterface.BUTTON_NEGATIVE);
			}
        });
    	getInstrumentation().waitForIdleSync();
    }
    
    private void clickDialogButtonPositive(Object dialogManager) throws NoSuchFieldException, IllegalArgumentException, IllegalAccessException, InterruptedException {
    	Field mDialogs = Utils.getPrivateField(dialogManager.getClass(), "mDialogs");
    	List<CallScreenDialog> dialogs = (List<CallScreenDialog>) mDialogs.get(dialogManager);
    	assertEquals(1, dialogs.size());
    	CallScreenDialog invitationConfirmDialog = dialogs.get(0);
    	Field mAlertDialog = Utils.getPrivateField(CallScreenDialog.class, "mAlertDialog");
    	final AlertDialog alertDialog = (AlertDialog) mAlertDialog.get(invitationConfirmDialog);
    	assertNotNull(alertDialog);
    	Field mBuilderField = Utils.getPrivateField(CallScreenDialog.class, "mBuilder");
    	final AlertDialog.Builder mBuilder = (AlertDialog.Builder) mBuilderField.get(invitationConfirmDialog);
    	final Field PField = Utils.getPrivateField(mBuilder.getClass(), "P");
    	assertNotNull(PField);
    	final AlertController.AlertParams P = (AlertController.AlertParams) PField.get(mBuilder);
    	assertNotNull(P.mPositiveButtonListener);
    	Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(new Runnable() {
			@Override
			public void run() {
				P.mPositiveButtonListener.onClick(null, DialogInterface.BUTTON_POSITIVE);
			}
        });
    	getInstrumentation().waitForIdleSync();
    }
    
    private void clickDialogButtonNegative(Object dialogManager) throws NoSuchFieldException, IllegalArgumentException, IllegalAccessException, InterruptedException {
    	Field mDialogs = Utils.getPrivateField(dialogManager.getClass(), "mDialogs");
    	List<CallScreenDialog> dialogs = (List<CallScreenDialog>) mDialogs.get(dialogManager);
    	assertEquals(1, dialogs.size());
    	CallScreenDialog invitationConfirmDialog = dialogs.get(0);
    	Field mAlertDialog = Utils.getPrivateField(CallScreenDialog.class, "mAlertDialog");
    	final AlertDialog alertDialog = (AlertDialog) mAlertDialog.get(invitationConfirmDialog);
    	assertNotNull(alertDialog);
    	Field mBuilderField = Utils.getPrivateField(CallScreenDialog.class, "mBuilder");
    	final AlertDialog.Builder mBuilder = (AlertDialog.Builder) mBuilderField.get(invitationConfirmDialog);
    	final Field PField = Utils.getPrivateField(mBuilder.getClass(), "P");
    	assertNotNull(PField);
    	final AlertController.AlertParams P = (AlertController.AlertParams) PField.get(mBuilder);
    	assertNotNull(P.mNegativeButtonListener);
    	Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(new Runnable() {
			@Override
			public void run() {
				P.mNegativeButtonListener.onClick(null, DialogInterface.BUTTON_NEGATIVE);
			}
        });
    	getInstrumentation().waitForIdleSync();
    }
    
    /**
     * This is to test getState() function
     */
    public void testCase10_GetState() throws Throwable {
    	Logger.d(TAG, "testCase10_GetState() entry");
    	final ImageSharingPlugin imageSharingPlugin = new ImageSharingPlugin(mActivity);
    	imageSharingPlugin.setCallScreenHost(mCallScreenHost);
    	imageSharingPlugin.registerForCapabilityChange(PHONE_NUMBER);
    	wait_pluginCreated(imageSharingPlugin);
    	
    	Field mImageSharingStatus = Utils.getPrivateField(ImageSharingPlugin.class, "mImageSharingStatus");
    	mImageSharingStatus.set(imageSharingPlugin, ImageSharingStatus.UNKNOWN);
    	assertEquals(Constants.SHARE_FILE_STATE_IDLE, imageSharingPlugin.getState());
    	mImageSharingStatus.set(imageSharingPlugin, ImageSharingStatus.DECLINE);
    	assertEquals(Constants.SHARE_FILE_STATE_IDLE, imageSharingPlugin.getState());
    	mImageSharingStatus.set(imageSharingPlugin, ImageSharingStatus.COMPLETE);
    	assertEquals(Constants.SHARE_FILE_STATE_DISPLAYING, imageSharingPlugin.getState());
    	mImageSharingStatus.set(imageSharingPlugin, ImageSharingStatus.INCOMING);
    	assertEquals(Constants.SHARE_FILE_STATE_TRANSFERING, imageSharingPlugin.getState());
    }
    
    /**
     * This is to test start() and stop() function
     */
    public void testCase11_StartStop() throws Throwable {
    	Logger.d(TAG, "testCase11_Start() entry");
    	final ImageSharingPlugin imageSharingPlugin = new ImageSharingPlugin(mActivity);
    	imageSharingPlugin.setCallScreenHost(mCallScreenHost);
    	imageSharingPlugin.registerForCapabilityChange(PHONE_NUMBER);
    	wait_pluginCreated(imageSharingPlugin);
    	
    	Field mImageSharingStatus = Utils.getPrivateField(ImageSharingPlugin.class, "mImageSharingStatus");
    	com.mediatek.rcse.plugin.phone.Utils.setInVideoSharing(true);
    	imageSharingPlugin.start(PHONE_NUMBER);
    	Thread.sleep(SLEEP_TIME);
    	com.mediatek.rcse.plugin.phone.Utils.setInVideoSharing(false);
    	mImageSharingStatus.set(imageSharingPlugin, ImageSharingStatus.UNKNOWN);
    	imageSharingPlugin.start(PHONE_NUMBER);
    	Thread.sleep(SLEEP_TIME);
    	imageSharingPlugin.stop();
    	Field mCallScreenDialogManager = Utils.getPrivateField(ImageSharingPlugin.class, "mCallScreenDialogManager");
    	Object dialogManager = mCallScreenDialogManager.get(imageSharingPlugin);
    	Field mDialogs = Utils.getPrivateField(dialogManager.getClass(), "mDialogs");
    	List<CallScreenDialog> dialogs = (List<CallScreenDialog>) mDialogs.get(dialogManager);
    	assertEquals(0, dialogs.size());
    	Thread.sleep(SLEEP_TIME);
    }
    
    /**
     * This is to test showFullImage() function
     */
    public void testCase12_showFullImage() throws Throwable {
    	Logger.d(TAG, "testCase12_showFullImage() entry");
    	final ImageSharingPlugin imageSharingPlugin = new ImageSharingPlugin(mActivity);
    	imageSharingPlugin.setCallScreenHost(mCallScreenHost);
    	imageSharingPlugin.registerForCapabilityChange(PHONE_NUMBER);
    	wait_pluginCreated(imageSharingPlugin);
    	
    	Field mDisplayArea = Utils.getPrivateField(ImageSharingPlugin.class.getSuperclass(), "mDisplayArea");
    	mDisplayArea.set(imageSharingPlugin, mCallScreenHost.requestAreaForDisplay());
    	mIsOnStateChange = false;
    	com.mediatek.rcse.plugin.phone.Utils.setInImageSharing(true);
    	Method showFullImage = Utils.getPrivateMethod(ImageSharingPlugin.class, "showFullImage", String.class);
    	showFullImage.invoke(imageSharingPlugin, getFilePath());
    	long startTime = System.currentTimeMillis();
        while (mIsOnStateChange == false) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("testCase12_showFullImage timeout");
            }
        }
    	assertFalse(com.mediatek.rcse.plugin.phone.Utils.isInImageSharing());
    	assertTrue(mIsOnStateChange);
    }
    
    /**
     * This is to test declineImageSharing() function
     */
    public void testCase13_DeclineImageSharing() throws Throwable {
    	Logger.d(TAG, "testCase13_DeclineImageSharing() entry");
    	final ImageSharingPlugin imageSharingPlugin = new ImageSharingPlugin(mActivity);
    	imageSharingPlugin.setCallScreenHost(mCallScreenHost);
    	imageSharingPlugin.registerForCapabilityChange(PHONE_NUMBER);
    	wait_pluginCreated(imageSharingPlugin);
    	MockRichCallApi richCallApi = new MockRichCallApi(mActivity);
        Field filed = Utils.getPrivateField(ImageSharingPlugin.class.getSuperclass(), "mRichCallApi");
        filed.set(imageSharingPlugin, richCallApi);
        Field mRichCallStatusField = Utils.getPrivateField(ImageSharingPlugin.class.getSuperclass(), "mRichCallStatus");
        mRichCallStatusField.set(imageSharingPlugin, CONNECTED);
        // When CONNECTED
        mIsRejectSession = false;
    	Method declineImageSharing = Utils.getPrivateMethod(ImageSharingPlugin.class, "declineImageSharing");
    	declineImageSharing.invoke(imageSharingPlugin);
    	Thread.sleep(SLEEP_TIME);
    	assertTrue(mIsRejectSession);
    	// When not CONNECTED
    	Field mPendingActionsField = Utils.getPrivateField(ImageSharingPlugin.class, "mPendingActions");
    	ArrayList<Runnable> mPendingActions = (ArrayList<Runnable>) mPendingActionsField.get(imageSharingPlugin);
    	mPendingActions.clear();
    	mRichCallStatusField.set(imageSharingPlugin, DISCONNECTED);
    	declineImageSharing.invoke(imageSharingPlugin);
    	Thread.sleep(SLEEP_TIME);
    	assertEquals(1, mPendingActions.size());
    }
    
    /**
     * This is to test IncomingSessionEventListener
     */
    public void testCase14_IncomingSessionEventListener() throws Throwable {
    	Logger.d(TAG, "testCase14_IncomingSessionEventListener() entry");
    	com.mediatek.rcse.plugin.phone.Utils.resetVibratedStatus();
    	MockImageSharePlugin imageSharingPlugin = new MockImageSharePlugin(mActivity);
        imageSharingPlugin.setCallScreenHost(mCallScreenHost);
        // Wait Plugin created
        wait_pluginCreated(imageSharingPlugin);
        Intent intent = new Intent();
        intent.setAction(IMAGE_SHARING_INVITATION_ACTION);
        intent.putExtra(CONTACT, PHONE_NUMBER);
        intent.putExtra(CONTACT_DISPLAYNAME, "test");
        intent.putExtra(SESSION_ID, "123456");
        intent.putExtra(IMAGE_NAME, "test.jpg");
        intent.putExtra(IMAGE_SIZE, 102L);
        getInstrumentation().getTargetContext().sendBroadcast(intent);
        // Should wait the broadcast to be received
        wait_invitation();
        assertEquals(true, com.mediatek.rcse.plugin.phone.Utils.isVibrated());
        
        Field mIncomingSessionEventListenerField = Utils.getPrivateField(ImageSharingPlugin.class, "mIncomingSessionEventListener");
        IImageSharingEventListener.Stub incomingListener = (IImageSharingEventListener.Stub) mIncomingSessionEventListenerField.get(imageSharingPlugin);
        assertNotNull(incomingListener);
        incomingListener.handleSessionStarted();
        incomingListener.handleSessionAborted(0);
        Field mCallScreenDialogManager = Utils.getPrivateField(ImageSharingPlugin.class, "mCallScreenDialogManager");
    	Object dialogManager = mCallScreenDialogManager.get(imageSharingPlugin);
        waitDialogShow(dialogManager, "showTimeOutDialog");
        incomingListener.handleSessionTerminatedByRemote();
        waitDialogShow(dialogManager, "showTerminatedDialog");
        incomingListener.handleSharingError(ContentSharingError.UNEXPECTED_EXCEPTION);
        waitDialogShow(dialogManager, "showTerminatedDialog");
        incomingListener.handleSharingError(ContentSharingError.SESSION_INITIATION_FAILED);
        waitDialogShow(dialogManager, "showInitFailDialog");
        incomingListener.handleSharingError(ContentSharingError.MEDIA_SAVING_FAILED);
        waitDialogShow(dialogManager, "showNoStorageDialog");
        incomingListener.handleSharingError(ContentSharingError.MEDIA_STREAMING_FAILED);
        Thread.sleep(SLEEP_TIME);
        
        Field mIncomingImageSharingSession = Utils.getPrivateField(ImageSharingPlugin.class, "mIncomingImageSharingSession");
        mIncomingImageSharingSession.set(imageSharingPlugin, new MockIncomingImageSharingSession());
        String filePath = getFilePath();
        mIsOnStateChange = false;
        incomingListener.handleImageTransfered(filePath);
        long startTime = System.currentTimeMillis();
        while (mIsOnStateChange == false) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("testCase14_IncomingSessionEventListener timeout");
            }
        }
        assertTrue(mIsOnStateChange);
        
        Field mImageSharingView = Utils.getPrivateField(ImageSharingPlugin.class, "mImageSharingView");
		mImageSharingView.set(imageSharingPlugin, null);
		assertNull(mImageSharingView.get(imageSharingPlugin));
        incomingListener.handleSharingProgress(MOCK_CURRENT_SIZE, MOCK_TOTAL_SIZE);
        startTime = System.currentTimeMillis();
        while (mImageSharingView.get(imageSharingPlugin) == null) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("testCase14_IncomingSessionEventListener timeout");
            }
        }
        assertNotNull(mImageSharingView.get(imageSharingPlugin));
    }
    
    /**
     * This is to test OutgoingSessionEventListener
     */
    public void testCase15_OutgoingSessionEventListener() throws Throwable {
    	Logger.d(TAG, "testCase15_OutgoingSessionEventListener() entry");
    	com.mediatek.rcse.plugin.phone.Utils.resetVibratedStatus();
    	MockImageSharePlugin imageSharingPlugin = new MockImageSharePlugin(mActivity);
        imageSharingPlugin.setCallScreenHost(mCallScreenHost);
        imageSharingPlugin.registerForCapabilityChange(PHONE_NUMBER);
        wait_pluginCreated(imageSharingPlugin);
    	MockRichCallApi richCallApi = new MockRichCallApi(mActivity);
        Field filed = Utils.getPrivateField(ImageSharingPlugin.class.getSuperclass(), "mRichCallApi");
        filed.set(imageSharingPlugin, richCallApi);
        
        mInitiateImageSharing = false;
        String filePath = getFilePath();
        Method addOutGoingListener = Utils.getPrivateMethod(ImageSharingPlugin.class, "addOutGoingListener", String.class);
        addOutGoingListener.invoke(imageSharingPlugin, filePath);
        long startTime = System.currentTimeMillis();
        while (mInitiateImageSharing == false) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("testCase15_OutgoingSessionEventListener timeout");
            }
        }
        
        Field mCallScreenDialogManager = Utils.getPrivateField(ImageSharingPlugin.class, "mCallScreenDialogManager");
    	Object dialogManager = mCallScreenDialogManager.get(imageSharingPlugin);
        Field mOutgoingSessionEventListenerField = Utils.getPrivateField(ImageSharingPlugin.class, "mOutgoingSessionEventListener");
        IImageSharingEventListener.Stub outgoingListener = (IImageSharingEventListener.Stub) mOutgoingSessionEventListenerField.get(imageSharingPlugin);
        assertNotNull(outgoingListener);
        outgoingListener.handleSessionStarted();
        Thread.sleep(SLEEP_TIME);
        mIsOnStateChange = false;
        outgoingListener.handleSessionAborted(0);
        Thread.sleep(SLEEP_TIME);
        assertTrue(mIsOnStateChange);
        outgoingListener.handleSessionTerminatedByRemote();
        waitDialogShow(dialogManager, "showTerminatedDialog");
        outgoingListener.handleSharingError(ContentSharingError.SESSION_INITIATION_DECLINED);
        waitDialogShow(dialogManager, "showRejectedDialog");
        outgoingListener.handleSharingError(ContentSharingError.UNEXPECTED_EXCEPTION);
        waitDialogShow(dialogManager, "showInitFailDialog");
        outgoingListener.handleSharingError(ContentSharingError.SESSION_INITIATION_CANCELLED);
        waitDialogShow(dialogManager, "showTerminatedDialog");
        outgoingListener.handleSharingError(ContentSharingError.SESSION_INITIATION_FAILED);
        waitDialogShow(dialogManager, "showInitFailDialog");
        outgoingListener.handleSharingError(ContentSharingError.SESSION_INITIATION_TIMEOUT);
        waitDialogShow(dialogManager, "showTimeOutDialog");
        outgoingListener.handleSharingError(ContentSharingError.MEDIA_SAVING_FAILED);
        waitDialogShow(dialogManager, "showNoStorageDialog");
        outgoingListener.handleSharingError(ContentSharingError.MEDIA_STREAMING_FAILED);
        Thread.sleep(SLEEP_TIME);
        
        Field mOutgoingImageSharingSession = Utils.getPrivateField(ImageSharingPlugin.class, "mOutgoingImageSharingSession");
        mOutgoingImageSharingSession.set(imageSharingPlugin, new MockOutgoingImageSharingSession());
        
        com.mediatek.rcse.plugin.phone.Utils.setInImageSharing(true);
        outgoingListener.handleImageTransfered(filePath);
        startTime = System.currentTimeMillis();
        while (com.mediatek.rcse.plugin.phone.Utils.isInImageSharing()) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("testCase15_OutgoingSessionEventListener timeout");
            }
        }
        assertFalse(com.mediatek.rcse.plugin.phone.Utils.isInImageSharing());
        
        Field mImageSharingView = Utils.getPrivateField(ImageSharingPlugin.class, "mImageSharingView");
		mImageSharingView.set(imageSharingPlugin, null);
		assertNull(mImageSharingView.get(imageSharingPlugin));
		outgoingListener.handleSharingProgress(MOCK_CURRENT_SIZE, MOCK_TOTAL_SIZE);
        startTime = System.currentTimeMillis();
        while (mImageSharingView.get(imageSharingPlugin) == null) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("testCase15_OutgoingSessionEventListener timeout");
            }
        }
        assertNotNull(mImageSharingView.get(imageSharingPlugin));
        com.mediatek.rcse.plugin.phone.Utils.setInImageSharing(true);
        outgoingListener.handleSharingProgress(MOCK_TOTAL_SIZE, MOCK_CURRENT_SIZE);
        startTime = System.currentTimeMillis();
        while (com.mediatek.rcse.plugin.phone.Utils.isInImageSharing()) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("testCase15_OutgoingSessionEventListener timeout");
            }
        }
        assertFalse(com.mediatek.rcse.plugin.phone.Utils.isInImageSharing());
    }
    
    /**
     * This is to test ImageSharingInvitationReceiver class
     */
    public void testCase16_ImageSharingInvitationReceiver() throws Throwable {
    	Logger.d(TAG, "testCase16_ImageSharingInvitationReceiver() entry");
    	final ImageSharingPlugin imageSharingPlugin = new ImageSharingPlugin(mActivity);
         imageSharingPlugin.setCallScreenHost(mCallScreenHost);
         imageSharingPlugin.registerForCapabilityChange(PHONE_NUMBER);
         wait_pluginCreated(imageSharingPlugin);
         MockRichCallApi richCallApi = new MockRichCallApi(mActivity);
         Field filed = Utils.getPrivateField(ImageSharingPlugin.class.getSuperclass(), "mRichCallApi");
         filed.set(imageSharingPlugin, richCallApi);
         
         Field mImageSharingInvitationReceiverField = Utils.getPrivateField(ImageSharingPlugin.class, "mImageSharingInvitationReceiver");
         BroadcastReceiver mImageSharingInvitationReceiver = (BroadcastReceiver) mImageSharingInvitationReceiverField.get(imageSharingPlugin);

         // INVITATION_ACTION
         Intent invitationIt = new Intent();
         invitationIt.setAction(IMAGE_SHARING_INVITATION_ACTION);
         invitationIt.putExtra(CONTACT, PHONE_NUMBER);
         invitationIt.putExtra(CONTACT_DISPLAYNAME, "test");
         invitationIt.putExtra(SESSION_ID, "123456");
         invitationIt.putExtra(IMAGE_NAME, "test.jpg");
         invitationIt.putExtra(IMAGE_SIZE, Long.MAX_VALUE);
         Field mRichCallStatusField = Utils.getPrivateField(ImageSharingPlugin.class.getSuperclass(), "mRichCallStatus");
         mRichCallStatusField.set(imageSharingPlugin, DISCONNECTED);
         mIsRejectSession = false;
         mImageSharingInvitationReceiver.onReceive(mActivity, invitationIt);
         Thread.sleep(SLEEP_TIME);
         assertTrue(mIsRejectSession);
         
         // START_ACTION
         String imageName = getFilePath();
         Intent startSharingIt = new Intent();
         startSharingIt.setAction(IMAGE_SHARING_START_ACTION);
         startSharingIt.putExtra("imageName", imageName);
         com.mediatek.rcse.plugin.phone.Utils.setInImageSharing(false);
         mImageSharingInvitationReceiver.onReceive(mActivity, startSharingIt);
         assertTrue(com.mediatek.rcse.plugin.phone.Utils.isInImageSharing());
         // error conditions
         imageSharingPlugin.setCallScreenHost(null);
         mImageSharingInvitationReceiver.onReceive(mActivity, startSharingIt);
         mImageSharingInvitationReceiver.onReceive(mActivity, null);
         Intent unknownIt = new Intent();
         unknownIt.setAction(IMAGE_SHARING_UNKNOWN_ACTION);
         mImageSharingInvitationReceiver.onReceive(mActivity, unknownIt);
    }
    
    /**
     * This is to test addIncomingImageSharingListener() function
     */
    public void testCase17_AddIncomingImageSharingListener() throws Throwable {
    	Logger.d(TAG, "testCase17_AddIncomingImageSharingListener() entry");
    	final ImageSharingPlugin imageSharingPlugin = new ImageSharingPlugin(mActivity);
        imageSharingPlugin.setCallScreenHost(mCallScreenHost);
        imageSharingPlugin.registerForCapabilityChange(PHONE_NUMBER);
        wait_pluginCreated(imageSharingPlugin);
        MockRichCallApi richCallApi = new MockRichCallApi(mActivity);
        Field filed = Utils.getPrivateField(ImageSharingPlugin.class.getSuperclass(), "mRichCallApi");
        filed.set(imageSharingPlugin, richCallApi);
        
        Method addIncomingImageSharingListener = Utils.getPrivateMethod(ImageSharingPlugin.class, "addIncomingImageSharingListener");
        Field mRichCallStatusField = Utils.getPrivateField(ImageSharingPlugin.class.getSuperclass(), "mRichCallStatus");
        mRichCallStatusField.set(imageSharingPlugin, CONNECTED);
        mIsAddSessionListener = false;
        addIncomingImageSharingListener.invoke(imageSharingPlugin);
        assertTrue(mIsAddSessionListener);
        
        Field mPendingActionsField = Utils.getPrivateField(ImageSharingPlugin.class, "mPendingActions");
        ArrayList<Runnable> mPendingActions = (ArrayList<Runnable>) mPendingActionsField.get(imageSharingPlugin);
        mPendingActions.clear();
        mRichCallStatusField.set(imageSharingPlugin, DISCONNECTED);
        addIncomingImageSharingListener.invoke(imageSharingPlugin);
        assertEquals(1, mPendingActions.size());
    }
    
    /**
     * This is to test addOutgoingImageSharingListener() function
     */
    public void testCase18_AddOutgoingImageSharingListener() throws Throwable {
    	Logger.d(TAG, "testCase18_AddOutgoingImageSharingListener() entry");
    	ImageSharingPlugin imageSharingPlugin = new ImageSharingPlugin(mActivity);
        imageSharingPlugin.setCallScreenHost(mCallScreenHost);
        imageSharingPlugin.registerForCapabilityChange(PHONE_NUMBER);
        wait_pluginCreated(imageSharingPlugin);
        MockRichCallApi richCallApi = new MockRichCallApi(mActivity);
        Field filed = Utils.getPrivateField(ImageSharingPlugin.class.getSuperclass(), "mRichCallApi");
        filed.set(imageSharingPlugin, richCallApi);
        
        Method addOutGoingListener = Utils.getPrivateMethod(ImageSharingPlugin.class, "addOutGoingListener", String.class);
        mIsAddSessionListener = false;
        addOutGoingListener.invoke(imageSharingPlugin, getFilePath());
        Thread.sleep(SLEEP_TIME);
        assertTrue(mIsAddSessionListener);
        
        Method finishImageSharing = Utils.getPrivateMethod(ImageSharingPlugin.class,"finishImageSharing");
        finishImageSharing.invoke(imageSharingPlugin);
        imageSharingPlugin = null;
    }
    
    /**
     * This is to test CompressDialog class
     */
    public void testCase19_CompressDialog() throws Throwable {
    	Logger.d(TAG, "testCase19_CompressDialog() entry");
    	ImageSharingPlugin imageSharingPlugin = new ImageSharingPlugin(mActivity);
    	imageSharingPlugin.setCallScreenHost(mCallScreenHost);
    	imageSharingPlugin.registerForCapabilityChange(PHONE_NUMBER);
    	wait_pluginCreated(imageSharingPlugin);
    	
    	Method prepareStartImageSharing = Utils.getPrivateMethod(ImageSharingPlugin.class, "prepareStartImageSharing", String.class);
        assertNotNull(prepareStartImageSharing);
        prepareStartImageSharing.invoke(imageSharingPlugin, getFilePath());
    	
        Thread.sleep(2000);
    	final Activity activity = mCallScreenHost.getCallScreenActivity();
    	assertNotNull(activity);
    	
    	final CompressDialog compressDialog = imageSharingPlugin.new CompressDialog();
    	compressDialog.setOrigFileName(MOCK_ORI_NAME);
    	Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(new Runnable() {
            @Override
            public void run() {
            	compressDialog.show(activity.getFragmentManager(), "CompressDialog");
            }
        });
        getInstrumentation().waitForIdleSync();
        //mCheckNotRemind
        Field mCheckNotRemindFiledField = Utils.getPrivateField(CompressDialog.class, "mCheckNotRemind");
        final CheckBox mCheckNotRemind = (CheckBox) mCheckNotRemindFiledField.get(compressDialog);
        mainHandler.post(new Runnable() {
			@Override
			public void run() {
				mCheckNotRemind.setChecked(true);
				
			}
        });
        getInstrumentation().waitForIdleSync();
        Method handleOk = Utils.getPrivateMethod(CompressDialog.class, "handleOk");
        handleOk.invoke(compressDialog);
        Thread.sleep(SLEEP_TIME);
        assertEquals(true, RcsSettings.getInstance().isEnabledCompressingImage());
        // onSaveInstance
        final Bundle saveBundle = new Bundle();
        mainHandler.post(new Runnable() {
            @Override
            public void run() {
            	compressDialog.onSaveInstanceState(saveBundle);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertNotNull(saveBundle.getString("CompressDialog"));
        // onClick
        mainHandler.post(new Runnable() {
            @Override
            public void run() {
            	compressDialog.onClick(null, DialogInterface.BUTTON_POSITIVE);
            	compressDialog.onClick(null, DialogInterface.BUTTON_NEGATIVE);
            }
        });
        getInstrumentation().waitForIdleSync();
        // hanldeCancel
        final Method handleCancel = Utils.getPrivateMethod(CompressDialog.class, "handleCancel");
        mainHandler.post(new Runnable() {
        	 @Override
             public void run() {
        		 try {
					handleCancel.invoke(compressDialog);
				} catch (IllegalArgumentException e) {
					e.printStackTrace();
				} catch (IllegalAccessException e) {
					e.printStackTrace();
				} catch (InvocationTargetException e) {
					e.printStackTrace();
				}
        	 }
        });

        Method finishImageSharing = Utils.getPrivateMethod(ImageSharingPlugin.class,"finishImageSharing");
        finishImageSharing.invoke(imageSharingPlugin);
        imageSharingPlugin = null;
    }
    
    /**
     * This is to test SharingPlugin#RichCallApiListener class
     */
    public void testCase20_RichCallApiListener() throws Throwable {
    	Logger.d(TAG, "testCase20_RichCallApiListener() entry");
    	final ImageSharingPlugin imageSharingPlugin = new ImageSharingPlugin(mActivity);
        imageSharingPlugin.setCallScreenHost(mCallScreenHost);
        imageSharingPlugin.registerForCapabilityChange(PHONE_NUMBER);
        wait_pluginCreated(imageSharingPlugin);
        MockRichCallApi richCallApi = new MockRichCallApi(mActivity);
        Field filed = Utils.getPrivateField(ImageSharingPlugin.class.getSuperclass(), "mRichCallApi");
        filed.set(imageSharingPlugin, richCallApi);
        
        Field mRichCallApiListenerField = Utils.getPrivateField(ImageSharingPlugin.class.getSuperclass(), "mRichCallApiListener");
        ClientApiListener mRichCallApiListener = (ClientApiListener) mRichCallApiListenerField.get(imageSharingPlugin);
        assertNotNull(mRichCallApiListener);
        Field mRichCallStatusField = Utils.getPrivateField(ImageSharingPlugin.class.getSuperclass(), "mRichCallStatus");
        mRichCallStatusField.set(imageSharingPlugin, CONNECTING);
        mRichCallApiListener.handleApiConnected();
        assertEquals(CONNECTED, mRichCallStatusField.get(imageSharingPlugin));
        mRichCallApiListener.handleApiDisabled();
        assertEquals(DISCONNECTED, mRichCallStatusField.get(imageSharingPlugin));
        mRichCallStatusField.set(imageSharingPlugin, CONNECTING);
        mRichCallApiListener.handleApiDisconnected();
        assertEquals(DISCONNECTED, mRichCallStatusField.get(imageSharingPlugin));
    }
    
    private void waitDialogShow(Object dialogManager, String methodName) throws NoSuchMethodException, IllegalArgumentException, IllegalAccessException, InvocationTargetException, NoSuchFieldException, InterruptedException {
    	Field mDialogs = Utils.getPrivateField(dialogManager.getClass(), "mDialogs");
    	List<CallScreenDialog> dialogs = (List<CallScreenDialog>) mDialogs.get(dialogManager);
    	dialogs.clear();
    	Method showNoStorageDialog = Utils.getPrivateMethod(dialogManager.getClass(), methodName);
    	showNoStorageDialog.invoke(dialogManager);
    	long startTime = System.currentTimeMillis();
        while (dialogs.size() == 0) {
            Thread.sleep(SLEEP_TIME);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("waitDialogShow timeout");
            }
        }
    	assertEquals(1, dialogs.size());
    }
    
    private static final class ImageSharingStatus {
        private static final int UNKNOWN = 0;
        private static final int INCOMING = 1;
        private static final int INCOMING_BLOCKING = 2;
        private static final int OUTGOING = 3;
        private static final int OUTGOING_BLOCKING = 4;
        private static final int COMPLETE = 5;
        private static final int DECLINE = 6;
    }
    
    /**
     * Get a image file path from database
     */
    private String getFilePath() {
        Logger.v(TAG, "getFilePath()");
        Context context = getInstrumentation().getTargetContext();
        Cursor cursor = null;
        String filePath = null;
        try {
            cursor = context.getContentResolver().query(
                    MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
            if (null != cursor && cursor.getCount() > 0) {
                cursor.moveToFirst();
                filePath = cursor.getString(cursor
                        .getColumnIndex(MediaStore.Images.ImageColumns.DATA));
            } else {
                fail("testCase2_SetAsyncImage() Cannot find image in sdcard");
            }
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        Logger.v(TAG, "getFilePath() out, filePath is " + filePath);
        return filePath;
    }

    private void wait_invitation() throws InterruptedException {
        Logger.d(TAG, "wait_invitation()");
        long startTime = System.currentTimeMillis();
        while (!com.mediatek.rcse.plugin.phone.Utils.isVibrated()) {
            if (System.currentTimeMillis() - startTime > WAIT_TIME) {
                return;
            }
            Thread.sleep(SLEEP_TIME);
        }
    }

    private boolean wait_pluginCreated(ImageSharingPlugin imageSharingPlugin)
            throws NoSuchFieldException, IllegalArgumentException, IllegalAccessException,
            InterruptedException {
    	Field field = null;
    	if (imageSharingPlugin instanceof MockImageSharePlugin) {
    		field = imageSharingPlugin.getClass().getSuperclass()
                	.getDeclaredField("mImageSharingInvitationReceiver");
    	} else {
    		field = imageSharingPlugin.getClass().getDeclaredField("mImageSharingInvitationReceiver");
    	}
        field.setAccessible(true);
        BroadcastReceiver imageSharingInvitationReceiver = null;
        long startTime = System.currentTimeMillis();
        while ((imageSharingInvitationReceiver = (BroadcastReceiver) field.get(imageSharingPlugin)) == null) {
            if (System.currentTimeMillis() - startTime > WAIT_TIME) {
                fail("ImageSharingPlugin did not be ready");
            }
            Thread.sleep(SLEEP_TIME);
        }
        return imageSharingInvitationReceiver != null;
    }

    private void waitForHaveImage() throws InterruptedException {
        long startTime = System.currentTimeMillis();
        while (mImageSharingView.getChildCount() == 0) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }
    }

    private void waitForImageNotNull() throws InterruptedException, NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        long startTime = System.currentTimeMillis();
        while (mImageSharingView == null) {
            Field field = Utils.getPrivateField(ImageSharingPlugin.class, "mImageSharingView");
            mImageSharingView = (ViewGroup) field.get(mImageSharingPlugin);
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            }
            Thread.sleep(SLEEP_TIME);
        }
    }

    private ICallScreenHost mCallScreenHost = new ICallScreenHost() {
        public ViewGroup requestAreaForDisplay() {
            Logger.d(TAG, "requestAreaForDisplay() entry!");
            return (ViewGroup) mActivity.findViewById(android.R.id.content).getRootView();
        }

        public void onStateChange(int arg0) {
        	mIsOnStateChange = true;
        }

        public void onCapabilityChange(String arg0, boolean arg1) {
        }

        public Activity getCallScreenActivity() {
            return mActivity;
        }
    };

    private class MockImageSharePlugin extends ImageSharingPlugin {
        public boolean imageShareSupported = true;
        public boolean videoShareSupported = true;

        public MockImageSharePlugin(Context context) {
            super(context);
        }

        @Override
        protected void imageShareNotSupported() {
            imageShareSupported = false;
        }

        @Override
        protected void videoShareNotSupported() {
            videoShareSupported = false;
        }

        @Override
        public boolean isImageShareSupported(String number) {
            return false;
        }

        @Override
        public boolean isVideoShareSupported(String number) {
            return false;
        }

        @Override
        public boolean getCapability(String number) {
            Logger.d(TAG, "getCapability(), number = " + number
                    + ", for test case always return true.");
            return true;
        }
    }

    private class MockIncomingImageSharingSession extends IImageSharingSession.Stub {

		@Override
		public void acceptSession() throws RemoteException {
			
		}

		@Override
		public void addSessionListener(IImageSharingEventListener listener)
				throws RemoteException {
			mIsAddSessionListener = true;
		}

		@Override
		public void cancelSession() throws RemoteException {
			
		}

		@Override
		public String getFilename() throws RemoteException {
			return null;
		}

		@Override
		public long getFilesize() throws RemoteException {
			return 0;
		}

		@Override
		public String getRemoteContact() throws RemoteException {
			return null;
		}

		@Override
		public String getSessionID() throws RemoteException {
			return null;
		}

		@Override
		public int getSessionState() throws RemoteException {
			return 0;
		}

		@Override
		public void rejectSession() throws RemoteException {
			mIsRejectSession = true;
		}

		@Override
		public void removeSessionListener(IImageSharingEventListener listener)
				throws RemoteException {
		}

		@Override
		public byte[] getFileThumbnail() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public int getSessionDirection() throws RemoteException {
			// TODO Auto-generated method stub
			return 0;
		}
    }
    
    private class MockIncomingImageSharingSession2 extends IImageSharingSession.Stub {

		@Override
		public void acceptSession() throws RemoteException {
			
		}

		@Override
		public void addSessionListener(IImageSharingEventListener listener)
				throws RemoteException {
			
		}

		@Override
		public void cancelSession() throws RemoteException {
			
		}

		@Override
		public String getFilename() throws RemoteException {
			return null;
		}

		@Override
		public long getFilesize() throws RemoteException {
			return Long.MAX_VALUE;
		}

		@Override
		public String getRemoteContact() throws RemoteException {
			return null;
		}

		@Override
		public String getSessionID() throws RemoteException {
			return null;
		}

		@Override
		public int getSessionState() throws RemoteException {
			return 0;
		}

		@Override
		public void rejectSession() throws RemoteException {
		}

		@Override
		public void removeSessionListener(IImageSharingEventListener listener)
				throws RemoteException {
		}

		@Override
		public byte[] getFileThumbnail() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public int getSessionDirection() throws RemoteException {
			// TODO Auto-generated method stub
			return 0;
		}
    }
    
    private class MockOutgoingImageSharingSession extends IImageSharingSession.Stub {

		@Override
		public void acceptSession() throws RemoteException {
			
		}

		@Override
		public void addSessionListener(IImageSharingEventListener listener)
				throws RemoteException {
			mIsAddSessionListener = true;
		}

		@Override
		public void cancelSession() throws RemoteException {
			
		}

		@Override
		public String getFilename() throws RemoteException {
			return null;
		}

		@Override
		public long getFilesize() throws RemoteException {
			return 0;
		}

		@Override
		public String getRemoteContact() throws RemoteException {
			return null;
		}

		@Override
		public String getSessionID() throws RemoteException {
			return null;
		}

		@Override
		public int getSessionState() throws RemoteException {
			return 0;
		}

		@Override
		public void rejectSession() throws RemoteException {
		}

		@Override
		public void removeSessionListener(IImageSharingEventListener listener)
				throws RemoteException {
		}

		@Override
		public byte[] getFileThumbnail() throws RemoteException {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public int getSessionDirection() throws RemoteException {
			// TODO Auto-generated method stub
			return 0;
		}
    }
    
    /**
     * Mock RichCallApi with a mocked VF account and a mocked number.
     */
    private class MockRichCallApi extends RichCallApi {

        public MockRichCallApi(Context ctx) {
            super(ctx);
        }

        @Override
        public String getVfAccountViaNumber(String number) throws ClientApiException {
            if (number.equals(MOCK_NMUBER)) {
                return MOCK_VF_ACCOUNT;
            }
            return null;
        }

        @Override
        public String getNumberViaVfAccount(String account) throws ClientApiException {
            if (account.equals(MOCK_VF_ACCOUNT)) {
                return MOCK_NMUBER;
            }
            return null;
        }
        
        @Override
        public IImageSharingSession getImageSharingSession(String id)
        		throws ClientApiException {
        	return new MockIncomingImageSharingSession();
        }
        
        @Override
        public IImageSharingSession initiateImageSharing(String contact,
        		String file) throws ClientApiException {
        	mInitiateImageSharing = true;
        	return new MockOutgoingImageSharingSession();
        }
    }
}
