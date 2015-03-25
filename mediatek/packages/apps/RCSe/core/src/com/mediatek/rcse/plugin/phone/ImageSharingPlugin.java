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

package com.mediatek.rcse.plugin.phone;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.Bitmap;
import android.media.MediaFile;
import android.media.ThumbnailUtils;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.provider.MediaStore.Images;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.webkit.MimeTypeMap;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import android.graphics.BitmapFactory;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.PluginApiManager;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.core.ims.service.richcall.ContentSharingError;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.ClientApiException;
import com.orangelabs.rcs.service.api.client.richcall.IImageSharingEventListener;
import com.orangelabs.rcs.service.api.client.richcall.IImageSharingSession;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * This class defined to implement the function interface of ICallScreenPlugIn,
 * and archive the main function here
 */
public class ImageSharingPlugin extends SharingPlugin {
    private static final String TAG = "ImageSharingPlugin";
    /* package */static final String IMAGE_SHARING_INVITATION_ACTION =
            "com.orangelabs.rcs.richcall.IMAGE_SHARING_INVITATION";
    /* package */static final String IMAGE_SHARING_START_ACTION =
            "com.mediatek.phone.plugin.IMAGE_SHARING_START_ACTION";
    /* package */static final String IMAGE_SHARING_REPIC_ACTION =
            "com.mediatek.phone.plugin.IMAGE_SHARING_REPIC_ACTION";
    /* package */static final String IMAGE_SHARING_WARN_ACTION =
            "com.mediatek.phone.plugin.IMAGE_SHARING_WARN_ACTION";
    /* package */static final String IMAGE_NAME = "imageName";
    /* package */static final String SELECT_TYPE = "select_type";
    private ImageSharingInvitationReceiver mImageSharingInvitationReceiver = null;

    private int mImageSharingStatus = ImageSharingStatus.UNKNOWN;

    private IImageSharingSession mOutgoingImageSharingSession = null;
    private View mImageSharingView = null;

    private IImageSharingSession mIncomingImageSharingSession = null;
    private IncomingSessionEventListener mIncomingSessionEventListener = null;
    private OutgoingSessionEventListener mOutgoingSessionEventListener = null;

    private static final String FILE_UNIT = "KB";
    private static final double FILE_UNIT_SIZE = 1024;
    // For incoming image share information
    private String mIncomingContact = null;
    private String mIncomingContactDisplayname = null;
    private String mIncomingSessionId = null;
    private String mIncomingImageName = null;
    private String mOutGoingImageName = null;
	private byte[] mIncomingThumbnail = null;
    private long mIncomingImageSize = 0;
    private CompressDialog mCompressDialog = null;

    private static final String URI_PREFIX = "file://";
    private static final String IDENTIFY_TEL = "tel:";
    private static final int MAX_SIZE_PIXELS = 3 * 1024 * 1024;

    private CallScreenDialogManager mCallScreenDialogManager = null;
    private ArrayList<Runnable> mPendingActions = new ArrayList<Runnable>();
    private WarningDialog mReceiveWarningDialog = null;
    private WarningDialog mInviteWarningDialog = null;
    private RepickDialog mRepickDialog = null;  
    private static final int entry_initiate = 0;
    private static final int entry_terminated = 1;
    private long mMaxImageSharingSize = 0;
    private static final int COLOR = 0xFFFFFFFF;

    public static final class ImageSharingStatus {
        private static final int UNKNOWN = 0;
        private static final int INCOMING = 1;
        private static final int INCOMING_BLOCKING = 2;
        private static final int OUTGOING = 3;
        private static final int OUTGOING_BLOCKING = 4;
        private static final int COMPLETE = 5;
        private static final int DECLINE = 6;
    }

