/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.FMRadio;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.media.MediaPlayer;
import android.media.MediaRecorder;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Environment;
import android.os.FileUtils;
import android.os.StatFs;
import android.os.SystemClock;
import android.provider.MediaStore;
import android.text.format.DateFormat;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

/**
 * This class provider interface to recording, stop recording, save recording file, play recording file
 *
 */
public class FMRecorder implements MediaPlayer.OnCompletionListener, MediaPlayer.OnErrorListener,
        MediaRecorder.OnErrorListener {
    private static final String TAG = "FmRx/Recorder"; // log tag
    private static final String RECORDING_FILE_PREFIX = "FM"; // file prefix
    public static final String RECORDING_FILE_EXTENSION = ".ogg"; // file extension
    private static final String FM_RECORD_FOLDER = "FM Recording"; // recording file folder
    private static final String RECORDING_FILE_TYPE = "audio/ogg";
    private static final String RECORDING_FILE_SOURCE = "FM Recordings";
    // error type no sdcard
    public static final int ERROR_SDCARD_NOT_PRESENT = 0; 
    // error type sdcard not have enough space
    public static final int ERROR_SDCARD_INSUFFICIENT_SPACE = 1; 
    // error type can't write sdcard
    public static final int ERROR_SDCARD_WRITE_FAILED = 2;
    // error type recorder internal error occur
    public static final int ERROR_RECORDER_INTERNAL = 3;
    // error type player internal error occur
    public static final int ERROR_PLAYER_INTERNAL = 4;
    // FM Recorder state not recording and not playing
    public static final int STATE_IDLE = 5;
    // FM Recorder state recording
    public static final int STATE_RECORDING = 6;
    // FM Recorder state playing
    public static final int STATE_PLAYBACK = 7;
    // FM Recorder state invalid, need to check
    public static final int STATE_INVALID = -1;
    
    // record need minimum storage space
    public static final long LOW_SPACE_THRESHOLD = 512 * 1024;
    // is record file delete after sdcard insert
    private boolean mDeleteUponSDInsertion = false;
    // use to record current FM recorder state
    public int mInternalState = STATE_IDLE;
    // current sdcard id
    private int mSDCardID = 0;
    // the recording time after start recording
    private long mRecordTime = 0;
    // record start time
    private long mRecordStartTime = 0;
    // the record file path delete before sdcard eject
    private String mOldFilePath = null;
    // current record file 
    private File mRecordFile = null;
    // record current record file is saved by user
    private boolean mIsRecordingFileSaved = false;
    // listener use for notify service the record state or error state
    private OnRecorderStateChangedListener mStateListener = null;
    // player use for play recording file
    private MediaPlayer mPlayer = null;
    // recorder use for record file
    private MediaRecorder mRecorder = null;
    /**
     * recording the voice of FM, also check the pre-conditions, if not meet, 
     * will return an error message to the caller. if can start recording success,
     * will set FM record state to recording and notify to the caller 
     */
    public void startRecording(Context context) {
        LogUtils.d(TAG, ">> startRecording");
        mRecordTime = 0;
        /*
        // check whether sdcard is mounted, if not mounted notify caller error message
        String state = FMRadioUtils.getInternalStorageState(context);
        if (!Environment.MEDIA_MOUNTED.equals(state)) {
            // external storage is NOT ready
            LogUtils.e(TAG, "SD card is not ready!!");
            setError(ERROR_SDCARD_NOT_PRESENT);
            return;
        }
        */
        String sdPath = FMRadioUtils.getDefaultStoragePath();
        //mSDCardID = FileUtils.getFatVolumeId(sdPath);
        // check whether have sufficient storage space, if not will notify caller error message
        if (!isHaveAvailableStorage(sdPath)) {
            setError(ERROR_SDCARD_INSUFFICIENT_SPACE);
            LogUtils.e(TAG, "SD card does not have sufficient space!!");
            return;
        }
        //LogUtils.d(TAG, "current card id=" + mSDCardID);
        
        // get external storage directory
        File sdDir = new File(sdPath);
        LogUtils.d(TAG, "external storage dir = " + sdDir.getAbsolutePath());
        File recordingDir = new File(sdDir, FM_RECORD_FOLDER);
        // exist a file named FM Recording, so can't create FM recording folder 
        if (recordingDir.exists() && !recordingDir.isDirectory()) { 
            LogUtils.e(TAG, "A FILE with name \"FM Recording\" already exists!!");
            setError(ERROR_SDCARD_WRITE_FAILED);
            return;
        } else if (!recordingDir.exists()) { //try to create recording folder
            boolean mkdirResult = recordingDir.mkdir();
            if (!mkdirResult) { // create recording file failed
                setError(ERROR_SDCARD_WRITE_FAILED);
                return;
            }
        }
        // create recording temporary file
        long curTime = System.currentTimeMillis();
        Date date = new Date(curTime);
        SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyyMMdd_HHmmss", Locale.ENGLISH);
        String time = simpleDateFormat.format(date);
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append(RECORDING_FILE_PREFIX).append("_").append(time).append(RECORDING_FILE_EXTENSION);
        String name = stringBuilder.toString();
        mRecordFile = new File(recordingDir, name);
        try {            
            if (mRecordFile.createNewFile()) {
                LogUtils.e(TAG, "createNewFile success");
            }
        } catch (IOException ioex) {
            LogUtils.e(TAG, "IOException while createTempFile: " + ioex);
            ioex.printStackTrace();
            setError(ERROR_SDCARD_WRITE_FAILED);
            return;
        }
        // set record parameter and start recording
        try {
            LogUtils.d(TAG, "new record file is:" + mRecordFile.getName());

            mRecorder = new MediaRecorder();
            LogUtils.d(TAG, "startRecording: create new media record instance");
            mRecorder.setOnErrorListener(this);
            mRecorder.setAudioSource(MediaRecorder.AudioSource.FM);
            mRecorder.setOutputFormat(MediaRecorder.OutputFormat.OUTPUT_FORMAT_OGG);
            mRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.VORBIS);
            final int samplingRate = 44100;
            mRecorder.setAudioSamplingRate(samplingRate);
            final int bitRate = 12800;
            mRecorder.setAudioEncodingBitRate(bitRate);
            final int channels = 2;
            mRecorder.setAudioChannels(channels);
            mRecorder.setOutputFile(mRecordFile.getAbsolutePath());
            mRecorder.prepare();
            mRecordStartTime = SystemClock.elapsedRealtime();
            mRecorder.start();
            mIsRecordingFileSaved = false;
            LogUtils.d(TAG, "startRecording: start");
        } catch (IllegalStateException e) {
            LogUtils.e(TAG, "IllegalStateException while starting recording!");
            setError(ERROR_RECORDER_INTERNAL);
            return;
        } catch (IOException e) {
            LogUtils.e(TAG, "IOException while starting recording!");
            setError(ERROR_RECORDER_INTERNAL);
            return;
        } catch (RuntimeException exception) {
            LogUtils.e(TAG, "RuntimeException while start recording");
            setError(ERROR_RECORDER_INTERNAL);
            return;
        }
        setState(STATE_RECORDING);
        LogUtils.d(TAG, "<< startRecording");
    }

    /**
     * stop recording, compute recording time and update FM recorder state
     */
    public void stopRecording() {
        LogUtils.d(TAG, ">> stopRecording");
        if (STATE_RECORDING != mInternalState) {
            LogUtils.w(TAG, "stopRecording() called in wrong state!!");
            return;
        }

        mRecordTime = SystemClock.elapsedRealtime() - mRecordStartTime;
        stopRecorder();
        setState(STATE_IDLE);
        LogUtils.d(TAG, "<< stopRecording");
    }
    
    /**
     * play current recorded file, if failed notify error message to caller, 
     * if success update FM recorder state
     */
    public void startPlayback() {
        LogUtils.d(TAG, ">> startPlayback");
        if (null == mRecordFile) {
            LogUtils.e(TAG, "no file to playback!");
            return;
        }

        mPlayer = new MediaPlayer();
        try {
            mPlayer.setDataSource(mRecordFile.getAbsolutePath());
            LogUtils.d(TAG, "MediaPlayer.setDataSource(" + mRecordFile.getAbsolutePath() + ")");
            mPlayer.setOnCompletionListener(this);
            mPlayer.setOnErrorListener(this);
            mPlayer.prepare();
            LogUtils.d(TAG, "MediaPlayer.prepare()");
            mPlayer.start();
        } catch (IOException e) {
            LogUtils.e(TAG, "Exception while trying to playback recording file: " + e);
            setError(ERROR_PLAYER_INTERNAL);
            return;
        } catch (IllegalArgumentException e) {
            LogUtils.e(TAG, "Exception while trying to playback recording file: " + e);
            setError(ERROR_PLAYER_INTERNAL);
            return;
        } catch (SecurityException e) {
            LogUtils.e(TAG, "Exception while trying to playback recording file: " + e);
            setError(ERROR_PLAYER_INTERNAL);
            return;
        } catch (IllegalStateException e) {
            LogUtils.e(TAG, "Exception while trying to playback recording file: " + e);
            setError(ERROR_PLAYER_INTERNAL);
            return;
        }
        setState(STATE_PLAYBACK);
        LogUtils.d(TAG, "<< startPlayback");
    }
    /**
     * stop playing the current recording file, and update FM recorder state
     */
    public void stopPlayback() {
        LogUtils.d(TAG, ">> stopPlayback");
        if (null == mPlayer 
            || STATE_PLAYBACK != mInternalState 
            || (null != mPlayer && !mPlayer.isPlaying())) {
            LogUtils.w(TAG, "stopPlayback called in wrong state!!");
            // need call back to update button status
            setState(STATE_IDLE);
            return;
        }
        
        stopPlayer();
        setState(STATE_IDLE);
        LogUtils.d(TAG, "<< stopPlayback");
    }
    
    /**
     * compute the current record time
     * @return current record time
     */
    public long recordTime() {
        if (STATE_RECORDING == mInternalState) {
            mRecordTime = SystemClock.elapsedRealtime() - mRecordStartTime;
        }
        return mRecordTime;
    }
    
    /**
     * get FM recorder current state
     * @return FM recorder current state
     */
    public int getState() {
        return mInternalState;
    }
    
    /**
     * get current recording file name
     * @return current recording file name
     */
    public String getRecordingName() {
        if (null != mRecordFile) {
            String fileName = mRecordFile.getName();
            if (fileName.toLowerCase().endsWith(RECORDING_FILE_EXTENSION) 
                && fileName.length() > RECORDING_FILE_EXTENSION.length()) {
                // remove the extension sub string first
                fileName = fileName.substring(0, fileName.length() - RECORDING_FILE_EXTENSION.length());
            }
            return fileName;
        }
        return null;
    }

    /**
     * get current recording file name with full path
     * @return
     */
    public String getRecordingNameWithPath() {
        if (null != mRecordFile) {
            String fileName = mRecordFile.getAbsolutePath();
            if (fileName.toLowerCase().endsWith(RECORDING_FILE_EXTENSION) 
                && fileName.length() > RECORDING_FILE_EXTENSION.length()) {
                // remove the extension sub string first
                fileName = fileName.substring(0, fileName.length() - RECORDING_FILE_EXTENSION.length());
            }
            LogUtils.d(TAG, "getRecordingNameWithPath: fileName is " + fileName);
            return fileName;
        }
        return null;
    }
    
    /**
     * save recording with the given name, and save recording file info to database 
     * @param context context of activity or application
     * @param newName the name to override default recording name
     */
    public void saveRecording(Context context, String newName) {
        LogUtils.d(TAG, ">> saveRecording(" + newName + ")");

        if (null == mRecordFile) {
            LogUtils.e(TAG, "<< saveRecording: recording file is null!");
            return;
        }
        
        File parentFile = mRecordFile.getParentFile();
        
        if (null == parentFile) {
            LogUtils.e(TAG, "<< saveRecording: parent recording file is null!");
            return;
        }

        // rename the recording file with given name
        if (null != newName && !newName.equals(getRecordingName())) {
            File sdFile = new File(parentFile.getPath(), newName + RECORDING_FILE_EXTENSION);
            if (sdFile.exists()) {
                LogUtils.w(TAG, "A file with the same new name will be deleted: " + sdFile.getAbsolutePath());
                if (!sdFile.delete()) {
                    LogUtils.e(TAG, "can't delete the file already exits");
                }
            }
            if (null != parentFile) {
                if (!mRecordFile.renameTo(new File(parentFile.getPath(), newName + RECORDING_FILE_EXTENSION))) {
                    LogUtils.e(TAG, "can't rename file, use default name to save");
                    newName = getRecordingName();
                }
                mRecordFile = new File(parentFile.getPath(), newName + RECORDING_FILE_EXTENSION);
            }
        }
        mIsRecordingFileSaved = true;
        // save recording file info to database
        addCurrentRecordingToDb(context);
        LogUtils.d(TAG, "<< saveRecording(" + newName + ")");
    }
    
    /**
     * discard current recording file, release recorder and player
     */
    public void discardRecording() {
        LogUtils.d(TAG, ">> discardRecording");
        // release recorder
        if ((STATE_RECORDING == mInternalState) && (null != mRecorder)) {
            stopRecorder();
       // release player
        } else if ((STATE_PLAYBACK == mInternalState) && (null != mPlayer)) {
            stopPlayer();
        }

        if (null != mRecordFile && !mIsRecordingFileSaved) {
            if (!mRecordFile.delete()) {
                // deletion failed, possibly due to hot plug out SD card
                // delete again the next time SD card is inserted again.
                LogUtils.d(TAG, "discardRecording: deletion failed! will try deleting it again when card is inserted back");
                mOldFilePath = mRecordFile.getAbsolutePath();
                mDeleteUponSDInsertion = true;
            }
            mRecordFile = null;
            mRecordStartTime = 0;
            mRecordTime = 0;
        }
        setState(STATE_IDLE);
        LogUtils.d(TAG, "<< discardRecording");
    }
    /**
     * set the callback use to notify FM recorder state and error message 
     * @param listener the callback
     */
    public void registerRecorderStateListener(OnRecorderStateChangedListener listener) {
        mStateListener = listener;
    }
    
    /**
     * interface to notify FM recorder state and error message
     */
    public interface OnRecorderStateChangedListener {
        
        /**
         * notify FM recorder state
         * @param state current FM recorder state
         */
        void onRecorderStateChanged(int state);
        
        /**
         * notify FM recorder error message
         * @param error error type
         */
        void onRecorderError(int error);
        
        /**
         * notify play FM record file complete
         */
        void onPlayRecordFileComplete();
    }
    
    /**
     * play current recording file complete, release player, update FM recorder state to idle
     * @param mp current player
     * 
     */
    public void onCompletion(MediaPlayer mp) {
        LogUtils.d(TAG, ">> MediaPlayer.onCompletion");
        synchronized (this) {
            if (null != mPlayer) {
                mPlayer.stop();
                mPlayer.release();
                mPlayer = null;
            }
        }
        setState(STATE_IDLE);
        if (null != mStateListener) {
            mStateListener.onPlayRecordFileComplete();
        }
        LogUtils.d(TAG, "<< MediaPlayer.onCompletion");
    }
    
    /**
     * player occur error, release player, notify error message, update FM recorder state to idle
     * @param mp current player
     * @param what error message type
     * @param extra error message extra
     * @return true or false indicate handle this error or not
     */
    public boolean onError(MediaPlayer mp, int what, int extra) {
        LogUtils.e(TAG, "MediaPlayer.onError: what=" + what + ", extra=" + extra);
        int error = (MediaPlayer.MEDIA_ERROR_SERVER_DIED == what) ? what : ERROR_PLAYER_INTERNAL;
        setError(error);
        synchronized (this) {
            if (null != mPlayer) {
                mPlayer.release();
                mPlayer = null;
            }
        }
        if (STATE_PLAYBACK == mInternalState) {
            setState(STATE_IDLE);
        }
        return true;
    }
    
    /**
     * recorder occur error, release player, notify error message, update FM recorder state to idle
     * @param mr current recorder
     * @param what error message type
     * @param extra error message extra
     */
    public void onError(MediaRecorder mr, int what, int extra) {
        LogUtils.e(TAG, "MediaRecorder.onError: what=" + what + ", extra=" + extra);
        stopRecorder();
        setError(ERROR_RECORDER_INTERNAL);
        if (STATE_RECORDING == mInternalState) {
            setState(STATE_IDLE);
        }
    }
    
    /**
     * sdcard insert, try to delete last temporary file
     */
    /*public void onSDInserted() {
        LogUtils.d(TAG, ">> onSDInserted");
        int newCardID = FileUtils.getFatVolumeId(FMRadioUtils.getDefaultStoragePath());
        LogUtils.d(TAG, "new card id=" + newCardID);
        if (mSDCardID != newCardID) {
            // SD card has changed...
            LogUtils.d(TAG, "onSDInserted: card has been changed!!");
            mSDCardID = newCardID;
        } else if (mDeleteUponSDInsertion) {
            // Previous deletion has failed
            // and card is the same one
            mDeleteUponSDInsertion = false;
            //FMRadioLogUtils.d(LOGTAG, "onSDInserted: trying to delete previous unfinished recording file...");
            File oldFile = new File(mOldFilePath);
            boolean isDeleted = oldFile.delete();
            LogUtils.d(TAG, "onSDInserted: delete old recording file=" + isDeleted);
            mRecordFile = null;
            mOldFilePath = null;
        } // else do noting
        LogUtils.d(TAG, "<< onSDInserted");
    }*/
    
    /**
     * reset FM recorder  
     */
    public void resetRecorder() {
        if (mRecorder != null) {
            mRecorder.release();
            mRecorder = null;
        }
        if (mPlayer != null) {
            mPlayer.release();
            mPlayer = null;
        }
        mRecordFile = null;
        mRecordStartTime = 0;
        mRecordTime = 0;
        mInternalState = STATE_IDLE;
    }
    
    /**
     * according storage path to judge whether have sufficient storage space
     * @param storagePath the storage directory
     * @return true or false to indicate whether have sufficient storage space
     */
    private boolean isHaveAvailableStorage(String storagePath) {
        boolean ret = false;
        String sdcard = storagePath;
        try {
            StatFs fs = new StatFs(sdcard);
            long blocks = fs.getAvailableBlocks();
            long blockSize = fs.getBlockSize();
            long spaceLeft = blocks * blockSize;
            LogUtils.d(TAG, "checkRemainingStorage: available space=" + spaceLeft);
            ret = spaceLeft > FMRecorder.LOW_SPACE_THRESHOLD ? true : false;
        } catch (IllegalArgumentException e) {//ALPS01259807
            LogUtils.e(TAG, "sdcard may be unmounted:" + sdcard);
        }
        return ret;
    }
    
    /**
     * notify error message to the callback
     * @param error FM recorder error type
     */
    private void setError(int error) {
        LogUtils.e(TAG, "setError: " + error);
        if (null != mStateListener) {
            mStateListener.onRecorderError(error);
        }
    }
    
    /**
     * notify FM recorder state message to the callback
     * @param state FM recorder current state
     */
    private void setState(int state) {
        mInternalState = state;
        if (null != mStateListener) {
            mStateListener.onRecorderStateChanged(state);
        }
    }
    
    /**
     * save recording file info to database
     * @param context application context
     */
    private void addCurrentRecordingToDb(final Context context) {
        LogUtils.v(TAG, ">> addCurrentRecordingToDb");
        if (null == mRecordFile || !mRecordFile.exists()) {
            LogUtils.e(TAG, "<< addCurrentRecordingToDb: file does not exists");
            return;
        }
        long curTime = System.currentTimeMillis();
        long modDate = mRecordFile.lastModified();
        Date date = new Date(curTime);

        java.text.DateFormat dateFormatter = DateFormat.getDateFormat(context);
        java.text.DateFormat timeFormatter = DateFormat.getTimeFormat(context);
        String title = getRecordingName();
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append(FM_RECORD_FOLDER).append(" ").append(dateFormatter.format(date)).append(" ").append(
                timeFormatter.format(date));
        String artist = stringBuilder.toString();

        final int size = 9;
        ContentValues cv = new ContentValues(size);
        cv.put(MediaStore.Audio.Media.IS_MUSIC, 0);
        cv.put(MediaStore.Audio.Media.TITLE, title);
        cv.put(MediaStore.Audio.Media.DATA, mRecordFile.getAbsolutePath());
        final int oneSecond = 1000;
        cv.put(MediaStore.Audio.Media.DATE_ADDED, (int) (curTime / oneSecond));
        cv.put(MediaStore.Audio.Media.DATE_MODIFIED, (int) (modDate / oneSecond));
        cv.put(MediaStore.Audio.Media.MIME_TYPE, RECORDING_FILE_TYPE);
        cv.put(MediaStore.Audio.Media.ARTIST, artist);
        cv.put(MediaStore.Audio.Media.ALBUM, RECORDING_FILE_SOURCE);
        cv.put(MediaStore.Audio.Media.DURATION, mRecordTime);

        int recordingId = addToMediaDB(context, cv);
        if (-1 == recordingId) {
            // insert failed
            return;
        }
        int playlistId = getPlaylistId(context);
        if (-1 == playlistId) {
            // play list not exist, create FM Recording play list
            playlistId = createPlaylist(context);
        }
        // insert item to FM recording play list
        addToPlaylist(context, playlistId, recordingId);
        // Notify applications listening to the scanner events that a recorded audio file just created.
        // Connect to mediascanner to update duration
        MediaScannerConnection.scanFile(context, new String[] {mRecordFile.getAbsolutePath()}, null, null);
    }
    
    private int getPlaylistId(final Context context) {
        Cursor playlistCursor = context.getContentResolver().query(
                MediaStore.Audio.Playlists.getContentUri("external"),
                new String[] {MediaStore.Audio.Playlists._ID},
                MediaStore.Audio.Playlists.DATA + "=?", 
                new String[] {FMRadioUtils.getPlaylistPath(context) + RECORDING_FILE_SOURCE},
                null);
        int playlistId = -1;
        if (null != playlistCursor) {
            try {
                if (playlistCursor.moveToFirst()) {
                    playlistId = playlistCursor.getInt(0);
                }
            } finally {
                playlistCursor.close();
            }
        }
        return playlistId;
    }
    
    private int createPlaylist(final Context context) {
        final int size = 1;
        ContentValues cv = new ContentValues(size);
        cv.put(MediaStore.Audio.Playlists.NAME, RECORDING_FILE_SOURCE);
        LogUtils.d(TAG, "addToPlaylist: insert playlist");
        Uri newPlaylistUri = context.getContentResolver().insert(MediaStore.Audio.Playlists.getContentUri("external"), cv);
        return Integer.valueOf(newPlaylistUri.getLastPathSegment());
    }
    
    private int addToMediaDB(final Context context, final ContentValues cv) {        
        Uri insertResult = null;
        int recordingId = -1;
        
        Cursor existItems = context.getContentResolver().query(
            MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
            new String[] {"_id"},
            MediaStore.Audio.Media.DATA + "=?",
            new String[] {mRecordFile.getAbsolutePath()},
            null);
        
        if (null != existItems) {
            try {
                
                if (existItems.moveToFirst()) {
                    // there's already a file with the same name in DB, update the item.
                    recordingId = existItems.getInt(0);
                    LogUtils.d(TAG, "existing items update recording id" + recordingId);
                    int rowCnt = context.getContentResolver().update(
                            MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                            cv,
                            MediaStore.Audio.Media.DATA + "=?",
                            new String[] {mRecordFile.getAbsolutePath()});
                    
                } else {
                    // not exist file has a same name in DB, insert the item.
                    LogUtils.d(TAG, "addToMediaDB: insert data");
                    insertResult = context.getContentResolver().insert(
                            MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, cv);
                    if (null != insertResult) {
                        recordingId = Integer.valueOf(insertResult.getLastPathSegment());
                    }
                }
            } finally {
                existItems.close();
                existItems = null;
            }
        }
        return recordingId;
    }
    
    private void addToPlaylist(final Context context, final int playlistId, final int recordingId) {
        if (-1 == playlistId) {
            return;
        }
        Uri membersUri = MediaStore.Audio.Playlists.Members.getContentUri("external", playlistId);
        Cursor itemInMembers = context.getContentResolver().query(membersUri,
                new String[] { MediaStore.Audio.Playlists.Members.AUDIO_ID },
                MediaStore.Audio.Playlists.Members.AUDIO_ID + "=?",
                new String[] { String.valueOf(recordingId) }, null);
        if (null != itemInMembers) {
            try {
                if (itemInMembers.moveToFirst()) {
                    // Item already in playlist member,
                    // no further actions (modify play_order is difficult,
                    // since a same audio_id can appear multiple times in one playlist
                    // with different play_order)
                    LogUtils.d(TAG, "new item already in playlists.members table");
                    LogUtils.d(TAG, "<< addCurrentRecordingToMediaDB");
                    return;
                }
            } finally {
                itemInMembers.close();
                itemInMembers = null;
            }
        }
    LogUtils.d(TAG, "addCurrentRecordingToMediaDB: query members");
    Cursor membersCursor = context.getContentResolver().query(
            membersUri,
            new String[] {"count(*)"},
            null,
            null,
            null);
    
    if (null != membersCursor) {
        try {
            if (membersCursor.moveToFirst()) {
                // insert item to playlist
                int base = membersCursor.getInt(0);
                final int size = 2;
                ContentValues cv = new ContentValues(size);
                cv.put(MediaStore.Audio.Playlists.Members.PLAY_ORDER, Integer.valueOf(base));
                cv.put(MediaStore.Audio.Playlists.Members.AUDIO_ID, recordingId);
                LogUtils.d(TAG, "addCurrentRecordingToMediaDB: insert to members");
                context.getContentResolver().insert(membersUri, cv);
            }
        } finally {
            membersCursor.close();
            membersCursor = null;
        }
    }
    }

    private void stopRecorder() {
        synchronized (this) {
            boolean flag = true;
            if (null != mRecorder) {
                try {
                    mRecorder.stop();
                    LogUtils.d(TAG, "stopRecorder: stop");
                } catch (IllegalStateException ex) {
                    LogUtils.e(TAG, "IllegalStateException ocurr" + ex);
                    setError(ERROR_RECORDER_INTERNAL);
                    return;
                } finally {
                    mRecorder.release();
                    LogUtils.d(TAG, "stopRecorder: release");
                    mRecorder = null;
                }
            }
        }
    }

    private void stopPlayer() {
        synchronized (this) {
            if (null != mPlayer) {
                try {
                    mPlayer.stop();
                } catch (IllegalStateException e) {
                    LogUtils.e(TAG, "IllegalStateException while discard recording!");
                    setError(ERROR_PLAYER_INTERNAL);
                    return;
                } finally {
                    mPlayer.release();
                    mPlayer = null;
                }
            }
        }
    }
}
