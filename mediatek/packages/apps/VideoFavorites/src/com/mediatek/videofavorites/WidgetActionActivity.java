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
 */

package com.mediatek.videofavorites;

import android.app.Activity;
import android.content.ContentValues;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.graphics.Rect;
import android.media.CamcorderProfile;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.preference.PreferenceManager;
import android.provider.ContactsContract;
import android.provider.MediaStore;
import android.provider.MediaStore.Video;
import android.widget.Toast;

import com.mediatek.camcorder.CamcorderProfileEx;
import com.mediatek.transcode.VideoTranscode;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;



/**
 * This activity handles pending intent from collection items:
 * the behavior should be different for different actions:
 * <p>
 * ACTION_PICK, with no data set: <br>
 * launch contact first for user to select a contact.
 * after obtaining the result, enter video gallery to choose a video
 * and finally encode it and put into provider.
 * <p>
 * ACTION_PICK, with data, and extra which contain the entry id : <br>
 * The data is supposed to be video uri, we'll launch contact to select a contact.
 * <p>
 * ACTION_VIEW, with data: <br>
 * Launch the activity which can view the contact uri (should be contact uri)
 * <p>
 * ACTION_DELETE, with data:<br>
 * The data contains the uri in video favorite's content provider, and delete the desired entry.
 */