    private ISharingPlugin mImageSharingCallBack = new ISharingPlugin() {

        @Override
        public void onApiConnected() {
            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                    // there may be an case that content sharing plugin starts
                    // earlier than core service, so we need to refresh data
                    // that may be modified by auto-configuration
                    PhoneUtils.initialize(mContext);
                    mMaxImageSharingSize = RcsSettings.getInstance().getMaxImageSharingSize() * 1024;
                }
            });
            int size = mPendingActions.size();
            for (int i = 0; i < size; i++) {
                mPendingActions.get(i).run();
            }
            mPendingActions.clear();
        }

        @Override
        public void onFinishSharing() {
            finishImageSharing();
        }

    };

    /**
     * Constructor
     * 
     * @param ctx Application context
     */
    public ImageSharingPlugin(final Context context) {
        super(context);
        Logger.d(TAG, "ImageSharingPlugin contructor entry, context: " + context);
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                Logger.v(TAG, "ImageSharingPlugin contructor doInBackground entry");
                RcsSettings rcsSetting = RcsSettings.getInstance();
                if (rcsSetting != null) {
                    mMaxImageSharingSize = rcsSetting.getMaxImageSharingSize() * 1024;
                    Logger.v(TAG,
                            "ImageSharingPlugin contructor doInBackground  mMaxImageSharingSize = "
                                    + mMaxImageSharingSize);
                } else {
                    Logger
                            .e(TAG,
                                    "ImageSharingPlugin contructor doInBackground RcsSettings.getInstance() return null");
                }
                Logger.v(TAG, "ImageSharingPlugin contructor doInBackground exit");
                return null;
            }

            @Override
            protected void onPostExecute(Void result) {
                Logger.v(TAG, "ImageSharingPlugin contructor onPostExecute entry");
                mImageSharingInvitationReceiver = new ImageSharingInvitationReceiver();
                // Register rich call invitation listener
                IntentFilter intentFilter = new IntentFilter();
                intentFilter.addAction(IMAGE_SHARING_INVITATION_ACTION);
                intentFilter.addAction(IMAGE_SHARING_START_ACTION);
                intentFilter.addAction(IMAGE_SHARING_REPIC_ACTION);
                intentFilter.addAction(IMAGE_SHARING_WARN_ACTION);
                mContext.registerReceiver(mImageSharingInvitationReceiver, intentFilter);
                mCountDownLatch.countDown();
                Logger.v(TAG, "ImageSharingPlugin contructor onPostExecute exit");
            }
        }.execute();
        mCallScreenDialogManager = new CallScreenDialogManager();
        mInterface = mImageSharingCallBack;
    }

    private void startImageSharing(final String imageName) {
        Logger.d(TAG, "startImageSharing entry, with image name: " + imageName
                + " mImageSharingStatus: " + mImageSharingStatus);
        showThumbnails(imageName);
        if (mRichCallStatus == RichCallStatus.CONNECTED) {
            mImageSharingStatus = ImageSharingStatus.OUTGOING;
            addOutGoingListener(imageName);
        } else {
            mImageSharingStatus = ImageSharingStatus.OUTGOING_BLOCKING;
            mPendingActions.add(new Runnable() {
                @Override
                public void run() {
                    addOutGoingListener(imageName);
                }
            });
        }
        Logger.d(TAG, "startImageSharing exit");
    }

    private void addOutGoingListener(final String imageName) {
        Logger.d(TAG, "addOutGoingListener entry");
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                boolean success = false;
                mOutgoingSessionEventListener = new OutgoingSessionEventListener(imageName);
                try {
                    mOutgoingImageSharingSession = mRichCallApi.initiateImageSharing(IDENTIFY_TEL
                            + getVodafoneAccount(mNumber), imageName,RcsSettings.getInstance().isFileTransferThumbnailSupported());
                    Logger.d(TAG, "addOutGoingListener() mOutgoingImageSharingSession: "
                            + mOutgoingImageSharingSession);
                    if (mOutgoingImageSharingSession != null) {
                        mOutgoingImageSharingSession
                                .addSessionListener(mOutgoingSessionEventListener);
                        success = true;
                    }
                } catch (ClientApiException e) {
                    e.printStackTrace();
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
                if (!success) {
                    reset();
                }
                Logger.d(TAG, "addOutGoingListener() success: " + success);
            }
        });
        Logger.d(TAG, "addOutGoingListener exit");
    }

    private class ThumbnailAsyncTask extends AsyncTask<Void, Void, Bitmap> {
        private String mImageName = null;
        private long mFileSize = 0;

        public ThumbnailAsyncTask(String imageName) {
            mImageName = imageName;
        }

        @Override
        protected Bitmap doInBackground(Void... params) {
            Logger.d(TAG, "ThumbnailAsyncTask doInBackground entry");
            Bitmap image = null;
            if (com.mediatek.rcse.service.Utils.isFileExist(mImageName)) {
                image =
                        ThumbnailUtils
                                .createImageThumbnail(mImageName, Images.Thumbnails.MINI_KIND);
                int degrees = com.mediatek.rcse.service.Utils.getDegreesRotated(mImageName);
                if (0 != degrees) {
                    image = com.mediatek.rcse.service.Utils.rotate(image, degrees);
                } else {
                    Logger.d(TAG, "ThumbnailAsyncTask file degress is zero, so no need to rotate");
                }
            } else {
                Logger.e(TAG, "ThumbnailAsyncTask the file " + mImageName + " doesn't exist!");
            }
            java.io.File file = new File(mImageName);
            mFileSize = file.length();
            Logger.d(TAG, "ThumbnailAsyncTask doInBackground exit");
            return image;
        }

        @Override
        protected void onPostExecute(Bitmap bitmap) {
            Logger.d(TAG, "ThumbnailAsyncTask onPostExecute entry");
            ImageView view = (ImageView) mImageSharingView.findViewById(R.id.shared_image);
            if (view != null) {
                view.setImageBitmap(bitmap);
            } else {
                Logger.d(TAG, "ThumbnailAsyncTask view is null");
            }
            final ImageView cancelView =
                    (ImageView) mImageSharingView.findViewById(R.id.cancel_image);
            cancelView.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    mCallScreenDialogManager.showTerminateConfirmDialog();
                }
            });
            TextView fileNameView = (TextView) mImageSharingView.findViewById(R.id.file_name);
            if (mImageName != null) {
                fileNameView.setText(mImageName.substring(mImageName.lastIndexOf("/") + 1));
            } else {
                Logger.d(TAG, "ThumbnailAsyncTask imageName is null");
            }
            TextView fileSizeView = (TextView) mImageSharingView.findViewById(R.id.file_size);
            if (fileSizeView != null) {
                String value = roundDouble((double) mFileSize / FILE_UNIT_SIZE, 1) + FILE_UNIT;
                fileSizeView.setText(value);
            } else {
                Logger.d(TAG, "ThumbnailAsyncTask fileSizeView is null");
            }
            Logger.d(TAG, "ThumbnailAsyncTask onPostExecute exit");
        }
    }

    private void showThumbnails(final String imageName) {
        Logger.v(TAG, "showThumbnails entry");
        initTransferringLayout();
        if (mCallScreenHost != null) {
            if (mDisplayArea == null) {
                // Here requestAreaForDisplay() may return null.
                mDisplayArea = mCallScreenHost.requestAreaForDisplay();
            }
            if (mDisplayArea != null) {
                mDisplayArea.removeAllViews();
                mDisplayArea.addView(mImageSharingView);
                mCallScreenHost.onStateChange(getCurrentState());
            } else {
                Logger.d(TAG, "ThumbnailAsyncTask mDisplayArea is:" + mDisplayArea);
            }
        } else {
            Logger.d(TAG, "ThumbnailAsyncTask mCallScreenHost is null");
        }
        new ThumbnailAsyncTask(imageName).execute();
        Logger.v(TAG, "showThumbnails exit");
    }

    /**
     * Outgoing image sharing session event listener
     */
    private class OutgoingSessionEventListener extends IImageSharingEventListener.Stub {
        private static final String TAG = "OutgoingSessionEventListener";
        private String mFile = null;

        public OutgoingSessionEventListener(String file) {
            mFile = file;
        }

        // Session is started
        public void handleSessionStarted() {
            Logger.v(TAG, "handleSessionStarted()");
            showAcceptedToast();
        }

        // Session has been aborted
        public void handleSessionAborted(int reason) {
            Logger.v(TAG, "handleSessionAborted()");
            finishImageSharing();
        }

        // Session has been terminated by remote
        public void handleSessionTerminatedByRemote() {
            Logger.v(TAG, "handleSessionTerminatedByRemote()");
            mCallScreenDialogManager.showTerminatedDialog();
            finishImageSharing();
        }

        // Content sharing error
        public void handleSharingError(final int error) {
            Logger.v(TAG, "handleSharingError(), error = " + error);
            switch (error) {
                case ContentSharingError.SESSION_INITIATION_DECLINED:
                    mCallScreenDialogManager.showRejectedDialog();
                    break;
                case ContentSharingError.UNEXPECTED_EXCEPTION:
                    mCallScreenDialogManager.showInitFailDialog();
                    break;
                case ContentSharingError.SESSION_INITIATION_CANCELLED:
                    mCallScreenDialogManager.showTerminatedDialog();
                    break;
                case ContentSharingError.SESSION_INITIATION_FAILED:
                    mCallScreenDialogManager.showInitFailDialog();
                    break;
                case ContentSharingError.SESSION_INITIATION_TIMEOUT:
                    mCallScreenDialogManager.showTimeOutDialog();
                    break;
                case ContentSharingError.MEDIA_SAVING_FAILED:
                    mCallScreenDialogManager.showNoStorageDialog();
                    break;
                default:
                    break;
            }
            finishImageSharing();
        }

        @Override
        public void handleImageTransfered(String arg0) throws RemoteException {
            Logger.d(TAG, "handleImageTransfered entry, file: " + arg0);
            showFullImage(arg0);
            mOutgoingImageSharingSession.removeSessionListener(mOutgoingSessionEventListener);
            if (mWakeLockCount.decrementAndGet() >= 0) {
                mWakeLock.release();
            }
            Logger.d(TAG, "handleImageTransfered exit");
        }

        @Override
        public void handleSharingProgress(final long arg0, final long arg1) throws RemoteException {
            Logger.d(TAG, "handleSharingProgress entry");
            /*
             * work around for ALPS00288583, because SUCCESS-REPORT cannot be
             * received on the sender side.
             */
            if (arg0 < arg1) {
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        updateProgressBar(arg0, arg1);
                    }
                });
            } else {
                Logger.d(TAG, "handleSharingProgress handleImageTransfered");
                handleImageTransfered(mFile);
            }
            Logger.d(TAG, "handleSharingProgress exit");
        }
    };

    /*
     * finish image sharing when there is abort, error, terminated or cancel
     * happened.
     */
    private void finishImageSharing() {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                Logger.d(TAG, "finishImageSharing entry");
                if (mWakeLockCount.decrementAndGet() >= 0) {
                    mWakeLock.release();
                }
                reset();
                if (mDisplayArea != null) {
                    mDisplayArea.removeAllViews();
                } else {
                    Logger.d(TAG, "finishImageSharing mDisplayArea is null");
                }
                if (mCallScreenHost != null) {
                    mCallScreenHost.onStateChange(getCurrentState());
                } else {
                    Logger.w(TAG, "finishImageSharing mCallScreenHost is null");
                }
                Logger.d(TAG, "finishImageSharing exit");
            }
        });
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                Logger.d(TAG, "finishImageSharing doInBackground entry");
                clearSavedDialogs();
                if (mOutgoingImageSharingSession != null) {
                    try {
                        mOutgoingImageSharingSession
                                .removeSessionListener(mOutgoingSessionEventListener);
                        mOutgoingImageSharingSession.cancelSession();
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                } else {
                    Logger.d(TAG, "finishImageSharing mOutgoingImageSharingSession is null");
                }
                if (mIncomingImageSharingSession != null) {
                    try {
                        mIncomingImageSharingSession
                                .removeSessionListener(mIncomingSessionEventListener);
                        mIncomingImageSharingSession.cancelSession();
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                } else {
                    Logger.d(TAG, "finishImageSharing mIncomingImageSharingSession is null");
                }
                Logger.d(TAG, "finishImageSharing doInBackground exit");
                return null;
            }
        }.execute();
    }

    private void reset() {
        Logger.v(TAG, "reset() entry");
        mImageSharingStatus = ImageSharingStatus.UNKNOWN;
        Utils.setInImageSharing(false);
        Logger.v(TAG, "reset() exit");
    }

    private void updateProgressBar(long currentSize, long totalSize) {
        Logger.d(TAG, "updateProgressBar entry, with currentSize: " + currentSize);
        if (mImageSharingView == null) {
            initTransferringLayout();
        }
        if (mImageSharingView != null) {
            ProgressBar progressBar =
                    (ProgressBar) mImageSharingView.findViewById(R.id.progress_bar);
            if (progressBar != null) {
                if (currentSize != 0) {
                    double position = ((double) currentSize / (double) totalSize) * 100.0;
                    progressBar.setProgress((int) position);
                } else {
                    progressBar.setProgress(0);
                }
            } else {
                Logger.e(TAG, "updateProgressBar(), progressBar is null!");
            }
        } else {
            Logger.e(TAG, "updateProgressBar(), mImageSharingView is null!");
        }
        Logger.d(TAG, "updateProgressBar exit");
    }

    private void initTransferringLayout() {
        Logger.v(TAG, "initTransferringLayout entry");
        mImageSharingView =
                LayoutInflater.from(mContext).inflate(R.layout.richcall_image_sharing, null);
        Logger.d(TAG, "initTransferringLayout exit, mImageSharingView: " + mImageSharingView);
    }

    /**
     * Incoming video sharing session event listener
     */
    private class IncomingSessionEventListener extends IImageSharingEventListener.Stub {
        private static final String TAG = "IncomingSessionEventListener";

        // Session is started
        public void handleSessionStarted() {
            Logger.v(TAG, "handleSessionStarted() entry");
        }

        // Session has been aborted
        public void handleSessionAborted(int reason) {
            Logger.v(TAG, "handleSessionAborted()");
            mCallScreenDialogManager.showTimeOutDialog();
            finishImageSharing();
        }

        // Session has been terminated by remote
        public void handleSessionTerminatedByRemote() {
            Logger.v(TAG, "handleSessionTerminatedByRemote()");
            mCallScreenDialogManager.showTerminatedDialog();
            finishImageSharing();
        }

        // Sharing error
        public void handleSharingError(final int error) {
            Logger.v(TAG, "handleSharingError(), error = " + error);
            switch (error) {
                case ContentSharingError.UNEXPECTED_EXCEPTION:
                    mCallScreenDialogManager.showTerminatedDialog();
                    break;
                case ContentSharingError.SESSION_INITIATION_FAILED:
                    mCallScreenDialogManager.showInitFailDialog();
                    break;
                case ContentSharingError.MEDIA_SAVING_FAILED:
                    mCallScreenDialogManager.showNoStorageDialog();
                    break;
                default:
                    break;
            }
            finishImageSharing();
        }

        @Override
        public void handleImageTransfered(String arg0) throws RemoteException {
            Logger.d(TAG, "handleImageTransfered entry, file: " + arg0);
            showFullImage(arg0);
            mImageSharingStatus = ImageSharingStatus.COMPLETE;
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mCallScreenHost != null) {
                        mCallScreenHost.onStateChange(getCurrentState());
                    } else {
                        Logger.w(TAG, "handleImageTransfered mCallScreenHost is null");
                    }
                }
            });
            mIncomingImageSharingSession.removeSessionListener(mIncomingSessionEventListener);
            if (mWakeLockCount.decrementAndGet() >= 0) {
                mWakeLock.release();
            }
            Logger.d(TAG, "handleImageTransfered exit");
        }

        @Override
        public void handleSharingProgress(final long arg0, final long arg1) throws RemoteException {
            Logger.d(TAG, "handleSharingProgress entry");
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    updateProgressBar(arg0, arg1);
                }
            });
            Logger.d(TAG, "handleSharingProgress exit");
        }
    }

    private void showFullImage(final String imageName) {
        new AsyncTask<Void, Void, Bitmap>() {
            @Override
            protected Bitmap doInBackground(Void... params) {
                Logger.d(TAG, "showFullImage doInBackground entry");
                RcseImage image = new RcseImage(mContext, Uri.parse(URI_PREFIX + imageName));
                Bitmap bitmap = image.fullSizeBitmap(MAX_SIZE_PIXELS, true);
                Logger.d(TAG, "showFullImage doInBackground exit");
                return bitmap;
            }

            @Override
            protected void onPostExecute(Bitmap bitmap) {
                Logger.d(TAG, "showFullImage(), onPostExecute entry");
                if (mDisplayArea != null) {
                    if (mCallScreenHost != null) {
                        mDisplayArea.removeView(mImageSharingView);
                        mImageSharingView =
                                LayoutInflater.from(mContext).inflate(
                                        R.layout.richcall_image_full_display, mDisplayArea);
                        ImageView imageview =
                                (ImageView) mImageSharingView.findViewById(R.id.shared_image);
                        imageview.setImageBitmap(bitmap);
                    } else {
                        Logger.d(TAG, "showFullImage(), mCallScreenHost is null");
                    }
                    Logger.d(TAG, "showFullImage(), mDisplayArea display full image: " + bitmap);
                } else {
                    Logger.d(TAG, "showFullImage(), mDisplayArea is null");
                }
                mImageSharingStatus = ImageSharingStatus.COMPLETE;
                Utils.setInImageSharing(false);
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mCallScreenHost != null) {
                            mCallScreenHost.onStateChange(getCurrentState());
                        } else {
                            Logger.w(TAG, "showFullImage(), mCallScreenHost is null");
                        }
                    }
                });
                Logger.d(TAG, "showFullImage(), onPostExecute exit");
            }
        }.execute();
    }

    private void onUserAcceptImageSharing(Context context, String fileName, long fileSize) {
        Logger.i(TAG, "onUserAcceptImageSharing(), entry!");
        long maxFileSize = RcsSettings.getInstance().getMaxImageSharingSize() * 1024;
        long warningFileSize = RcsSettings.getInstance()
                .getWarningMaxImageTransferSize() * 1024;
        Logger.w(TAG, "onUserAcceptImageSharing() maxFileSize is " + maxFileSize);
        Logger.w(TAG, "onUserAcceptImageSharing() warningFileSize is " + warningFileSize);
        if (fileSize >= warningFileSize && warningFileSize != 0) {
            boolean isRemind = RcsSettings.getInstance().restoreRemindWarningLargeImageFlag();
            Logger.w(TAG, "WarningDialog onCreateDialog the remind status is " + isRemind);
            if (isRemind) {
                Activity activity = null;
                if (mCallScreenHost != null) {
                    activity = mCallScreenHost.getCallScreenActivity();
                    if (activity != null) {
                        Logger.d(TAG, "show compress dialog");
                        mReceiveWarningDialog = new WarningDialog();
                        mReceiveWarningDialog.saveParameters(fileName, entry_terminated);
                        mReceiveWarningDialog.show(activity.getFragmentManager(), WarningDialog.TAG);
                    }
                    return;
                }

            } else {
                acceptImageSharing();
            }
        } else {
            acceptImageSharing();
        }
        Logger.i(TAG, "onUserAccept(), exit!");
    }

    private void acceptImageSharing() {
        new AsyncTask<Void, Void, Void>() {

            @Override
            protected void onPreExecute() {
                Logger.d(TAG, "acceptImageSharing onPreExecute entry");
                initTransferringLayout();
                TextView fileNameView = (TextView) mImageSharingView.findViewById(R.id.file_name);
                if (mIncomingImageName != null) {
                    fileNameView.setText(mIncomingImageName);
                } else {
                    Logger.d(TAG, "acceptImageSharing mIncomingImageName is null");
                }
				ImageView thumbView = (ImageView) mImageSharingView.findViewById(R.id.shared_image);
	            if (thumbView != null) {
	                //view.setImageBitmap(bitmap);
	            if(mIncomingThumbnail != null){
					thumbView.setImageBitmap(BitmapFactory.decodeByteArray(mIncomingThumbnail,0,mIncomingThumbnail.length));
	            }
	            } else {
	                Logger.d(TAG, "ThumbnailAsyncTask view is null");
	            }
                final ImageView cancelView =
                        (ImageView) mImageSharingView.findViewById(R.id.cancel_image);
                cancelView.setOnClickListener(new OnClickListener() {

                    @Override
                    public void onClick(View v) {
                        mCallScreenDialogManager.showTerminateConfirmDialog();
                    }
                });
                TextView fileSizeView = (TextView) mImageSharingView.findViewById(R.id.file_size);
                if (mIncomingImageName != null) {
                    String value =
                            roundDouble((double) mIncomingImageSize / FILE_UNIT_SIZE, 1)
                                    + FILE_UNIT;
                    fileSizeView.setText(value);
                } else {
                    Logger.d(TAG, "acceptImageSharing fileSizeView is null");
                }
                if (mCallScreenHost != null) {
                    mDisplayArea = mCallScreenHost.requestAreaForDisplay();
                    if (mDisplayArea != null) {
                        mDisplayArea.removeAllViews();
                        mDisplayArea.addView(mImageSharingView);
                    } else {
                        Logger.d(TAG, "acceptImageSharing mDisplayArea is null");
                    }
                    mCallScreenHost.onStateChange(getCurrentState());
                } else {
                    Logger.d(TAG, "acceptImageSharing mCallScreenHost is null");
                }
                Logger.d(TAG, "acceptImageSharing onPreExecute exit");
            }

            @Override
            protected Void doInBackground(Void... params) {
                Logger.d(TAG, "acceptImageSharing doInBackground entry, mRichCallStatus: "
                        + mRichCallStatus);
                if (mRichCallStatus == RichCallStatus.CONNECTED) {
                    mImageSharingStatus = ImageSharingStatus.INCOMING;
                    acceptImageSharingSession();
                } else {
                    mImageSharingStatus = ImageSharingStatus.INCOMING_BLOCKING;
                    mPendingActions.add(new Runnable() {
                        @Override
                        public void run() {
                            acceptImageSharingSession();
                        }
                    });
                }
                Logger.d(TAG, "acceptImageSharing doInBackground exit");
                return null;
            }
        }.execute();
    }

    private double roundDouble(double val, int precision) {
        double result = 0;
        double factor = Math.pow(10, precision);
        if (factor != 0) {
            result = Math.floor(val * factor + 0.5) / factor;
        } else {
            Logger.d(TAG, "roundDouble factor is 0");
        }
        return result;
    }

    private void declineImageSharing() {
        Logger.v(TAG, "declineImageSharing() entry");
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                boolean success = false;
                try {
                    Logger.d(TAG, "addIncomingImageSharingListener mRichCallStatus is: "
                            + mRichCallStatus);
                    if (mRichCallStatus == RichCallStatus.CONNECTED) {
                        mImageSharingStatus = ImageSharingStatus.DECLINE;
                        mIncomingImageSharingSession = mRichCallApi
                                .getImageSharingSession(mIncomingSessionId);
                        Logger.w(TAG, "declineImageSharing mIncomingImageSharingSession: "
                                + mIncomingImageSharingSession);
                        if (mIncomingImageSharingSession != null) {
                            mIncomingImageSharingSession
                                    .removeSessionListener(mIncomingSessionEventListener);
                            mIncomingImageSharingSession.rejectSession();
                            success = true;
                        }
                    } else {
                        mPendingActions.add(new Runnable() {
                            @Override
                            public void run() {
                                declineImageSharing();
                            }
                        });
                    }
                } catch (ClientApiException e) {
                    e.printStackTrace();
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
                if (!success) {
                    reset();
                }
                Logger.d(TAG, "declineImageSharing() success: " + success);
            }
        });
        Logger.v(TAG, "declineImageSharing() exit");
    }

    /**
     * Image sharing invitation receiver.
     */
    private class ImageSharingInvitationReceiver extends BroadcastReceiver {
        private static final String TAG = "ImageSharingInvitationReceiver";

        @Override
        public void onReceive(Context context, Intent intent) {
            Logger.v(TAG, "onReceive entry, with intent: " + intent);
            if (intent != null) {
                String action = intent.getAction();
                if (IMAGE_SHARING_INVITATION_ACTION.equals(action)) {
                    handleImageSharingInvitation(context, intent);
                } else if (IMAGE_SHARING_START_ACTION.equals(action)) {
                    mOutGoingImageName = intent.getStringExtra(IMAGE_NAME);
                    Logger.v(TAG, "onReceive mOutGoingImageName is: " + mOutGoingImageName);
                    if (mCallScreenHost != null) {
                        Utils.setInImageSharing(true);
                        prepareStartImageSharing(mOutGoingImageName);
                    } else {
                        Logger.d(TAG, "onReceive mCallScreenHost is null");
                    }
                }else if(IMAGE_SHARING_REPIC_ACTION.equals(action)) {
                    int select_type = intent.getIntExtra(SELECT_TYPE, RichcallProxyActivity.REQUEST_CODE_GALLERY);  
                    Activity activity = null;
                    if (mCallScreenHost != null) {
                        activity = mCallScreenHost.getCallScreenActivity();
                        if (activity != null) {
                            Logger.d(TAG, "show Repic dialog");
                            mRepickDialog = new RepickDialog();
                            mRepickDialog.saveSelection(select_type);
                            mRepickDialog.show(activity.getFragmentManager(), RepickDialog.TAG);
                        }
                        return;
                    }
                }else if (IMAGE_SHARING_WARN_ACTION.equals(action)) {
                    String fileName = intent.getStringExtra(IMAGE_NAME);
                    if(fileName == null){
                        return;
                    }
                    Activity activity = null;
                    if (mCallScreenHost != null) {
                        activity = mCallScreenHost.getCallScreenActivity();
                        if (activity != null) {
                            Logger.d(TAG, "show Warnings dialog");
                            mInviteWarningDialog = new WarningDialog();
                            mInviteWarningDialog.saveParameters(fileName, entry_initiate);
                            mInviteWarningDialog.show(activity.getFragmentManager(), WarningDialog.TAG);
                        }
                        return;
                    }
                } else {
                    Logger.w(TAG, "onReceive unknown action");
                }
            } else {
                Logger.w(TAG, "onReceive intent is null");
            }
            Logger.v(TAG, "onReceive exit");
        }

        private void handleImageSharingInvitation(Context context, Intent intent) {
            Logger.v(TAG, "handleImageSharingInvitation entry");
            AndroidFactory.setApplicationContext(context);
            mIncomingContact = intent.getStringExtra(RichcallProxyActivity.CONTACT);
            mIncomingContactDisplayname =
                    intent.getStringExtra(RichcallProxyActivity.CONTACT_DISPLAYNAME);
            mIncomingSessionId = intent.getStringExtra(RichcallProxyActivity.SESSION_ID);
            mIncomingImageName = intent.getStringExtra(RichcallProxyActivity.IMAGE_NAME);
            mIncomingImageSize = intent.getLongExtra(RichcallProxyActivity.IMAGE_SIZE, 0);
			mIncomingThumbnail = intent.getByteArrayExtra(RichcallProxyActivity.THUMBNAIL_TYPE);
				
            long availabeSize = com.mediatek.rcse.service.Utils.getFreeStorageSize();
            Logger.v(TAG, "handleImageSharingInvitation mIncomingContact: " + mIncomingContact
                    + " mIncomingImageName: " + mIncomingImageName + " mIncomingImageSize: "
                    + mIncomingImageSize + " mIncomingContactDisplayname: "
                    + mIncomingContactDisplayname + " availabeSize: " + availabeSize);
            boolean noStorage = false;
            if (mIncomingImageSize > availabeSize) {
                noStorage = true;
                final String toastText;
                if (availabeSize == -1) {
                    toastText = context
                            .getString(R.string.rcse_no_external_storage_for_image_share);
                } else {
                    toastText = context.getString(R.string.rcse_no_enough_storage_for_image_share);
                }
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(mContext, toastText, Toast.LENGTH_LONG).show();
                    }
                });
            }
            if (noStorage || !getCapability(mNumber)) {
                try {
                    IImageSharingSession sharingSession = mRichCallApi
                            .getImageSharingSession(mIncomingSessionId);
                    if (sharingSession != null) {
                        try {
                            sharingSession.rejectSession();
                        } catch (RemoteException e) {
                            e.printStackTrace();
                        }
                    }
                } catch (ClientApiException e) {
                    e.printStackTrace();
                }
                return;
            }
            mIncomingSessionEventListener = new IncomingSessionEventListener();
            mCallScreenDialogManager.showInvitationConfirmDialog();
            Logger.v(TAG, "handleImageSharingInvitation exit");
        }
    }

    private void addIncomingImageSharingListener() {
        Logger.d(TAG, "addIncomingImageSharingListener() entry, mRichCallStatus: "
                + mRichCallStatus);
        boolean success = false;
        try {
            if (mRichCallStatus == RichCallStatus.CONNECTED) {
                mImageSharingStatus = ImageSharingStatus.INCOMING;
                mIncomingImageSharingSession = mRichCallApi
                        .getImageSharingSession(mIncomingSessionId);
                Logger.w(TAG, "addIncomingImageSharingListener() mIncomingImageSharingSession: "
                        + mIncomingImageSharingSession);
                if (mIncomingImageSharingSession != null) {
                    mIncomingImageSharingSession.addSessionListener(mIncomingSessionEventListener);
                    success = true;
                }
            } else {
                mImageSharingStatus = ImageSharingStatus.INCOMING_BLOCKING;
                mPendingActions.add(new Runnable() {
                    @Override
                    public void run() {
                        addIncomingImageSharingListener();
                    }
                });
            }
        } catch (ClientApiException e) {
            e.printStackTrace();
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        if (!success) {
            reset();
        }
        Logger.d(TAG, "addIncomingImageSharingListener() exit, success: " + success);
    }

    private void acceptImageSharingSession() { 
        Logger.d(TAG, "acceptImageSharingSession() entry, mIncomingImageSharingSession is "
                + (mIncomingImageSharingSession != null ? mIncomingImageSharingSession.toString()
                        : "null"));
        boolean success = false;
        try {
            if (mIncomingImageSharingSession != null) {
                long receivedFileSize = mIncomingImageSharingSession.getFilesize();
                long currentStorageSize = Utils.getFreeStorageSize();
                Logger.d(TAG, "receivedFileSize = " + receivedFileSize + "/currentStorageSize = "
                        + currentStorageSize);
                if (currentStorageSize > 0) {
                    if (receivedFileSize <= currentStorageSize) {
                        mIncomingImageSharingSession.acceptSession();
                        Utils.setInImageSharing(true);
                        success = true;
                    } else {
                        mIncomingImageSharingSession.rejectSession();
                        Utils.setInImageSharing(false);
                        success = false;
                        new Handler(Looper.getMainLooper()).post(new Runnable() {
                            @Override
                            public void run() {
                                Context context = ApiManager.getInstance().getContext();
                                String strToast = context
                                        .getString(R.string.rcse_no_enough_storage_for_image_share);
                                Toast.makeText(context, strToast, Toast.LENGTH_LONG).show();
                            }
                        });
                    }
                } else {
                    mIncomingImageSharingSession.rejectSession();
                    Utils.setInImageSharing(false);
                    success = false;
                    new Handler(Looper.getMainLooper()).post(new Runnable() {
                        @Override
                        public void run() {
                            Context context = ApiManager.getInstance().getContext();
                            String strToast = context.getString(R.string.rcse_no_external_storage_for_image_share);
                            Toast.makeText(context, strToast, Toast.LENGTH_LONG).show();
                        }
                    });
                }
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        if (!success) {
            reset();
        }
        Logger.d(TAG, "acceptImageSharingSession() exit, success: " + success);
    }

    private void showAcceptedToast() {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                Logger.v(TAG, "showAcceptedToast entry");
                String message = mContext.getResources().getString(R.string.image_sharing_accepted);
                showToast(message);
                Logger.v(TAG, "showAcceptedToast exit");
            }
        });
    }

    private boolean alreadyOnGoing() {
        Logger.v(TAG, "alreadyOnGoing entry");
		if (Utils.isInVideoSharing()  && RCSeInCallUIExtension.getInstance().getVideoSharePlugIn().getStatus() == Constants.LIVE_OUT) {
		Logger.v(TAG, "alreadyOnGoing isInImageSharing true");
		    mCallScreenDialogManager.alreadyOnGoingVideoShare();
			return false;
		} else if (Utils.isInVideoSharing() && RCSeInCallUIExtension.getInstance().getVideoSharePlugIn().getStatus() == Constants.LIVE_IN) {
			Logger.v(TAG, "alreadyOnGoing isInImageSharing Incmoing file true");
			return false;
			//mVideoFinished = false;
	        //startVideoShare(true);
		} else {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                String message =
                        mContext.getResources().getString(R.string.image_sharing_already_ongoing);
                showToast(message);
            }
        });
			}
        Logger.v(TAG, "alreadyOnGoing exit");
		return true;
    }

    @Override
    public int getCurrentState() {
        Logger.d(TAG, "getCurrentState entry");
        int state = Constants.SHARE_FILE_STATE_IDLE;
        if (mImageSharingStatus == ImageSharingStatus.UNKNOWN
                || mImageSharingStatus == ImageSharingStatus.DECLINE) {
            state = Constants.SHARE_FILE_STATE_IDLE;
        } else if (mImageSharingStatus == ImageSharingStatus.COMPLETE) {
            state = Constants.SHARE_FILE_STATE_DISPLAYING;
        } else if(mImageSharingStatus == ImageSharingStatus.INCOMING) {
            state = Constants.SHARE_FILE_INCOMING;
        }
        else if(mImageSharingStatus == ImageSharingStatus.OUTGOING) {
            state = Constants.SHARE_FILE_OUTGOING;
        }
        Logger.d(TAG, "getCurrentState exit with state: " + state);
        return state;
    }

    private final class CallScreenDialogManager {
        private static final String TAG = "CallScreenDialogManager";
        private static final int CAMERA_POSITION = 0;
        private static final int GALLERY_POSITION = 1;
        private static final int CHAT_POSITION = 2;
        private CopyOnWriteArraySet<CallScreenDialog> mDialogs = new CopyOnWriteArraySet<CallScreenDialog>();
        private List<CallScreenDialog> mSavedDialogs = new ArrayList<CallScreenDialog>();
        private View view = null;
        
        public void saveAlertDialogs() {
            Logger.d(TAG, "saveAlertDialogs entry");
            mSavedDialogs.clear();
            mSavedDialogs.addAll(mDialogs);
            Logger.d(TAG, "saveAlertDialogs exit" + mSavedDialogs );
        }
        
        public void showAlertDialogs() {
            Logger.d(TAG, "showAlertDialogs entry");
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {                    
                    if (view != null) {
                        ViewGroup parentViewGroup = (ViewGroup) view.getParent();
                        if (parentViewGroup != null) {
                            Logger.d(TAG, "Parent View Group not null");
                            parentViewGroup.removeAllViews();
                        }
                    }
                    if(mSavedDialogs.size() > 1){
                        mDialogs.add(mSavedDialogs.get(mSavedDialogs.size() - 1));
                        mSavedDialogs.get(mSavedDialogs.size() - 1).show();
                    }
                    else if(mSavedDialogs.size() == 1){
                mDialogs.add(mSavedDialogs.get(0));
                        mSavedDialogs.get(0).show();
                    }
                    mSavedDialogs.clear();
                }
            });
            Logger.d(TAG, "showAlertDialogs exit");
        }
        
        public void clearSavedDialogs(){
            Logger.v(TAG, "clearSavedDialogs() entry mSavedDialogs size is "+mSavedDialogs.size());
            mSavedDialogs.clear();
            Logger.v(TAG, "clearSavedDialogs() entry mSavedDialogs size is "+mSavedDialogs.size());
        }

        public void dismissOtherDialog() {
            Logger.i(TAG, "dismissOtherDialog entry()");
            if(mCompressDialog != null)
            {
                mCompressDialog.dismiss();
            }
            if(mReceiveWarningDialog != null)
            {
                mReceiveWarningDialog.dismiss();
            }
            if(mInviteWarningDialog != null)
            {
                mInviteWarningDialog.dismiss();
            }
            if(mRepickDialog != null)
            {
                mRepickDialog.dismiss();
            }
            mCompressDialog = null;
            mReceiveWarningDialog = null;
            mInviteWarningDialog = null;
            mRepickDialog = null; 
            Logger.i(TAG, "dismissOtherDialog exit()");
        }

		 public void alreadyOnGoingVideoShare() {
            Logger.v(TAG, "alreadyOnGoingVideoShare entry");      
                         
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    String message =  mContext.getResources().getString(R.string.video_share_ongoing_image_start);                
                    Logger.v(TAG, "alreadyOnGoingVideoShare msg = " + message);
                    //createAndShowAlertDialog(message,mAlreadyOnGoingShareListener );
                    showTerminateConfirmDialog(message); 
                }
            });
            Logger.v(TAG, "alreadyOnGoing exit");
        }
        
        DialogInterface.OnClickListener mAlreadyOnGoingShareListener = new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Logger.v(TAG, "onClick(), which = " + which);
                switch (which) {
                case DialogInterface.BUTTON_POSITIVE:
                    Logger.v(TAG, "stop image share first");
                    RCSeInCallUIExtension.getInstance().getmShareFilePlugIn().stop();
                   // mVideoFinished = false;
                   // startVideoShare(true);
                    break;
                case DialogInterface.BUTTON_NEGATIVE:
                    Logger.v(TAG, "Do nothing");
                    break;
                default:
                    Logger.v(TAG, "Unknown option");
                    break;
                }
                dismissDialog();
                clearSavedDialogs();
            }
        };

        public void showInvitationConfirmDialog() {
            Logger.d(TAG, "showInvitationConfirmDialog entry");
            addIncomingImageSharingListener();
            if (mCallScreenHost == null) {
                Logger.d(TAG, "showInvitationConfirmDialog mCallScreenHost is null");
                return;
            }
            Activity activity = mCallScreenHost.getCallScreenActivity();
            if (activity == null) {
                Logger.d(TAG, "showInvitationConfirmDialog getCallScreenActivity is null");
                return;
            }
            final CallScreenDialog callScreenDialog = new CallScreenDialog(activity);

            String filesize = com.mediatek.rcse.service.Utils.formatFileSizeToString(mContext,
                    mIncomingImageSize, com.mediatek.rcse.service.Utils.SIZE_TYPE_TOTAL_SIZE);
            view = LayoutInflater.from(mContext).inflate(R.layout.image_invitation_content,
                    null);
            TextView imageName = (TextView) view.findViewById(R.id.image_name);
            TextView imageSize = (TextView) view.findViewById(R.id.image_size);
            ImageView imageType = (ImageView) view.findViewById(R.id.image_type);
            if (mIncomingImageName == null) {
                Logger.d(TAG, "showInvitationConfirmDialog mIncomingImageName is null");
                return;
            }
            String mimeType = MediaFile.getMimeTypeForFile(mIncomingImageName);
            if (mimeType == null) {
                mimeType = MimeTypeMap.getSingleton().getMimeTypeFromExtension(
                        com.mediatek.rcse.service.Utils.getFileExtension(mIncomingImageName));
            }
            Intent intent = new Intent(Intent.ACTION_VIEW);
            intent.setDataAndType(
                    com.mediatek.rcse.service.Utils.getFileNameUri(mIncomingImageName), mimeType);
            PackageManager packageManager = mContext.getPackageManager();
            List<ResolveInfo> list = packageManager.queryIntentActivities(intent,
                    PackageManager.MATCH_DEFAULT_ONLY);
			if(mIncomingThumbnail == null){
            int size = list.size();
            if (size > 0) {
                imageType.setImageDrawable(list.get(0).activityInfo.loadIcon(packageManager));
            }
			}
			else {
				imageType.setImageBitmap(BitmapFactory.decodeByteArray(mIncomingThumbnail,0,mIncomingThumbnail.length));
			}
            imageName.setTextColor(COLOR);
            imageName.setText(mIncomingImageName);
            imageSize.setText(filesize);
            imageSize.setTextColor(COLOR);
            callScreenDialog.setContent(view);
            String contactName = (mIncomingContactDisplayname != null) ? mIncomingContactDisplayname
                    : mIncomingContact;
            callScreenDialog.setTitle(mContext.getString(R.string.file_type_image)
                    + mContext.getString(R.string.file_transfer_from) + contactName);

            if (mIncomingImageSize < mMaxImageSharingSize || mMaxImageSharingSize == 0) {
                callScreenDialog.setPositiveButton(mContext
                        .getString(R.string.rcs_dialog_positive_button),
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                callScreenDialog.dismissDialog();
                                mDialogs.remove(callScreenDialog);
                                clearSavedDialogs();
                                onUserAcceptImageSharing(mContext,mIncomingImageName ,mIncomingImageSize);
                            }
                        });
                callScreenDialog.setNegativeButton(mContext
                        .getString(R.string.rcs_dialog_negative_button),
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                callScreenDialog.dismissDialog();
                                mDialogs.remove(callScreenDialog);
                                clearSavedDialogs();
                                declineImageSharing();
                            }
                        });
            } else {
                declineImageSharing();
                String notifymessage =
                        mContext.getString(R.string.file_size_notification,
                                contactName);
                callScreenDialog.setMessage(notifymessage);
                callScreenDialog.setPositiveButton(mContext
                        .getString(R.string.rcs_dialog_positive_button),
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                callScreenDialog.dismissDialog();
                                mDialogs.remove(callScreenDialog);
                                clearSavedDialogs();
                            }
                        });
            }
            //Vibrate
            Utils.vibrate(mContext, Utils.MIN_VIBRATING_TIME);
            callScreenDialog.setCancelable(false);
            clearSavedDialogs();
            callScreenDialog.show();
            mDialogs.add(callScreenDialog);
            saveAlertDialogs();
            Logger.d(TAG, "showInvitationConfirmDialog exit");
        }

        public void showTimeOutDialog() {
            Logger.d(TAG, "showTimeOutDialog entry");
            dismissDialogs();
            clearSavedDialogs();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    Activity activity = null;
                    if (mCallScreenHost != null && mCallScreenHost.getCallScreenActivity() != null) {
                        activity = mCallScreenHost.getCallScreenActivity();
                        final CallScreenDialog callScreenDialog = new CallScreenDialog(activity);
                        callScreenDialog.setIcon(android.R.attr.alertDialogIcon);
                        callScreenDialog.setTitle(mContext.getString(R.string.attention_title));
                        callScreenDialog.setMessage(mContext
                                .getString(R.string.image_sharing_invitation_time_out));
                        callScreenDialog.setPositiveButton(mContext
                                .getString(R.string.rcs_dialog_positive_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        callScreenDialog.dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setCancelListener(
                                new DialogInterface.OnCancelListener() {
                                    @Override
                                    public void onCancel(DialogInterface dialog) {
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.show();
                        mDialogs.add(callScreenDialog);
                        saveAlertDialogs();
                    } else {
                        Logger.d(TAG,
                                "showTimeOutDialog mCallScreenHost is null or getCallScreenActivity is null, "
                                        + mCallScreenHost);
                    }
                }
            });
            Logger.d(TAG, "showTimeOutDialog exit");
        }

        public void showInitFailDialog() {
            Logger.d(TAG, "showInitFailDialog entry");
            dismissDialogs();
            clearSavedDialogs();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mCallScreenHost != null && mCallScreenHost.getCallScreenActivity() != null) {
                        Activity activity = mCallScreenHost.getCallScreenActivity();
                        final CallScreenDialog callScreenDialog = new CallScreenDialog(activity);
                        callScreenDialog.setIcon(android.R.attr.alertDialogIcon);
                        callScreenDialog.setTitle(mContext.getString(R.string.attention_title));
                        callScreenDialog.setMessage(mContext
                                .getString(R.string.image_sharing_terminated_due_to_network));
                        callScreenDialog.setPositiveButton(mContext
                                .getString(R.string.rcs_dialog_positive_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        callScreenDialog.dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setCancelListener(
                                new DialogInterface.OnCancelListener() {
                                    @Override
                                    public void onCancel(DialogInterface dialog) {
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.show();
                        mDialogs.add(callScreenDialog);
                        saveAlertDialogs();
                    } else {
                        Logger.d(TAG,
                                "showInitFailDialog mCallScreenHost is null or getCallScreenActivity is null, "
                                        + mCallScreenHost);
                    }
                }
            });
            Logger.d(TAG, "showInitFailDialog exit");
        }

        public void showRejectedDialog() {
            Logger.d(TAG, "showRejectedDialog entry");
            dismissDialogs();
            clearSavedDialogs();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mCallScreenHost != null && mCallScreenHost.getCallScreenActivity() != null) {
                        Activity activity = mCallScreenHost.getCallScreenActivity();
                        final CallScreenDialog callScreenDialog = new CallScreenDialog(activity);
                        callScreenDialog.setIcon(android.R.attr.alertDialogIcon);
                        callScreenDialog.setTitle(mContext.getString(R.string.attention_title));
                        callScreenDialog.setMessage(mContext
                                .getString(R.string.rejected_share_image));
                        callScreenDialog.setPositiveButton(mContext
                                .getString(R.string.rcs_dialog_positive_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        callScreenDialog.dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setCancelListener(
                                new DialogInterface.OnCancelListener() {
                                    @Override
                                    public void onCancel(DialogInterface dialog) {
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.show();
                        mDialogs.add(callScreenDialog);
                        saveAlertDialogs();
                    } else {
                        Logger.d(TAG,
                                "showRejectedDialog mCallScreenHost is null or getCallScreenActivity is null, "
                                        + mCallScreenHost);
                    }
                }
            });
            Logger.d(TAG, "showRejectedDialog exit");
        }

        public void showTerminatedDialog() {
            Logger.d(TAG, "showTerminatedDialog entry");
            dismissDialogs();
            clearSavedDialogs();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mCallScreenHost != null && mCallScreenHost.getCallScreenActivity() != null) {
                        Activity activity = mCallScreenHost.getCallScreenActivity();
                        final CallScreenDialog callScreenDialog = new CallScreenDialog(activity);
                        callScreenDialog.setIcon(android.R.attr.alertDialogIcon);
                        callScreenDialog.setPositiveButton(mContext
                                .getString(R.string.rcs_dialog_positive_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        callScreenDialog.dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setCancelListener(
                                new DialogInterface.OnCancelListener() {
                                    @Override
                                    public void onCancel(DialogInterface dialog) {
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setTitle(mContext.getString(R.string.attention_title));
                        callScreenDialog.setMessage(mContext
                                .getString(R.string.terminated_share_image));
                        callScreenDialog.show();
                        mDialogs.add(callScreenDialog);
                        saveAlertDialogs();
                    } else {
                        Logger.d(TAG,
                                "showTerminatedDialog mCallScreenHost is null or getCallScreenActivity is null, "
                                        + mCallScreenHost);
                    }
                }
            });
            Logger.d(TAG, "showTerminatedDialog exit");
        }

        public void showTerminateConfirmDialog() {
            Logger.d(TAG, "showTerminateConfirmDialog entry");
            dismissDialogs();
            clearSavedDialogs();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mCallScreenHost != null && mCallScreenHost.getCallScreenActivity() != null) {
                        Activity activity = mCallScreenHost.getCallScreenActivity();
                        final CallScreenDialog callScreenDialog = new CallScreenDialog(activity);
                        callScreenDialog.setIcon(android.R.attr.alertDialogIcon);
                        callScreenDialog.setPositiveButton(mContext
                                .getString(R.string.rcs_dialog_positive_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        Logger.d(TAG, "showTerminateConfirmDialog onClick");
                                        finishImageSharing();
                                        callScreenDialog.dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setNegativeButton(mContext
                                .getString(R.string.rcs_dialog_negative_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        callScreenDialog.dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setCancelListener(
                                new DialogInterface.OnCancelListener() {
                                    @Override
                                    public void onCancel(DialogInterface dialog) {
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setTitle(mContext.getString(R.string.attention_title));
                        callScreenDialog.setMessage(mContext
                                .getString(R.string.terminate_share_image));
                        callScreenDialog.show();
                        mDialogs.add(callScreenDialog);
                        saveAlertDialogs();
                    } else {
                        Logger.d(TAG,
                                "showTerminateConfirmDialog mCallScreenHost is null or getCallScreenActivity is null, "
                                        + mCallScreenHost);
                    }
                }
            });
            Logger.d(TAG, "showTerminateConfirmDialog exit");
        }


		public void showTerminateConfirmDialog(final String msg) {
            Logger.d(TAG, "showTerminateConfirmDialog entry");
            dismissDialogs();
            clearSavedDialogs();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mCallScreenHost != null && mCallScreenHost.getCallScreenActivity() != null) {
                        Activity activity = mCallScreenHost.getCallScreenActivity();
                        final CallScreenDialog callScreenDialog = new CallScreenDialog(activity);
                        callScreenDialog.setIcon(android.R.attr.alertDialogIcon);
                        callScreenDialog.setPositiveButton(mContext
                                .getString(R.string.rcs_dialog_positive_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        Logger.v(TAG, "stop video share first");
                                        RCSeInCallUIExtension.getInstance().getVideoSharePlugIn().stop();
                                        callScreenDialog.dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setNegativeButton(mContext
                                .getString(R.string.rcs_dialog_negative_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        callScreenDialog.dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setCancelListener(
                                new DialogInterface.OnCancelListener() {
                                    @Override
                                    public void onCancel(DialogInterface dialog) {
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setTitle(mContext.getString(R.string.attention_title));
                        //callScreenDialog.setMessage(mContext
                        //        .getString(R.string.terminate_share_image));
                        callScreenDialog.setMessage(msg);
                        callScreenDialog.show();
                        mDialogs.add(callScreenDialog);
                        saveAlertDialogs();
                    } else {
                        Logger.d(TAG,
                                "showTerminateConfirmDialog mCallScreenHost is null or getCallScreenActivity is null, "
                                        + mCallScreenHost);
                    }
                }
            });
            Logger.d(TAG, "showTerminateConfirmDialog exit");
        }

        public void showNoStorageDialog() {
            Logger.d(TAG, "showNoStorageDialog entry");
            dismissDialogs();
            clearSavedDialogs();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mCallScreenHost != null && mCallScreenHost.getCallScreenActivity() != null) {
                        Activity activity = mCallScreenHost.getCallScreenActivity();
                        final CallScreenDialog callScreenDialog = new CallScreenDialog(activity);
                        callScreenDialog.setIcon(android.R.attr.alertDialogIcon);
                        callScreenDialog.setPositiveButton(mContext
                                .getString(R.string.rcs_dialog_positive_button),
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        callScreenDialog.dismissDialog();
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setCancelListener(
                                new DialogInterface.OnCancelListener() {
                                    @Override
                                    public void onCancel(DialogInterface dialog) {
                                        mDialogs.remove(callScreenDialog);
                                        clearSavedDialogs();
                                    }
                                });
                        callScreenDialog.setTitle(mContext.getString(R.string.attention_title));
                        callScreenDialog.setMessage(mContext.getString(R.string.no_storage));
                        callScreenDialog.show();
                        mDialogs.add(callScreenDialog);
                        saveAlertDialogs();
                    } else {
                        Logger.d(TAG,
                                "showNoStorageDialog mCallScreenHost is null or getCallScreenActivity is null, "
                                        + mCallScreenHost);
                    }
                }
            });
            Logger.d(TAG, "showNoStorageDialog exit");
        }

        public void dismissDialogs() {
            Logger.d(TAG, "dismissDialogs entry");
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    for (CallScreenDialog dialog : mDialogs) {
                        dialog.dismissDialog();
                    }
                    mDialogs.clear();
                }
            });
            Logger.d(TAG, "dismissDialogs exit");
        }

        private class SelectionItem {
            public String text;
        }

        public void showSelectImageDialog() {
            final List<SelectionItem> list = new ArrayList<SelectionItem>();
            SelectionItem cameraItem = new SelectionItem();
            cameraItem.text = mContext.getString(R.string.camera_item);
            list.add(cameraItem);
            SelectionItem galleryItem = new SelectionItem();
            galleryItem.text = mContext.getString(R.string.gallery_item);
            list.add(galleryItem);
            SelectionItem joynChatItem = new SelectionItem();
            joynChatItem.text = mContext.getString(R.string.startchat);
            list.add(joynChatItem);
            if (mCallScreenHost != null) {
                final Context context = mCallScreenHost.getCallScreenActivity();
                if (context == null) {
                    Logger.d(TAG, "showSelectImageDialog getCallScreenActivity is null");
                    return;
                }
                final LayoutInflater dialogInflater =
                        (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                final ArrayAdapter<SelectionItem> adapter =
                        new ArrayAdapter<SelectionItem>(context, R.layout.image_selection_layout,
                                list) {
                            @Override
                            public View getView(int position, final View convertView,
                                    ViewGroup parent) {
                                View itemView = convertView;
                                if (itemView == null) {
                                    itemView =
                                            dialogInflater.inflate(mContext.getResources()
                                                    .getLayout(R.layout.image_selection_layout),
                                                    parent, false);
                                }
                                final TextView text =
                                        (TextView) itemView.findViewById(R.id.item_text);
                                SelectionItem item = getItem(position);
                                text.setText(item.text);
                                return itemView;
                            }
                        };
                final CallScreenDialog selectionDialog = new CallScreenDialog(context);
                selectionDialog.setTitle(mContext.getString(R.string.share_image));
                selectionDialog.setSingleChoiceItems(adapter, 0,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                        mDialogs.remove(selectionDialog);
                        clearSavedDialogs();
                                if (CAMERA_POSITION == which) {
                                    startCamera();
                                } else if (GALLERY_POSITION == which) {
                                    startGallery();
                                }else if (CHAT_POSITION == which) {
                                	startJoynChat();
                                }
                            }
                        });
                selectionDialog.setCancelable(true);
                selectionDialog.setCancelListener(
                        new DialogInterface.OnCancelListener() {
                            @Override
                            public void onCancel(DialogInterface dialog) {
                                mDialogs.remove(selectionDialog);
                                clearSavedDialogs();
                            }
                        });
                for(CallScreenDialog callDialog : mDialogs)
                {
                    callDialog.dismissDialog();
                    mDialogs.remove(callDialog);
                }
                clearSavedDialogs();
                selectionDialog.show();
                mDialogs.add(selectionDialog);
                saveAlertDialogs();
            } else {
                Logger.d(TAG, "showSelectImageDialog mCallScreenHost is null");
            }
        }
    }

    private void startCamera() {
        Intent intent = new Intent(RichcallProxyActivity.IMAGE_SHARING_SELECTION);
        intent
                .putExtra(RichcallProxyActivity.SELECT_TYPE,
                        RichcallProxyActivity.SELECT_TYPE_CAMERA);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        mContext.startActivity(intent);
    }

    private void startGallery() {
        Intent intent = new Intent(RichcallProxyActivity.IMAGE_SHARING_SELECTION);
        intent.putExtra(RichcallProxyActivity.SELECT_TYPE,
                RichcallProxyActivity.SELECT_TYPE_GALLERY);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        mContext.startActivity(intent);
    }

    private void startJoynChat() {
    	Intent imIntent = null;
    	imIntent = new Intent(PluginApiManager.RcseAction.PROXY_ACTION);
    	imIntent.putExtra(PluginApiManager.RcseAction.IM_ACTION, true);
    	imIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        imIntent.putExtra(PluginApiManager.RcseAction.CONTACT_NUMBER, mNumber);
        imIntent.putExtra("isjoyn", true);
        imIntent.putExtra(PluginApiManager.RcseAction.CONTACT_NAME, PluginUtils.getNameByNumber(mNumber));
    	 mContext.startActivity(imIntent);
    }

    @Override
    public int getState() {
        Logger.d(TAG, "getState entry");
        int state = Constants.SHARE_FILE_STATE_IDLE;
        if (mImageSharingStatus == ImageSharingStatus.UNKNOWN
                || mImageSharingStatus == ImageSharingStatus.DECLINE) {
            state = Constants.SHARE_FILE_STATE_IDLE;
        } else if (mImageSharingStatus == ImageSharingStatus.COMPLETE) {
            state = Constants.SHARE_FILE_STATE_DISPLAYING;
        } else {
            state = Constants.SHARE_FILE_STATE_TRANSFERING;
        }
        Logger.d(TAG, "getState exit with state: " + state);
        return state;
    }

    @Override
    public int getStatus() {
		 return 0 ;  //not to be used
	}

    @Override
    public void stop() {
        Logger.v(TAG, "stop() entry");
        finishImageSharing();
        if (mCallScreenDialogManager != null) {
            mCallScreenDialogManager.dismissDialogs();
            mCallScreenDialogManager.dismissOtherDialog();
        } else {
            Logger.d(TAG, "dismissDialog mCallScreenDialogManager is null");
        }
        Logger.v(TAG, "stop() exit");
    }

    @Override
    public void start(String number) {
        super.start(number);
        Logger.d(TAG, "start() entry, number: " + number + " mImageSharingStatus: "
                + mImageSharingStatus);
        boolean inImageShare = mImageSharingStatus != ImageSharingStatus.UNKNOWN
                && mImageSharingStatus != ImageSharingStatus.COMPLETE
                && mImageSharingStatus != ImageSharingStatus.DECLINE;
        if (Utils.isInVideoSharing() || inImageShare) {
            if(alreadyOnGoing());
            return;
        }
        if (!isImageShareSupported(number)) {
            imageShareNotSupported();
            return;
        }
        mCallScreenDialogManager.showSelectImageDialog();
    }

    public void saveAlertDialogs()
    {
        Logger.v(TAG, "saveAlertDialogs() entry");
        mCallScreenDialogManager.saveAlertDialogs();
        Logger.v(TAG, "saveAlertDialogs() exit");
    }
    
    public void showAlertDialogs()
    {
        Logger.v(TAG, "showAlertDialogs() entry");
        mCallScreenDialogManager.showAlertDialogs();
        Logger.v(TAG, "showAlertDialogs() exit");
    }
    
    public boolean dismissDialog() {
        Logger.v(TAG, "dismissDialog()");
        if (mCallScreenDialogManager != null) {
            mCallScreenDialogManager.dismissDialogs();
        }
        return false;
    }

    private void prepareStartImageSharing(final String imageName) {
        Logger.d(TAG, "prepareStartImageSharing()");
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                String compressedFileName = null;
                Logger.d(TAG, "Need to compress");
                if (RcsSettings.getInstance().isEnabledCompressingImageFromDB()) {
                    Logger.d(TAG, "Compress the image, do not hit the user");
                    compressedFileName = com.mediatek.rcse.service.Utils.compressImage(imageName);
                    Logger.d(TAG, "The compressed image file name = " + compressedFileName);
                    final String nameString = compressedFileName;
                    mMainHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            startImageSharing(nameString);
                        }
                    });
                } else {
                    boolean remind = RcsSettings.getInstance().restoreRemindCompressFlag();
                    Logger.d(TAG, "Do hit the user to select whether to compress. remind = "
                            + remind);
                    if (remind) {
                        mCompressDialog = new CompressDialog();
                        mCompressDialog.setOrigFileName(imageName);
                        Activity activity = null;
                        if (mCallScreenHost != null) {
                            activity = mCallScreenHost.getCallScreenActivity();
                            if (activity != null) {
                                Logger.d(TAG, "show compress dialog");
                                mCompressDialog.show(activity.getFragmentManager(),
                                        CompressDialog.TAG);
                            }
                            return;
                        }
                    } else {
                        Logger.d(TAG, "Do not compress image");
                        compressedFileName = imageName;
                        Logger.d(TAG, "The compressed image file name = " + compressedFileName);
                        final String nameString = compressedFileName;
                        mMainHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                startImageSharing(nameString);
                            }
                        });
                    }
                }
            }
        });
    }

    /**
     * A dialog to hint the user that a picture will be compressed before
     * sending.
     */
    public class CompressDialog extends DialogFragment implements DialogInterface.OnClickListener, DialogInterface.OnCancelListener{
        private static final String TAG = "CompressDialog";
        private CheckBox mCheckNotRemind = null;
        private Activity mActivity = null;
        private String mOriginFileName;

        /**
         * Constructor
         * 
         * @param originFileName The file may be compressed
         */
        public CompressDialog() {
            Logger.d(TAG, "CompressDialog()");
        }

        public void setOrigFileName(String originFileName) {
            Logger.d(TAG, "setOrigFileName():" + originFileName);
            mOriginFileName = originFileName;
        }

        @Override
        public void onSaveInstanceState(Bundle saveState) {
            // Override this method to workaround a google issue happen when API
            // level > 11
            Logger.d(TAG, "onSaveInstanceState()");
            saveState.putString(TAG, TAG);
            super.onSaveInstanceState(saveState);
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            Logger.d(TAG, "onCreateDialog");
            final AlertDialog alertDialog = new AlertDialog.Builder(getActivity(),
                    AlertDialog.THEME_HOLO_LIGHT).create();
            mActivity = getActivity();
            if (mContext != null && mActivity != null) {
                alertDialog.setTitle(mContext.getString(R.string.compress_image_title));
                LayoutInflater inflater = LayoutInflater.from(mActivity.getApplicationContext());
                View customView = inflater.inflate(
                        mContext.getResources().getLayout(R.layout.warning_dialog), null);
                mCheckNotRemind = (CheckBox) customView.findViewById(R.id.remind_notification);
                alertDialog.setView(customView);
                TextView contentView = (TextView) customView.findViewById(R.id.warning_content);
                contentView.setText(mContext.getString(R.string.compress_image_content));
                TextView remindAgainView = (TextView) customView.findViewById(R.id.remind_content);
                remindAgainView.setText(mContext.getString(R.string.file_size_remind_contents));
                alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                        mContext.getString(R.string.rcs_dialog_positive_button), this);
                alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                        mContext.getString(R.string.rcs_dialog_negative_button), this);
                alertDialog.setIconAttribute(android.R.attr.alertDialogIcon);
            } else {
                Logger.e(TAG, "activity is null in CompressDialog");
            }
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            Logger.i(TAG, "onClick() which is " + which);
            RcsSettings.getInstance().saveRemindCompressFlag(mCheckNotRemind.isChecked());
            if (which == DialogInterface.BUTTON_POSITIVE) {
                handleOk();
            } else {
                Logger.d(TAG, "the user cancle compressing image");
                handleCancel();
            }
            this.dismissAllowingStateLoss();
            mCompressDialog = null;
        }

        @Override
        public void onCancel (DialogInterface dialog)
        {
            Logger.i(TAG, "onCancel{} in CompressDialog entry");
            reset();
            Logger.i(TAG, "onCancel{} in CompressDialog exit");
        }

        private void handleOk() {
            Logger.i(TAG, "handleOk()");
            new AsyncTask<Void, Void, String>() {
                @Override
                protected String doInBackground(Void... params) {
                    if (mCheckNotRemind.isChecked()) {
                        Logger.d(TAG, "the user enable compressing image and not remind again");
                        RcsSettings.getInstance().setCompressingImage(true);
                    }
                    return com.mediatek.rcse.service.Utils.compressImage(mOriginFileName);
                }

                @Override
                protected void onPostExecute(String result) {
                    Logger.v(TAG, "onPostExecute(),result = " + result);
                    if (result != null) {
                        mOutGoingImageName = result;
                        startImageSharing(result);
                    } else {
                        reset();
                    }
                }
            }.execute();
        }

        private void handleCancel() {
            Logger.i(TAG, "handleCancel()");
            mOutGoingImageName = mOriginFileName;
            startImageSharing(mOriginFileName);
        }
    }

    public class WarningDialog extends DialogFragment implements DialogInterface.OnClickListener, DialogInterface.OnCancelListener {
        static final String TAG = "WarningDialog";
        private CheckBox mCheckRemind = null;
        Activity mActivity = null;
        String mFileName = null;
        int entry_type = entry_initiate;

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            mActivity = getActivity();
            alertDialog = new AlertDialog.Builder(getActivity(), AlertDialog.THEME_HOLO_LIGHT)
            .create();
            if (mContext != null && mActivity != null) {
                alertDialog.setTitle(mContext.getString(R.string.file_size_warning));
                LayoutInflater inflater = LayoutInflater.from(mActivity.getApplicationContext());
                View customView = inflater.inflate(
                        mContext.getResources().getLayout(R.layout.warning_dialog), null);
                mCheckRemind = (CheckBox) customView.findViewById(R.id.remind_notification);
                alertDialog.setView(customView);
                TextView contentView = (TextView) customView.findViewById(R.id.warning_content);
                contentView.setText(mContext.getString(R.string.file_size_warning_contents));
                TextView remindAgainView = (TextView) customView.findViewById(R.id.remind_content);
                remindAgainView.setText(mContext.getString(R.string.file_size_remind_contents));
                alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                        mContext.getString(R.string.rcs_dialog_positive_button), this);
                alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                        mContext.getString(R.string.rcs_dialog_negative_button), this);
                alertDialog.setIconAttribute(android.R.attr.alertDialogIcon);
            }else {
                Logger.e(TAG, "activity is null in WarningDialog");
            }
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            Logger.i(TAG, "onClick() which is " + which);
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mCheckRemind != null) {
                    boolean isCheck = mCheckRemind.isChecked();
                    Logger.w(TAG, "WarningDialog onClick ischeck" + isCheck);
                    RcsSettings.getInstance().saveRemindWarningLargeImageFlag(isCheck);
                }
                if (mFileName != null) {
                    if(entry_type == entry_terminated){
                        acceptImageSharing();
                    }else if(entry_type == entry_initiate){
                        mOutGoingImageName = mFileName;
                        Logger.v(TAG, "onwarning Dialog mOutGoingImageName is: " + mOutGoingImageName);
                        if (mCallScreenHost != null) {
                            Utils.setInImageSharing(true);
                            prepareStartImageSharing(mOutGoingImageName);
                        } else {
                            Logger.d(TAG, "onReceive mCallScreenHost is null");
                        }
                    }
}
            }else{
                declineImageSharing();
            }
            this.dismissAllowingStateLoss();
            if(mReceiveWarningDialog != null){
                mReceiveWarningDialog = null;
            }else if(mInviteWarningDialog !=  null){
                mInviteWarningDialog = null;
            }
        }

        @Override
        public void onCancel (DialogInterface dialog)
        {
            Logger.i(TAG, "onCancel{} in WarningDialog entry");
            reset();
            Logger.i(TAG, "onCancel{} in WarningDialog exit");
        }

        public void saveParameters(String fileName, int entry_type) {
            this.mFileName = fileName;
            this.entry_type = entry_type;
        }
    }

    public class RepickDialog extends DialogFragment implements DialogInterface.OnClickListener, DialogInterface.OnCancelListener {
        private static final String TAG = "RepickDialog";
        private int mRequestCode = 0;
        Activity mActivity = null;

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            mActivity = getActivity();
            alertDialog =
                    new AlertDialog.Builder(getActivity(), AlertDialog.THEME_HOLO_LIGHT).create();
            if (mContext != null && mActivity != null) {
                alertDialog.setIconAttribute(android.R.attr.alertDialogIcon);
                alertDialog.setTitle(mContext.getString(R.string.large_file_repick_title));
                alertDialog.setMessage(mContext.getString(R.string.large_file_repick_message));
                alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                        mContext.getString(R.string.rcs_dialog_positive_button), this);
                alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                        mContext.getString(R.string.rcs_dialog_negative_button), this);
            }else {
                Logger.e(TAG, "activity is null in RepickDialog");
            }
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            Logger.i(TAG, "onClick() which is " + which);
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mRequestCode == RichcallProxyActivity.REQUEST_CODE_CAMERA) {
                    startCamera();
                } else if (mRequestCode == RichcallProxyActivity.REQUEST_CODE_GALLERY) {
                    startGallery();
                } 
            }
            this.dismissAllowingStateLoss();
            mRepickDialog = null;
        }

        @Override
        public void onCancel (DialogInterface dialog)
        {
            Logger.i(TAG, "onCancel{} in RepickDialog entry");
            reset();
            Logger.i(TAG, "onCancel{} in RepickDialog exit");
        }

        public void saveSelection(int which) {
            this.mRequestCode = which;
        }
    }

}