public class WidgetActionActivity extends Activity implements
    ProgressDialogFragment.DialogActionListener, AlertDialogFragment.OnClickListener {

    private static final String TAG = "WidgetAction";

    public static final String ACTION_LAUNCH_RECORDER = "action_launch_recorder";

    /* checking pick contact or video at start*/
    public static final String KEY_ACTION_PICK_TYPE = "action_pick_type";

    // for ACTION_PICK and for starting following pick activity;
    public static final int CODE_PICK_BOTH = 0;
    public static final int CODE_PICK_CONTACT = 1;
    public static final int CODE_PICK_VIDEO = 2;
    public static final int CODE_RECORD_VIDEO = 3;
    public static final int CODE_VIEW_CONTACT = 4;

    private final ContentValues mValues = new ContentValues();

    private static final String KEY_VIDEO_URI = VideoFavoritesProviderValues.Columns.VIDEO_URI;
    private static final String KEY_CONTACT_URI = VideoFavoritesProviderValues.Columns.CONTACT_URI;
    private static final String KEY_NAME = VideoFavoritesProviderValues.Columns.NAME;
    private static final String KEY_LAUNCH_ACTIVITY = "launched";

    ProgressDialogFragment mProgressDlg;
    private static final int UPDATE_INTERVAL = 200;

    // private static final int MSG_RESERVED = 0;
    private static final int MSG_UPDATE_PROGRESS = 1;
    private static final int MSG_SUICIDE = 2;
    private static final int MSG_TIMER_TEST = 3;
    private static final int MSG_TRANSCODE_UNSUPPORTED_SUICIDE = 4;
    private static final int MSG_TRANSCODE_INVALID_SUICIDE = 5;
    private static final int MSG_START_TRANSCODE = 6;

    private int mPickType;
    private Uri mVideoUri;
    private int mIndexId = -1;

    private long mStartTime = -1;
    private boolean mEnableTimerTest;
    private boolean mPaused = true;

    private AlertDialogFragment mAlertDlg;
    private Uri mUri;

    private Uri mContactUri;

    private boolean mLeaveForActivity;

    private boolean mIsTransCoding;
    private boolean mLaunchActivity;

    private TranscodeTask mTranscodeTask;


    private void showToastAndSuicide(int message, int delay) {
        if (mProgressDlg != null && mProgressDlg.isVisible()) {
            mProgressDlg.dismiss();
            mProgressDlg = null;
        }
        Toast.makeText(this, message, Toast.LENGTH_LONG).show();
        mHandler.sendEmptyMessageDelayed(MSG_SUICIDE, delay);
    }

    /**
     * This Handler is used to post message back onto the main thread of the
     * application
     */
    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_UPDATE_PROGRESS:
                if (mProgressDlg == null || mTranscodeTask == null) {
                    break;
                }
                int progress = mTranscodeTask.getProgress();
                mProgressDlg.setProgress(progress);
                if (progress < 100) {
                    mHandler.sendEmptyMessageDelayed(MSG_UPDATE_PROGRESS, UPDATE_INTERVAL);
                }
                break;

            case MSG_SUICIDE:
                if (mProgressDlg != null && mProgressDlg.isVisible()) {
                    mProgressDlg.dismiss();
                }
                finish();
                break;

            case MSG_TRANSCODE_UNSUPPORTED_SUICIDE:
                showToastAndSuicide(R.string.toast_video_unsupported_resolution, 200);
                break;

            case MSG_TRANSCODE_INVALID_SUICIDE:
                showToastAndSuicide(R.string.toast_invalid_video, 200);
                break;

            case MSG_TIMER_TEST:
                Xlog.v(TAG, "MSG_TIMER_TEST, waited:" + (SystemClock.uptimeMillis() - mStartTime));

                if (!mPaused) {
                    Xlog.v(TAG, "sendEmptyMessageDelayed(), delay:" + UPDATE_INTERVAL);
                    mStartTime = SystemClock.uptimeMillis();
                    mHandler.sendEmptyMessageDelayed(MSG_TIMER_TEST, UPDATE_INTERVAL);
                }
                break;

            case MSG_START_TRANSCODE:
                if (mTranscodeTask != null
                        && mTranscodeTask.getStatus() == AsyncTask.Status.PENDING) {
                    mTranscodeTask.execute(mVideoUri);
                }
                break;

            default:
                break;
            }
        }
    };

    private static final String SELECT_ACCOUNT_TYPE = "account_type";
    private static final int TYPE_PHONE_CONTACT = 1;
    private void fireContactSelectActivity() {
        Intent i = new Intent(Intent.ACTION_GET_CONTENT);
        i.setType(ContactsContract.Contacts.CONTENT_ITEM_TYPE);
        i.putExtra(SELECT_ACCOUNT_TYPE, TYPE_PHONE_CONTACT);
        i.setPackage("com.android.contacts");

        mLeaveForActivity = true;
        startActivityForResult(i, CODE_PICK_CONTACT);
    }

    private void fireVideoSelectActivity() {
        Intent i = new Intent();
        i.setAction(Intent.ACTION_PICK);
        i.setType("video/*");
        mLeaveForActivity = true;
        startActivityForResult(i, CODE_PICK_VIDEO);
    }

    private boolean isPickContactOnly() {
        Xlog.e(TAG, "isPickContactOnly()" + (mPickType == CODE_PICK_CONTACT));
        return mPickType == CODE_PICK_CONTACT;
    }

    private boolean isPickVideoOnly() {
        Xlog.e(TAG, "isPickVideoOnly()" + (mPickType == CODE_PICK_VIDEO));
        return mPickType == CODE_PICK_VIDEO;
    }

    private void initActionPick() {
        Bundle extras = getIntent().getExtras();
        if (extras == null) {
            Xlog.e(TAG, "initActionPick, extra is null");
            if (getFavoriteCount() >= WidgetAdapter.LARGE_MAX_NUM_VIDEOS) {
                // block the race condition issue that may happen  when there's already 4 added
                // but widget is not updating yet.
                finish();
            }
            return;
        }
        mPickType = extras.getInt(KEY_ACTION_PICK_TYPE, CODE_PICK_BOTH);
        Xlog.i(TAG, "initActionPick, picktype: " + mPickType);

        if (isPickContactOnly()) {
            mVideoUri = getIntent().getData();
        } else if (isPickVideoOnly()) {
            mValues.put(KEY_CONTACT_URI, getIntent().getData().toString());
        } else if (!isStorageAvailable()) {
            Xlog.e(TAG, "storage is low");
            showLowStorageToast();
            finish();
        }

        mIndexId = extras.getInt(VideoFavoritesProviderValues.Columns._ID, -1);
    }

    private void initDelete() {
        final Intent i = getIntent();
        final Bundle extras = i.getExtras();
        mUri = i.getData();

        String name;
        if (extras == null) {
            Xlog.e(TAG, "extras should not be null");
            finish();
        } else {
            name = extras.getString(WidgetAdapter.KEY_NAME);
            mAlertDlg = AlertDialogFragment.newInstance(R.string.delete_dialog_title,
                        R.string.delete_dialog_prompt, name);
        }
    }

    private boolean isValidUri(Uri uri) {
        Cursor c = getContentResolver().query(uri, CONTACT_PROJECTION,
                                              null, null, null);
        boolean ret = true;

        if (c == null) {
            ret = false;
        } else {
            if (c.getCount() == 0) {
                ret = false;
            }
            c.close();
        }

        if (!ret) {
            Xlog.e(TAG, "contact is gone");
        }
        return ret;
    }

    private void initActionView() {
        Uri uri = getIntent().getData();
        if (isValidUri(uri)) {
            Intent i = new Intent(Intent.ACTION_VIEW, getIntent().getData());
            mLeaveForActivity = true;
            startActivity(i);
        } else {
            // uri is gone, ask user to repick
            Bundle extras = getIntent().getExtras();
            if (extras == null) {
                Xlog.e(TAG, "This should not happen. check the service why we don't have extra passed in intent");
            } else {
                mIndexId = extras.getInt(VideoFavoritesProviderValues.Columns._ID, -1);
                Xlog.e(TAG, "contact is gone, clear it");
                Uri target = Uri.parse(String.format(Locale.US, "%s/%d",
                                                     VideoFavoritesProviderValues.Columns.CONTENT_URI.toString() , mIndexId));
                mValues.put(VideoFavoritesProviderValues.Columns.CONTACT_URI.toString(), "");
                mValues.put(VideoFavoritesProviderValues.Columns.NAME.toString(), "");
                getContentResolver().update(target, mValues, null, null);
                notifyDataUpdate();
            }
            finish();
        }
    }

    private static final String KEY_ENABLE_TRANSCODE = "key_enable_transcode";

    private boolean isStorageAvailable() {
        long space = Storage.getAvailableSpace(Storage.TRANSCODE_PATH_BASE, this);
        Xlog.d(TAG, "checkStorageSpace: " + space);

        return (space > Storage.LOW_STORAGE_THRESHOLD);
    }



    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        boolean enableRotation = getResources().getBoolean(R.bool.allow_rotation);
        Xlog.v(TAG, "rotation = " + enableRotation);
        setRequestedOrientation(enableRotation ? ActivityInfo.SCREEN_ORIENTATION_BEHIND :
                                ActivityInfo.SCREEN_ORIENTATION_NOSENSOR);

        String action = getIntent().getAction();

        if (action.equals(Intent.ACTION_PICK)) {
            initActionPick();
        } else if (action.equals(Intent.ACTION_DELETE)) {
            initDelete();
        } else if (action.equals(Intent.ACTION_VIEW)) {
            mPickType = CODE_VIEW_CONTACT;
            mLaunchActivity = true;
        } else if (ACTION_LAUNCH_RECORDER.equals(action)) {
            if (getFavoriteCount() >= WidgetAdapter.LARGE_MAX_NUM_VIDEOS) {
                finish();
            } else {
                mPickType = CODE_RECORD_VIDEO;
                mLaunchActivity = true;
            }
        } else if ("SWITCH_TRANSCODE".equals(action)) {
            // backdoor for switching on / off transcode mechanism, to run:
            // >adb shell am start -a SWITCH_DECODE -n com.mediatek.videofavorites/.SelectActivity
            SharedPreferences s = PreferenceManager.getDefaultSharedPreferences(this);
            boolean isTranscode = s.getBoolean(KEY_ENABLE_TRANSCODE, true);
            SharedPreferences.Editor editor =  s.edit();
            editor.putBoolean(KEY_ENABLE_TRANSCODE, !isTranscode).commit();
            Toast.makeText(this, "trnascode " + (isTranscode ? "disabled" : "enabled"),
                           Toast.LENGTH_LONG).show();
            finish();
        } else if (("TIMER_TEST").equals(action)) {
            mEnableTimerTest = true;
        }
    }

    private void updateContactToProvider() {
        mValues.put(KEY_VIDEO_URI, mVideoUri.toString());
        Uri target = Uri.parse(String.format(Locale.US, "%s/%d",
                                             VideoFavoritesProviderValues.Columns.CONTENT_URI.toString() , mIndexId));

        getContentResolver().update(target, mValues, null, null);
    }

    private boolean isTransCodeEnable() {
        SharedPreferences s = PreferenceManager.getDefaultSharedPreferences(this);
        return s.getBoolean(KEY_ENABLE_TRANSCODE, true);
    }


    private static int sRecorderQuality;
    private static final int VIDEO_WIDTH = 640;
    private static final int VIDEO_HEIGHT = 480;

    private boolean isVideoResolutionMatch(CamcorderProfile profile, int width, int height) {
        return (profile != null)
               && (profile.videoFrameWidth == width
                   && profile.videoFrameHeight == height);
    }

    private int getRecorderQuality() {
        if (sRecorderQuality != 0) {
            return sRecorderQuality;
        }

        // try mtk mediaum & mtk HIGH first to see if any VGA resolution
        CamcorderProfile profile;
        if (CamcorderProfile.hasProfile(CamcorderProfile.QUALITY_MTK_HIGH)) {
            profile = CamcorderProfileEx.getProfile(CamcorderProfile.QUALITY_MTK_HIGH);
            if (isVideoResolutionMatch(profile, VIDEO_WIDTH, VIDEO_HEIGHT)) {
                sRecorderQuality = CamcorderProfile.QUALITY_MTK_HIGH;
                return sRecorderQuality;
            }
        }

        if (CamcorderProfile.hasProfile(CamcorderProfile.QUALITY_MTK_MEDIUM)) {
            profile = CamcorderProfileEx.getProfile(CamcorderProfile.QUALITY_MTK_MEDIUM);
            if (isVideoResolutionMatch(profile, VIDEO_WIDTH, VIDEO_HEIGHT)) {
                sRecorderQuality = CamcorderProfile.QUALITY_MTK_MEDIUM;
                return sRecorderQuality;
            }
        }


        // no VGA resolution, try 480p & QCIF
        sRecorderQuality = CamcorderProfile.hasProfile(CamcorderProfile.QUALITY_480P) ?
                           CamcorderProfile.QUALITY_480P : CamcorderProfile.QUALITY_QCIF;
        return sRecorderQuality;

    }

    private void launchVideoRecorder() {
        mLeaveForActivity = true;
        Intent i = new Intent(MediaStore.ACTION_VIDEO_CAPTURE);
        i.putExtra(MediaStore.EXTRA_VIDEO_QUALITY, getRecorderQuality());
        startActivityForResult(i, CODE_RECORD_VIDEO);
    }


    @Override
    public void onResume() {
        super.onResume();

        Xlog.v(TAG, "onResume()");
        mLeaveForActivity = false;

        if (mHandler.hasMessages(MSG_SUICIDE) || isFinishing()) {
            return;
        }

        // launch VideoRecorder or Contact
        if (mLaunchActivity) {
            mLaunchActivity = false;
            if (mPickType == CODE_RECORD_VIDEO) {
                launchVideoRecorder();
            } else if (mPickType == CODE_VIEW_CONTACT) {
                initActionView();
            }
            return;
        } else if (mPickType == CODE_VIEW_CONTACT) {
            finish();
            return;
        }

        // delete-mode
        if (mAlertDlg != null) {
            mAlertDlg.show(getFragmentManager(), "alertDialog");
            return;
        }

        // race condition, we resume when transcode is finished
        if (mIsTransCoding) {
            Xlog.v(TAG, "transcoding, skipping rest onResume()");
            return;
        }

        // non-delete-mode.
        if (mEnableTimerTest) {
            mPaused = false;
            mHandler.removeMessages(MSG_TIMER_TEST);
            Xlog.v(TAG, "sendEmptyMessageDelayed(), delay:" + UPDATE_INTERVAL);
            mStartTime = SystemClock.uptimeMillis();
            mHandler.sendEmptyMessageDelayed(MSG_TIMER_TEST, UPDATE_INTERVAL);
            return;
        }

        Xlog.v(TAG, "mValues.containsKey(KEY_CONTACT_URI): " + mValues.containsKey(KEY_CONTACT_URI));
        if (mValues.containsKey(KEY_CONTACT_URI)) {
            Xlog.v(TAG, "mVideoUri: " + mVideoUri);

            if (mVideoUri == null) {
                fireVideoSelectActivity();
            } else if (isTransCodeEnable()) {
                if (isPickContactOnly()) {
                    updateContactToProvider();
                    // this is a tricky solution due to orientation change will eat the
                    // widget update request. we use a timer to delay the suicide of activity.
                    notifyDataUpdate();
                    finish();
                } else {
                    mProgressDlg = ProgressDialogFragment.newInstance(R.string.app_name,
                                   R.string.transcoding_prompt);
                    mProgressDlg.show(getFragmentManager(), "progressDialog");
                    mIsTransCoding = true;
                    mTranscodeTask = new TranscodeTask();
                    mHandler.sendEmptyMessageDelayed(MSG_START_TRANSCODE, 200);
                }
            } else {
                if (isPickContactOnly()) {
                    updateContactToProvider();
                } else {
                    insertOrUpdateVideoUriToProvider(mVideoUri);
                }
                notifyDataUpdate();
                finish();
            }
        } else {
            fireContactSelectActivity();
        }
    }

    @Override
    public void onPause() {
        Xlog.v(TAG, "onPause()");
        if (mEnableTimerTest) {
            mPaused = true;
            mHandler.removeMessages(MSG_TIMER_TEST);
        }

        if (mAlertDlg != null && mAlertDlg.isAdded()) {
            mAlertDlg.dismiss();
            finish();
        }

        super.onPause();
    }

    private static final String [] CONTACT_PROJECTION = new String [] {
        ContactsContract.Contacts.DISPLAY_NAME_PRIMARY,
    };

    private String getContactName(Uri contactUri) {
        // try to get first name, if no, use last name
        Cursor c = getContentResolver().query(contactUri, CONTACT_PROJECTION, null, null, null);
        String name = null;
        if (c.getCount() != 0) {
            c.moveToFirst();
            name = c.getString(c.getColumnIndex(ContactsContract.Contacts.DISPLAY_NAME_PRIMARY));
        } else {
            Xlog.e(TAG, "getContactName(), query failed:" + contactUri);
        }
        c.close();

        return name;
    }

    private void insertOrUpdateVideoUriToProvider(Uri uri) {
        Xlog.v(TAG, "insertOrUpdateVideoUriToPrivider(), indexId" + mIndexId);
        mValues.put(KEY_VIDEO_URI, uri.toString());
        if (mIndexId == -1) {
            getContentResolver().insert(VideoFavoritesProviderValues.Columns.CONTENT_URI, mValues);
        } else {
            Uri target = Uri.parse(String.format(Locale.US, "%s/%d",
                                                 VideoFavoritesProviderValues.Columns.CONTENT_URI.toString() , mIndexId));
            getContentResolver().update(target, mValues, null, null);
        }
    }

    private void cancelTranscode() {
        mHandler.removeMessages(MSG_START_TRANSCODE);
        mHandler.removeMessages(MSG_UPDATE_PROGRESS);
        boolean isCancelled = true;
        if (mTranscodeTask != null) {
            // it is possible that user cancel transcode when it is already finished.
            // in this case forceCancel return false. and we won't show popup.
            isCancelled = mTranscodeTask.forceCancel();
        }
        mProgressDlg.dismiss();
        mProgressDlg = null;

        if (isCancelled) {
            Toast.makeText(this, R.string.toast_video_add_cancelled, Toast.LENGTH_LONG).show();
        }
    }

    // dialog cancel listener.
    public void onCancel() {
        cancelTranscode();
        mHandler.sendEmptyMessageDelayed(MSG_SUICIDE, 500);
    }

    private void showLowStorageToast() {
        Toast.makeText(this, R.string.toast_low_storage,
                       Toast.LENGTH_LONG).show();
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        Xlog.d(TAG, "onActivityResult(): " + requestCode + "/" + resultCode);
        mLeaveForActivity = false;
        if (RESULT_OK == resultCode) {
            // sometimes it seems that the App will be killed during recording
            // and cause Recorder to be launched again
            // prevent it by resetting the flag.
            mLaunchActivity = false;

            Uri uri = data.getData();
            if (uri == null) {
                Xlog.e(TAG, "data uri is null");
                mValues.clear();
                if (CODE_RECORD_VIDEO == requestCode) {
                    Toast.makeText(this, R.string.toast_video_add_cancelled,
                                   Toast.LENGTH_LONG).show();
                }
                finish();
                return;
            }

            if (CODE_PICK_CONTACT == requestCode) {
                Xlog.v(TAG, "contact selected: " + uri);
                String name = getContactName(uri);
                if (name == null) {
                    Xlog.e(TAG, "cannot get name, pick contact failed");
                    Toast.makeText(this, R.string.toast_video_add_cancelled,
                            Toast.LENGTH_LONG).show();
                    finish();
                } else {
                    mValues.put(KEY_NAME, getContactName(uri));
                    mValues.put(KEY_CONTACT_URI, uri.toString());
                }
            } else if (CODE_PICK_VIDEO == requestCode) {
                Xlog.i(TAG, "video selected: " + uri);
                if (!isStorageAvailable()) {
                    Xlog.e(TAG, "storage is low");
                    showLowStorageToast();
                    finish();
                }
                mVideoUri = uri;
            } else if (CODE_RECORD_VIDEO == requestCode) {
                final ContentValues values = new ContentValues();
                String filePath = getRealPathFromUri(uri);
                if (filePath == null) {
                    Toast.makeText(this, R.string.toast_video_add_cancelled,
                            Toast.LENGTH_LONG).show();
                } else {
                    values.put(KEY_VIDEO_URI, Uri.fromFile(new File(filePath)).toString());
                    getContentResolver().insert(VideoFavoritesProviderValues.Columns.CONTENT_URI,
                            values);
                    notifyDataUpdate();
                }
                finish();
            }
        } else {
            mValues.clear();
            finish();
        }
    }

    public String getRealPathFromUri(Uri contentUri) {
        String [] proj = {MediaStore.Video.Media.DATA};
        Cursor c = getContentResolver().query(contentUri, proj, null, null, null);
        if (c.getCount() == 0) {
            Xlog.e(TAG, "getRealPathFromUri(), uri not found in content resolver");
            return null;
        }

        String realPath;
        try {
            int colIdx = c.getColumnIndexOrThrow(MediaStore.Video.Media.DATA);
            c.moveToFirst();
            realPath = c.getString(colIdx);
        } catch (IllegalArgumentException ie) {
            realPath = null;
            Xlog.e(TAG, "getRealPathFromUri(): " + ie);
        } finally {
            c.close();
        }
        return realPath;
    }


    private static final int ENCODE_WIDTH = 320;
    private static final int ENCODE_HEIGHT = 240;

    private class TranscodeTask extends AsyncTask<Uri, Integer, String> {

        private static final int RESULT_OK = 0;
        private static final int RESULT_RESOULTION_TOO_HIGH = -1;
        private static final int RESULT_INVALID_VIDEO = -2;

        private static final int DURATION_DEFAULT = 10000;

        private int mResult = RESULT_OK;

        private boolean mTranscode;

        private long mTranscoderHandle;

        private long mDuration = 0;
        private Rect mTargetRect;

        private String createName(long dateTaken) {
            Date date = new Date(dateTaken);
            SimpleDateFormat dateFormat = new SimpleDateFormat(
                getString(R.string.video_file_name_format), Locale.US);

            return dateFormat.format(date);
        }

        private void prepareFolder(String path) {
            File f = new File(path);
            if (f.exists()) {
                return;
            }

            if (!f.mkdirs()) {
                Xlog.e(TAG, "folder creation failed!");
            }
        }

        // sets video width/height in outRect, and returns video duration
        // video is considered invalid if duration is 0
        private long getSourceVideoInfo(String filePath, Rect outRect) {

            String strWidth = null;
            String strHeight = null;
            String strDuration = null;
            MediaMetadataRetriever retriever = new MediaMetadataRetriever();
            String hasVideo = null;


            // check if file size is 0.
            File f = new File(filePath);
            if (f.length() == 0) {
                return 0;
            }

            try {
                retriever.setDataSource(filePath);

                hasVideo = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_HAS_VIDEO);

                if (hasVideo == null) {
                    Xlog.e(TAG, "getSourceVideoRect, no videoTrack");
                    return 0;
                }

                strWidth = retriever.extractMetadata(
                                MediaMetadataRetriever.METADATA_KEY_VIDEO_WIDTH);
                strHeight = retriever.extractMetadata(
                                MediaMetadataRetriever.METADATA_KEY_VIDEO_HEIGHT);
                strDuration = retriever.extractMetadata(
                                MediaMetadataRetriever.METADATA_KEY_DURATION);
            } catch (IllegalArgumentException ex) {
                // Assume this is a corrupt video file
                Xlog.e(TAG, "Exception:" + ex);
            } finally {
                retriever.release();
            }

            if (strWidth == null || strHeight == null) {
                Xlog.e(TAG, "invalid video width/height");
                return 0;
            }

            int width = Integer.decode(strWidth).intValue();
            int height = Integer.decode(strHeight).intValue();
            if (width == 0 || height == 0) {
                Xlog.e(TAG, "video width/height is 0");
                return 0;
            }
            long duration = Integer.decode(strDuration).longValue();

            outRect.set(0, 0, width, height);

            return duration;
        }

        private Rect getTargetRect(int srcWidth, int srcHeight, int maxWidth, int maxHeight) {
            float rSrc = (float) srcWidth / srcHeight;
            float rMax = (float) maxWidth / maxHeight;

            int targetWidth;
            int targetHeight;

            // crop and scale

            if (rSrc < rMax) {
                targetWidth = maxWidth;
                targetHeight = targetWidth * srcHeight / srcWidth;
            } else {
                targetHeight = maxHeight;
                targetWidth = targetHeight * srcWidth / srcHeight;
                // width must be the factor of 16, find closest but smallest factor
                // so hight won't larger than mHeight
                if (targetWidth % 16 != 0) {
                    targetWidth = (targetWidth - 15) >> 4 << 4;
                    targetHeight = targetWidth * srcHeight / srcWidth;
                }
            }

            return new Rect(0, 0, targetWidth, targetHeight);
        }



        private String generateOutputPath(String inputName) {
            long dateTaken = System.currentTimeMillis();
            String postfix = createName(dateTaken);
            File inputFile = new File(inputName);

            prepareFolder(Storage.TRANSCODE_PATH);
            StringBuilder sb = new StringBuilder(Storage.TRANSCODE_PATH);
            sb.append(inputFile.getName());
            int i = sb.lastIndexOf(".");
            if (i == -1) {
                sb.append(postfix);
            } else {
                sb.insert(i, postfix);
            }
            return sb.toString();
        }

        protected void onPreExecute() {
            mTranscoderHandle = VideoTranscode.init();
            mHandler.sendEmptyMessageDelayed(MSG_UPDATE_PROGRESS, UPDATE_INTERVAL);
        }

        // only support 1 uri now.
        protected String doInBackground(Uri... uris) {
            // do something to target path
            String inputPath = getRealPathFromUri(uris[0]);
            String outputPath;

            Rect srcRect = new Rect();
            mDuration = getSourceVideoInfo(inputPath, srcRect);

            if (mDuration <= 0) {
                mResult = RESULT_INVALID_VIDEO;
                return null;
            }
            if (mDuration > DURATION_DEFAULT) {
                mDuration = DURATION_DEFAULT;
            }

            long startTime = SystemClock.uptimeMillis();

            if (srcRect.width() <= ENCODE_WIDTH || srcRect.height() <= ENCODE_HEIGHT) {
                // video is not very large, skip transcodeing.
                outputPath = inputPath;
            } else {
                mTranscode = true;
                outputPath = generateOutputPath(inputPath);
                mTargetRect = getTargetRect(srcRect.width(), srcRect.height(), ENCODE_WIDTH,
                                                ENCODE_HEIGHT);
                Xlog.v(TAG, "srcRect: " + srcRect + " targetRect: " + mTargetRect);
                int result = VideoTranscode.transcode(mTranscoderHandle, inputPath, outputPath,
                                                      (long) mTargetRect.width(), (long) mTargetRect.height(), (long)0,
                                                      (long) DURATION_DEFAULT);

                Xlog.e(TAG, "transcode result: " + result);

                if (result != VideoTranscode.NO_ERROR) {
                    mResult = (result == VideoTranscode.ERROR_UNSUPPORTED_VIDEO) ?
                              RESULT_RESOULTION_TOO_HIGH : RESULT_INVALID_VIDEO;
                }
            }
            long timecost = SystemClock.uptimeMillis() - startTime;

            Xlog.v(TAG, "transcode spend(ms):" + timecost);
            if (mResult != VideoTranscode.NO_ERROR) {
                outputPath = null;
            }

            return outputPath;
        }

        protected void onPostExecute(String outPathName) {
            VideoTranscode.deinit(mTranscoderHandle);
            mTranscoderHandle = 0;
            if (mProgressDlg == null) {
                Xlog.e(TAG, "cancelled due to dialog dismissed");
                if (outPathName != null) {
                    File outFile = new File(outPathName);
                    outFile.delete();
                }
                return;
            }

            mHandler.removeMessages(MSG_UPDATE_PROGRESS);
            if (mResult != VideoTranscode.NO_ERROR || outPathName == null) {
                Xlog.e(TAG, "result: " + mResult);
                mHandler.sendEmptyMessage((mResult == RESULT_RESOULTION_TOO_HIGH) ?
                                          MSG_TRANSCODE_UNSUPPORTED_SUICIDE : MSG_TRANSCODE_INVALID_SUICIDE);
                return;
            }

            if (mTranscode) {
                ContentValues v = new ContentValues(5);
                long dateTaken = System.currentTimeMillis();
                long fileLength = new File(outPathName).length();
                if (fileLength == 0) {
                    Xlog.e(TAG, "File length is 0, transcode failed or disk full, don't insert it");
                    mHandler.sendEmptyMessage(MSG_TRANSCODE_INVALID_SUICIDE);
                    return;
                }

                v.put(Video.VideoColumns.RESOLUTION,
                        mTargetRect.width() + "x" + mTargetRect.height());
                v.put(Video.Media.DATE_TAKEN, dateTaken);
                v.put(Video.Media.SIZE, fileLength);
                v.put(Video.Media.DATA, outPathName);
                v.put(Video.Media.DISPLAY_NAME, outPathName);
                v.put(Video.Media.MIME_TYPE, "video/mp4");
                v.put(Video.Media.DURATION, mDuration);

                Uri videoTable = Uri.parse("content://media/external/video/media");
                Uri inserted = getContentResolver().insert(videoTable, v);
                if (inserted == null) {
                    Xlog.e(TAG, "insert failed");
                }
            }

            mVideoUri = Uri.fromFile(new File(outPathName));
            Xlog.d(TAG, "onPostExecute(), video inserted: " + mVideoUri);
            insertOrUpdateVideoUriToProvider(mVideoUri);
            notifyDataUpdate();
            mProgressDlg.setProgress(100);
            mHandler.sendEmptyMessage(MSG_SUICIDE);
        }

        public int getProgress() {
            AsyncTask.Status s = getStatus();
            if (s == AsyncTask.Status.RUNNING) {
                return VideoTranscode.getProgress(mTranscoderHandle);
            }
            return (s == AsyncTask.Status.PENDING) ? 0 : 100;
        }

        public boolean forceCancel() {
            if (mTranscoderHandle == 0) {
                return false;
            }
            VideoTranscode.cancel(mTranscoderHandle);
            return true;
        }
    }

    private void notifyDataUpdate() {
        VideoFavoritesRootView.sendDataUpdateBroadcast(this);
    }

    //
    //  onClickListener()
    //////////////////////
    @Override
    public void onClickPositiveButton() {
        getContentResolver().delete(mUri, null, null);
        mAlertDlg.dismiss();
        notifyDataUpdate();
        finish();
    }
    public void onClickNegativeButton() {
        mAlertDlg.dismiss();
        finish();
    }

    // fix the issue that the alert dialog and transcode won't be dismissed when user
    // pressed home key.
    @Override
    public void onUserLeaveHint() {
        if (mLeaveForActivity) {
            return;
        }

        if (mAlertDlg != null) {
            mAlertDlg.dismiss();
        }

        if (mProgressDlg != null) {
            cancelTranscode();
        }

        finish();
    }

    private void saveToBundle(Bundle b, ContentValues v, String key) {
        if (v == null || b == null) {
            Xlog.e(TAG, "Failed to save bundle b:" + b + ", v:" + v);
            return;
        }

        if (v.containsKey(key)) {
            Xlog.i(TAG, "saving key: #" + key);
            b.putString(key, v.getAsString(key));
        }
    }

    private void loadFromBundle(Bundle b, ContentValues v, String key) {
        if (v == null || b == null) {
            Xlog.e(TAG, "Failed to load bundle: b:" + b + ", v:" + v);
            return;
        }

        if (b.containsKey(key)) {
            v.put(key, b.getString(key));
            Xlog.i(TAG, "Loaded key:" + key);
        }

    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        Xlog.i(TAG, "onSaveInstanceState()");
        outState.putBoolean(KEY_LAUNCH_ACTIVITY, mLaunchActivity);
        saveToBundle(outState, mValues, KEY_NAME);
        saveToBundle(outState, mValues, KEY_CONTACT_URI);
        saveToBundle(outState, mValues, KEY_VIDEO_URI);
        super.onSaveInstanceState(outState);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        Xlog.i(TAG, "onRestoreInstanceState()");
        mLaunchActivity = savedInstanceState.getBoolean(KEY_LAUNCH_ACTIVITY);
        loadFromBundle(savedInstanceState, mValues, KEY_NAME);
        loadFromBundle(savedInstanceState, mValues, KEY_CONTACT_URI);
        loadFromBundle(savedInstanceState, mValues, KEY_VIDEO_URI);
    }

    private static final String [] PROJ_COLS = new String[] {"count(*)"};
    private int getFavoriteCount() {
        Cursor cur = getContentResolver().query(
                         VideoFavoritesProviderValues.Columns.CONTENT_URI, PROJ_COLS, null, null, null);
        cur.moveToFirst();
        final int cnt = cur.getInt(0);
        cur.close();
        Xlog.i(TAG, "getFavoriteCount():"  + cnt);
        return cnt;
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Xlog.v(TAG, "onConfigurationChanged():" + newConfig);
        // We know it's orientation change, and we don't want to change anyting.
        // Ignore it on purpose.
    }
}

