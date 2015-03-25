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

package com.mediatek.audioprofile;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Random;

import android.content.BroadcastReceiver;
import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.AssetFileDescriptor;
import android.database.ContentObserver;
import android.database.Cursor;
import android.media.AudioManager;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.storage.StorageManager;
import android.os.UserHandle;
import android.os.UserManager;
import android.provider.MediaStore;
import android.provider.Settings;
import android.provider.Telephony.SIMInfo;
import android.util.Log;

import com.mediatek.audioprofile.AudioProfileManager.ProfileSettings;
import com.mediatek.audioprofile.AudioProfileManager.Scenario;
import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.audioprofile.AudioProfileListener;
import com.mediatek.common.audioprofile.IAudioProfileExtension;
import com.mediatek.common.audioprofile.IAudioProfileListener;
import com.mediatek.common.audioprofile.IAudioProfileService;
import com.mediatek.common.featureoption.FeatureOption;

public class AudioProfileService extends IAudioProfileService.Stub {
    private static final String TAG = "AudioProfileService";

    // Message to handle AudioProfile changed(first persist profile settings
    private static final int MSG_PERSIST_VOICECALL_RINGTONE_TO_SYSTEM          = 1;
    private static final int MSG_PERSIST_NOTIFICATION_RINGTONE_TO_SYSTEM       = 2;
    private static final int MSG_PERSIST_VIDEOCALL_RINGTONE_TO_SYSTEM          = 3;
    private static final int MSG_PERSIST_DTMF_TONE_TO_SYSTEM                   = 4;
    private static final int MSG_PERSIST_SOUND_EFFECT_TO_SYSTEM                = 5;
    private static final int MSG_PERSIST_LOCKSCREEN_SOUND_TO_SYSTEM            = 6;
    private static final int MSG_PERSIST_HAPTIC_FEEDBACK_TO_SYSTEM             = 7;
    private static final int MSG_PERSIST_RINGER_VOLUME_TO_DATABASE             = 8;
    private static final int MSG_PERSIST_NOTIFICATION_VOLUME_TO_DATABASE       = 9;
    private static final int MSG_PERSIST_ALARM_VOLUME_TO_DATABASE              = 10;
    private static final int MSG_PERSIST_VOICECALL_RINGTONE_TO_DATABASE        = 11;
    private static final int MSG_PERSIST_NOTIFICATION_RINGTONE_TO_DATABASE     = 12;
    private static final int MSG_PERSIST_VIDEOCALL_RINGTONE_TO_DATABASE        = 13;
    private static final int MSG_PERSIST_VIBRATION_TO_DATABASE                 = 14;
    private static final int MSG_PERSIST_DTMF_TONE_TO_DATABASE                 = 15;
    private static final int MSG_PERSIST_SOUND_EFFECT_TO_DATABASE              = 16;
    private static final int MSG_PERSIST_LOCKSCREEN_SOUND_TO_DATABASE          = 17;
    private static final int MSG_PERSIST_HAPTIC_FEEDBACK_TO_DATABASE           = 18;
    private static final int MSG_PERSIST_PROFILE_NAME_TO_DATABASE              = 19;
    private static final int MSG_PERSIST_VALUES_TO_SYSTEM_BY_BATCH             = 20;

    private static final int MSG_DELAY_SET_VIBRATE_AVOID_CTS_FAIL              = 21;

    private static final int MSG_PERSIST_SIPCALL_RINGTONE_TO_SYSTEM            = 22;
    private static final int MSG_PERSIST_SIPCALL_RINGTONE_TO_DATABASE          = 23;

    // mDefaultRingtone index
    private static final int DEFAULT_RINGTONE_TYPE_CONUT = 4;
    private static final int DEFAULT_RINGER_INDEX = ProfileSettings.ringer_stream.ordinal();
    private static final int DEFAULT_NOTIFICATION_INDEX = ProfileSettings.notification_stream.ordinal();
    private static final int DEFAULT_VIDEOCALL_INDEX = ProfileSettings.videocall_Stream.ordinal();
    private static final int DEFAULT_SIPCALL_INDEX = ProfileSettings.sipcall_Stream.ordinal();

    // Avoid CTS fail we should delay to set vibration
    private static final long DELAY_TIME_AVOID_CTS_FAIL = 20000;
    // Add for edit outdoor new feature
    private static final boolean IS_SUPPORT_OUTDOOR_EDITABLE = false;


    // Add to identify silent notification
    public static final Uri SILENT_NOTIFICATION_URI = Uri.parse("com.mediatek.uri.silent_notificaton");

    // Because hashMap MINIMUM_CAPACITY is 4, we set our bundle default size to be the same.
    private static final int BUNDLE_DEFAULT_SIZE = 4;

    private final Context mContext;
    private final ContentResolver mContentResolver;
    private final AudioManager mAudioManager;
    private final StorageManager mStorageManager;
    private Handler mAudioProfileHandler;
    private String mActiveProfileKey;
    private String mLastActiveProfileKey;
    private final ArrayList<Record> mRecords = new ArrayList<Record>();
    private final List<String> mKeys = new ArrayList<String>(AudioProfileManager.MAX_PROFILES_COUNT);
    /** The four predefined profile keys: general, silent, meeting and outdoor */
    private final List<String> mPredefinedKeys = new ArrayList<String>(AudioProfileManager.PREDEFINED_PROFILES_COUNT);
    /** Four type default ringtone: voice call, notification, video call and SIP call*/
    private final List<Uri> mDefaultRingtone = new ArrayList<Uri>(DEFAULT_RINGTONE_TYPE_CONUT);
    // Delay set vibrate to avoid CTS fail
    private boolean mDelaySetVibrate = false;
    // Should override system because of outdoor experience
    private boolean mShouldOverrideSystem = false;
    // When default ringtone has not been write by mediascanner, we should get it again after
    // scan finish.
    private boolean mShouldGetDefaultRingtoneAfterScanFinish = false;
    // Use to restore last vibrate setting type to avoid CTS fail
    private int mLastVibrateType = -1;
    private boolean mResetFlag = false;

    // M: for multi user {
    //Current Uid
    private int mUserId = UserHandle.USER_OWNER;
    //Delete profile basic name
    private String mDeleteProfileName = "audio_delete_items";
    //Delete profile count name
    private String mDeleteCountName = "audio_delete_items_count";
    //Delete profile count
    private int mDeleteCount = 0;
    //}

    private ArrayList<String> mDeleteProfileTmp = new ArrayList<String>();

    /**
     * The profile states save all the persisted settings with all profiles. Init it when service
     * first start and maintain this hash map when profiles settings have been changed.
     */
    private final HashMap<String, AudioProfileState> mProfileStates = new HashMap<String, AudioProfileState>(
            AudioProfileManager.MAX_PROFILES_COUNT);

    /**
     * The custom profile's names.
     */
    private final HashMap<String, String> mCustomProfileName =
        new HashMap<String, String>(AudioProfileManager.MAX_PROFILES_COUNT - AudioProfileManager.PREDEFINED_PROFILES_COUNT);

    /**
     * The active custom profile has been deleted, if it is true, we should override system with all
     * persist values.
     */
    private boolean mIsLastCustomActiveProfileDeleted = true;

    private int mRingerMode = -1;

    /**
     * Record the status that user whether change the last active profile's settings (three type
     * volume and three type ringtone), if it is true, we should synchronize these changes to system
     * when we set last active profile to be active one.
     */
    private final ArrayList<Boolean> mShouldSyncToSystem = new ArrayList<Boolean>();

    private long mSimId = -1;
    private int mSingleSIM = -1;

    private final IAudioProfileExtension mExt;

    /**
     * RingerMode change listener
     */
    private final AudioProfileListener mRingerModeListener = new AudioProfileListener() {
        @Override
        public void onRingerModeChanged(int newRingerMode) {
            int ringerMode = mAudioManager.getRingerMode();
            if (ringerMode != newRingerMode) {
                Log.d(TAG, "onRingerModeChanged: ringermode is not latest: new = " + newRingerMode + ", latest = "
                        + ringerMode);
            }
            if (ringerMode == mRingerMode) {
                Log.d(TAG, "onRingerModeChanged with same ringerMode = " + ringerMode);
            } else {
                mRingerMode = ringerMode;

                // Only when ringermode has been change by other app, we should change profile.
                if (mExt.onRingerModeChanged(ringerMode)) {
                    return;
                }

                Scenario activeScenario = AudioProfileManager.getScenario(mActiveProfileKey);
                Log.d(TAG, "onRingerModeChanged: ringermode changed by other app, change profile! ringerMode = "
                        + ringerMode);

                switch (ringerMode) {
                    case AudioManager.RINGER_MODE_SILENT:
                        // RingerMode has been changed to be silent, if profile is not
                        // silent, set active to silent.
                        if (!Scenario.SILENT.equals(activeScenario)) {
                            Log.v(TAG, "RingerMode change to SILENT, change profile to silent");
                            setActiveProfile(mPredefinedKeys.get(Scenario.SILENT.ordinal()), false);
                        }
                        break;

                    case AudioManager.RINGER_MODE_VIBRATE:
                        // RingerMode has been changed to be vibrate, if profile is not
                        // meeting, set active to meeting.
                        if (!Scenario.MEETING.equals(activeScenario)) {
                            Log.v(TAG, "RingerMode change to VIBRATE, change profile to meeting!");
                            setActiveProfile(mPredefinedKeys.get(Scenario.MEETING.ordinal()), false);
                        }
                        break;

                    case AudioManager.RINGER_MODE_NORMAL:
                        // RingerMode has been changed to be normal, if profile is silent
                        // or meeting, set active to last active profile.
                        if (Scenario.SILENT.equals(activeScenario) || Scenario.MEETING.equals(activeScenario)) {
                            Log.v(TAG, "RingerMode change to NORMAL, change profile to last active profile!");
                            int systemVolume = mAudioManager.getStreamVolume(AudioProfileManager.STREAM_RING);
                            syncRingerVolumeToProfile(mLastActiveProfileKey, systemVolume);
                            setActiveProfile(mLastActiveProfileKey, false);
                        }
                        break;

                    default:
                        Log.e(TAG, "undefined RingerMode!");
                        break;
                }
            }
        }
    };
    
    /** 
     * Ringer volume change listener
     */
    private final AudioProfileListener mRingerVolumeListener = new AudioProfileListener() {
        @Override
        public void onRingerVolumeChanged(int oldVolume, int newVolume, String extra) {
            if (oldVolume == newVolume) {
                Log.w(TAG, "onRingerVolumeChanged with Volume don't change, do nothing!");
                return;
            }

            // Sysn the volume change to active profile volume
            if (mExt.onRingerVolumeChanged(oldVolume, newVolume, extra)) {
                return;
            }
            AudioProfileState activeProfileState = mProfileStates.get(mActiveProfileKey);
            Scenario activeScenario = AudioProfileManager.getScenario(mActiveProfileKey);
            switch (activeScenario) {
                case OUTDOOR:
                    final int minVolume = 0;
                    final int maxVolume = AudioProfileManager.DEFAULT_MAX_VOLUME_OUTDOOR;
                    if ((newVolume > minVolume) && (newVolume != maxVolume) && mExt.shouldSyncGeneralRingtoneToOutdoor()) {
                        // If the active profile is outdoor and volume has been changed
                        // (must > 0),we should change it to general profile in non-CMCC project
                        String generalProfilekey = mPredefinedKeys.get(Scenario.GENERAL.ordinal());
                        syncRingerVolumeToProfile(generalProfilekey, newVolume);
                        setActiveProfile(generalProfilekey);
                        Log.d(TAG, "onRingerVolumeChanged in outdoor profile, so change to general profile!");
                        break;
                    }

                case GENERAL:
                case CUSTOM:
                    if (activeProfileState.mRingerVolume != newVolume) {
                        // If active profile is general or custom and volume changed,
                        // sync to active profile.
                        notifyRingerVolumeChanged(oldVolume, newVolume, mActiveProfileKey);
                        syncRingerVolumeToProfile(mActiveProfileKey, newVolume);
                        Log.d(TAG, "onRingerVolumeChanged: ringer volume changed, "
                                + "sysn to active profile except silent, meeting and outdoor!");
                    }
                    break;

                default:
                    // This is a special case: when system volume change from non-zero to zero
                    // AudioManager will first change RingerMode from normal to vibrate,
                    // and then change volume to 0. but we want first change the profile's
                    // volume to be 0, and then change profile to match new RingerMode.So at
                    // this case we should sync this volume change to last active profile.

                    if (oldVolume > 0 && newVolume == 0) {
                        notifyRingerVolumeChanged(oldVolume, newVolume, mLastActiveProfileKey);
                        syncRingerVolumeToProfile(mLastActiveProfileKey, newVolume);
                        Log.d(TAG, "onRingerVolumeChanged: sync volume 1->0 to last active "
                                + "profile when it cause ringemode change!");
                    }
                    break;
            }
        }
    };

    /** 
     * Vibrate setting change listener, when current vibrate type is same as last one, it mean CTS is running,
     * so we should delay to persist vibrate setting to system to avoid CTS fail.
     */
    private final AudioProfileListener mVibrateSettingListener = new AudioProfileListener() {
        @Override
        public void onVibrateSettingChanged(int vibrateType, int vibrateSetting) {
            Log.d(TAG, "onVibrateSettingChanged: current vibrateType = " + vibrateType
                    + ", mLastVibrateType = " + mLastVibrateType);
            if (vibrateType == mLastVibrateType) {
                mDelaySetVibrate = true;
                Log.w(TAG, "onVibrateSettingChanged: current vibrate type is same as last one, delay set vibrate!");
            }
            mLastVibrateType = vibrateType;
        }
    };

    /** 
     * Ringtone observer, when ringtone changed from other app, synchronize to active profile.
     */
    private final ContentObserver mRingtoneObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            synchronized (mActiveProfileKey) {
                if (mExt.onRingtoneChange(selfChange)) {
                    return;
                }

                Scenario activeScenario = AudioProfileManager.getScenario(mActiveProfileKey);
                AudioProfileState activeState = getProfileState(mActiveProfileKey);
                String uriString = Settings.System.getString(mContentResolver, Settings.System.RINGTONE);
                Uri systemUri = (uriString == null ? null : Uri.parse(uriString));

                final boolean isPassiveChange = ((activeState.mRingerStream == null && systemUri != null)
                        || (activeState.mRingerStream != null && !activeState.mRingerStream.equals(systemUri)));
                Log.d(TAG, "Ringtone changed, mResetFlag = "  + mResetFlag + "  isPassiveChange = " + isPassiveChange + " uriString = " + uriString);
                switch (activeScenario) {
                    case GENERAL:
                    case OUTDOOR:
                        if (mExt.shouldSyncGeneralRingtoneToOutdoor()) {
                            // If ringtone has been changed and the active profile is general
                            // or outdoor profile, synchronize the current system ringtone
                            // to both profiles.
                            if (isPassiveChange && (!mResetFlag)) {
                                String generalKey = mPredefinedKeys.get(Scenario.GENERAL.ordinal());
                                String outdoorKey = mPredefinedKeys.get(Scenario.OUTDOOR.ordinal());
                                getProfileState(generalKey, mSimId).mRingerStream = systemUri;
                                getProfileState(outdoorKey, mSimId).mRingerStream = systemUri;
                                persistRingtoneUriToDatabase(generalKey, AudioProfileManager.TYPE_RINGTONE, mSimId, systemUri);
                                persistRingtoneUriToDatabase(outdoorKey, AudioProfileManager.TYPE_RINGTONE, mSimId, systemUri);
                                Log.d(TAG, "Ringtone changed by other app in non-silent "
                                        + "profile, synchronize to active profile: new uri = " + systemUri);
                            } else {
                                Log.d(TAG, "Ringtone changed by itself, do nothing!");
                            }
                            break;
                        }

                    case CUSTOM:
                        // If ringtone has been changed and the active profile is custom
                        // profile, synchronize the current system ringtone to active profile.
                        if (isPassiveChange && (!mResetFlag)) {
                            activeState.mRingerStream = systemUri;
                            persistRingtoneUriToDatabase(mActiveProfileKey, AudioProfileManager.TYPE_RINGTONE, mSimId, systemUri);
                            Log.d(TAG, "Ringtone changed by other app in non-silent profile,"
                                    + " synchronize to active profile: new uri = " + systemUri);
                        } else {
                            Log.d(TAG, "Ringtone changed by itself, do nothing!");
                        }
                        break;

                    default:
                        setShouldSyncToSystemFlag(AudioProfileManager.TYPE_RINGTONE, true);
                        Log.d(TAG, "Ringtone changed in silent profile, sync to system if switch to last active profile.");
                        break;
                }
            }
        }
    };
    
    /** 
     * Notification observer, when Notification changed by other app, synchronize to active profile.
     */
    private final ContentObserver mNotificationObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            synchronized (mActiveProfileKey) {
                if (mExt.onNotificationChange(selfChange)) {
                    return;
                }
                Scenario activeScenario = AudioProfileManager.getScenario(mActiveProfileKey);
                AudioProfileState activeState = getProfileState(mActiveProfileKey);
                String uriString = Settings.System.getString(mContentResolver, Settings.System.NOTIFICATION_SOUND);
                Uri systemUri = (uriString == null ? SILENT_NOTIFICATION_URI : Uri.parse(uriString));

                final boolean isPassiveChange = ((activeState.mNotificationStream == null && systemUri != null)
                        || (activeState.mNotificationStream != null && !activeState.mNotificationStream.equals(systemUri)));
                switch (activeScenario) {
                    case GENERAL:
                    case OUTDOOR:
                        if (mExt.shouldSyncGeneralRingtoneToOutdoor()) {
                            // If notification has been changed and the active profile is
                            // general or outdoor profile, synchronize the current system
                            // notification to both profiles.
                            if (isPassiveChange) {
                                String generalKey = mPredefinedKeys.get(Scenario.GENERAL.ordinal());
                                String outdoorKey = mPredefinedKeys.get(Scenario.OUTDOOR.ordinal());
                                getProfileState(generalKey).mNotificationStream = systemUri;
                                getProfileState(outdoorKey).mNotificationStream = systemUri;
                                persistRingtoneUriToDatabase(generalKey, AudioProfileManager.TYPE_NOTIFICATION, systemUri);
                                persistRingtoneUriToDatabase(outdoorKey, AudioProfileManager.TYPE_NOTIFICATION, systemUri);
                                Log.d(TAG, "Notification changed by other app in non-silent"
                                        + " profile, synchronize to active profile: new uri = " + systemUri);
                            } else {
                                Log.d(TAG, "Notification changed by itself, do nothing!");
                            }
                            break;
                        }

                    case CUSTOM:
                        // If notification has been changed and the active profile is custom
                        // profile,synchronize the current system notification to active profile
                        if (isPassiveChange) {
                            activeState.mNotificationStream = systemUri;
                            persistRingtoneUriToDatabase(mActiveProfileKey, AudioProfileManager.TYPE_NOTIFICATION, 
                                    systemUri);
                            Log.d(TAG, "Notification changed by other app in non-silent"
                                    + " profile, synchronize to active profile: new uri = " + systemUri);
                        } else {
                            Log.d(TAG, "Notification changed by itself, do nothing!");
                        }
                        break;

                    default:
                        setShouldSyncToSystemFlag(AudioProfileManager.TYPE_NOTIFICATION, true);
                        Log.d(TAG, "Notification changed in silent profile,sync to system if switch to last active profile");
                        break;
                }
            }
        }
    };
    
    /**
     * When first boot completed, we should update default rongtone from null to really value after
     * media scanner finished, because MedioScanner start after AudioProfileService. 
     */
    private final BroadcastReceiver mUpgradeReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "mUpgradeReceiver start update profile: action = " + action);
            

            if (Intent.ACTION_MEDIA_SCANNER_FINISHED.equals(action)) {
                // Get default ringtone if need after scan finish because maybe the default ringtone get before is null.
                if (mShouldGetDefaultRingtoneAfterScanFinish) {
                    // Get default ringtones from database again
                    readDefaultRingtones(); 
                    synchronized (mProfileStates) {
                        for (String profileKey : mKeys) {
                            AudioProfileState profileState = mProfileStates.get(profileKey);
                            // Voice call
                            if (null == profileState.mRingerStream) {
                                profileState.mRingerStream = mDefaultRingtone.get(DEFAULT_RINGER_INDEX);
                            }
                            // Notification
                            if (null == profileState.mNotificationStream) {
                                profileState.mNotificationStream =
                                        mDefaultRingtone.get(DEFAULT_NOTIFICATION_INDEX);
                            }
                            // Video call
                            if (null == profileState.mVideoCallStream) {
                                profileState.mVideoCallStream =
                                        mDefaultRingtone.get(DEFAULT_VIDEOCALL_INDEX);
                            }
                            //SIP call
                            if (null == profileState.mSIPCallStream) {
                                profileState.mSIPCallStream =
                                        mDefaultRingtone.get(DEFAULT_SIPCALL_INDEX);
                            }
                        }
                    }
                    // If after scan finish the default ringtone uri is still null, we need read defaultringtone again when
                    // MediaScanner scan finish next time.
                    mShouldGetDefaultRingtoneAfterScanFinish =
                        (mDefaultRingtone.get(DEFAULT_RINGER_INDEX) == null);
                }

                String[] volumePath = mStorageManager.getVolumePaths();

                ArrayList<String> allKeys = new ArrayList<String>();
                allKeys.addAll(mKeys);
                // Remove silent
                allKeys.remove(AudioProfileManager.getProfileKey(Scenario.SILENT));
                for (String profileKey : allKeys) {
                    Log.i(TAG, "mUpgradeReceive deal case: profileKey = " + profileKey);
                    Uri newUri = null;
                    AudioProfileState profileState = getProfileState(profileKey);
                    // Update ringtone uri
                    if (FeatureOption.MTK_MULTISIM_RINGTONE_SUPPORT) {
                        // add to get  selected SIM id                     
                        List<SIMInfo> simList = SIMInfo.getInsertedSIMList(mContext);
                        int simNum = simList.size();
                        Log.d(TAG, "simList.size() == " + simNum);                        
                        long simId = -1;
                        for (int i = 0; i < simNum; i++) {
                            simId = simList.get(i).mSimId;
                            // Update voice call uri with SIM id                             
                            updateRintoneUriWithType(profileKey, AudioProfileManager.TYPE_RINGTONE, volumePath, simId);
                            // Update video call uri with SIM id
                            updateRintoneUriWithType(profileKey, AudioProfileManager.TYPE_VIDEO_CALL, volumePath, simId);
                        }                        
                    }
                    // update ringtone URI with no SIM id
                    // Update voice call uri                    
                    updateRintoneUriWithType(profileKey, AudioProfileManager.TYPE_RINGTONE, volumePath);
                    // Update video call uri
                    updateRintoneUriWithType(profileKey, AudioProfileManager.TYPE_VIDEO_CALL, volumePath);
                    // Update notification uri
                    updateRintoneUriWithType(profileKey, AudioProfileManager.TYPE_NOTIFICATION, volumePath);
                    // Update sip call uri
                    updateRintoneUriWithType(profileKey, AudioProfileManager.TYPE_SIP_CALL, volumePath);

                }
                // Persist new uri to system
                persistRingtoneUriToSystem(AudioProfileManager.TYPE_RINGTONE);
                persistRingtoneUriToSystem(AudioProfileManager.TYPE_VIDEO_CALL);
                persistRingtoneUriToSystem(AudioProfileManager.TYPE_SIP_CALL);
                persistRingtoneUriToSystem(AudioProfileManager.TYPE_NOTIFICATION);
            }
            Log.d(TAG, "mUpgradeReceiver<<<");
        }
    };

    private final BroadcastReceiver mBootCompleteReceiver = new BroadcastReceiver() {
        
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.i(TAG, "mBootCompleteReceiver receive action " + action);
            if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
                // Persist profile settings to system after boot complete. Because some settings
                // will be reset after reboot device(such as vibration, google change AudioService
                // behavior in Jelly bean, it will not save vibrate settings to database, so after
                // reboot vibrate setting will reset to default values)
                Log.d(TAG, "Persist profile settings to system when boot complete!");
                persistValues(false);
                // When current ringermode is RINGER_MODE_NORMAL and the ringer volume is 0, AudioService will restore
                // volume to be default value, so AudioProfileService need to reset the volume to be 0 when boot complete.
                Scenario activeScenario = AudioProfileManager.getScenario(mActiveProfileKey);
                if (mRingerMode == AudioManager.RINGER_MODE_NORMAL
                        && getStreamVolume(mActiveProfileKey, AudioProfileManager.STREAM_RING) == 0) {
                    mAudioManager.setAudioProfileStreamVolume(AudioProfileManager.STREAM_RING, 0, 0);
                    mAudioManager.setAudioProfileStreamVolume(AudioProfileManager.STREAM_NOTIFICATION, 0, 0);
                    Log.d(TAG, "Persist system volume to be 0 if ringermode is normal and volume is 0 when boot complete!");
                }
            }
            context.unregisterReceiver(mBootCompleteReceiver);
            Log.i(TAG, "unregister mBootCompleteReceiver!");
        }
    };
    
    private void updateRintoneUriWithType(String profileKey, int ringtoneType, String[] volumePath, long simId) {
        Log.v(TAG, "updateRintoneUriWithType  " + profileKey + " with ringtoneType  " + ringtoneType + " simId = " + simId);
        // Update ringtone uri with ringtone type
        
        String dataKey = AudioProfileManager.getDataKey(AudioProfileManager.getStreamUriKey(profileKey, ringtoneType, simId));
        Uri newUri = updateRintoneUri(dataKey, profileKey, ringtoneType, volumePath);
        Log.v(TAG, "updateRintoneUriWithType   uri = " + newUri + "  with ringtoneType  " + ringtoneType );
          
        if (null != newUri) {
            ContentValues values = new ContentValues(1);

            AudioProfileState profileState = getProfileState(profileKey);        
            switch (ringtoneType) {
                case AudioProfileManager.TYPE_RINGTONE:
                    profileState.mRingerStream = newUri;
                    values.put(MediaStore.Audio.Media.IS_RINGTONE, "1");
                    break;
                
                case AudioProfileManager.TYPE_NOTIFICATION:
                    profileState.mNotificationStream = newUri;
                    values.put(MediaStore.Audio.Media.IS_NOTIFICATION, "1");
                    break;
                
                case AudioProfileManager.TYPE_VIDEO_CALL:
                    profileState.mVideoCallStream = newUri;
                    values.put(MediaStore.Audio.Media.IS_RINGTONE, "1");
                    break;
                    
                case AudioProfileManager.TYPE_SIP_CALL:
                    values.put(MediaStore.Audio.Media.IS_RINGTONE, "1");                
                    profileState.mSIPCallStream = newUri;
                    break;
                
                default:
                    Log.e(TAG, "getRingtoneUri with unsupport type!");
            }
            
            persistRingtoneUriToDatabase(profileKey, ringtoneType, simId, newUri);
            if (newUri.toString().startsWith(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI.toString())) {
                try {
                    mContentResolver.update(newUri, values, null, null);
                } catch (UnsupportedOperationException ex) {
                    Log.e(TAG, "couldn't set ringtone flag for id " + newUri);
                }
            }
        }
           
    };  
    
    private void updateRintoneUriWithType(String profileKey, int ringtoneType, String[] volumePath) {
        Log.v(TAG, "updateRintoneUriWithType  " + profileKey + " with ringtoneType  " + ringtoneType);
        // Update ringtone uri with ringtone type
        updateRintoneUriWithType(profileKey, ringtoneType, volumePath,  mSingleSIM);
           
    };  
                  
    private Uri updateRintoneUri(String dataKey, String profileKey, int streamType, String[] volumePath) {
        
        Log.v(TAG, "updateRintoneUri :  " + profileKey + " with ringtoneType  " + streamType +  "  -  " + volumePath);
        AudioProfileState profileState = getProfileState(profileKey);
        Uri oldUri = null;        
        
        switch (streamType) {
            case AudioProfileManager.TYPE_RINGTONE:
                oldUri = profileState.mRingerStream;
                break;
            
            case AudioProfileManager.TYPE_NOTIFICATION:
                oldUri = profileState.mNotificationStream;
                break;
            
            case AudioProfileManager.TYPE_VIDEO_CALL:
                oldUri = profileState.mVideoCallStream;
                break;
                
            case AudioProfileManager.TYPE_SIP_CALL:
                oldUri = profileState.mSIPCallStream;
                break;
            
            default:
                Log.e(TAG, "updateRintoneUri with unsupport type!");
                return null;
        }        
        
        if ((null != oldUri) && (oldUri.toString().startsWith(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI.toString()))) {
            Uri newUri = null;
            String uriData = Settings.System.getString(mContentResolver, dataKey);
            if (null == uriData) {
                return null;
            }
            String uriSdPath = (uriData.split("/"))[2];
            int length = volumePath.length;

            // If we can query this ringtone in new database, replace old uri with new one,
            // otherwise set to default ringtone
            for (int i = 0; i < length; i++) {
                String sdPath = volumePath[i].substring(volumePath[i].lastIndexOf("/") + 1);
                String newUriData = uriData.replaceAll(uriSdPath, sdPath);
                Cursor cursor = mContentResolver.query(
                        MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                        new String[] { MediaStore.Audio.Media._ID },
                        MediaStore.Audio.Media.DATA + "=?",
                        new String[] { newUriData },
                        null);

                try {
                    if (null != cursor && cursor.moveToFirst()) {
                        long id = cursor.getLong(0);
                        newUri = ContentUris.withAppendedId(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, id);
                        Log.v(TAG, "Update ringtone uri for " + dataKey + " with new uri: " + newUri);
                        return newUri;
                    }
                } finally {
                    if (null != cursor) {
                        cursor.close();
                        cursor = null;
                    }
                }
            }

            Log.v(TAG, "Update ringtone uri for " + dataKey + " with uri: " + newUri);
            return newUri;
        }
        return null;
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Construction
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /** @hide */
    public AudioProfileService(Context context) {
        Log.v(TAG, "Initial AudioProfileService start");
        mContext = context;
        mContentResolver = mContext.getContentResolver();
        mAudioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
        mStorageManager = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);

        // Get default ringtones from database
        readDefaultRingtones();

        // Get predefined keys and custom keys
        readPredefinedProfileKeys();
        readAllProfileKeys();

        // Initial active profile key and last active profile key, if not exist,
        // use general as defalut.
        String activeProfileKey = Settings.System.getString(mContentResolver, AudioProfileManager.KEY_ACTIVE_PROFILE);
        mActiveProfileKey = (activeProfileKey == null ? mPredefinedKeys.get(Scenario.GENERAL.ordinal()) : activeProfileKey);
        String lastActiveProfileKey = Settings.System.getString(mContentResolver, AudioProfileManager.LAST_ACTIVE_PROFILE);
        mLastActiveProfileKey =
                (lastActiveProfileKey == null ? mPredefinedKeys.get(Scenario.GENERAL.ordinal()) : lastActiveProfileKey);
        mIsLastCustomActiveProfileDeleted =
                Boolean.valueOf(Settings.System.getString(mContentResolver, AudioProfileManager.LAST_ACTIVE_CUSTOM_DELETED));

        createOverrideSystemThread();

        // Get profiles setting from database
        for (String profileKey : mKeys) {
            readPersistedSettings(profileKey);
        }

        // Listen RingerMode and RingerVolume changed
        mRingerMode = mAudioManager.getRingerMode();
        mAudioManager.listenRingerModeAndVolume(mRingerModeListener, AudioProfileListener.LISTEN_RINGERMODE_CHANGED);
        mAudioManager.listenRingerModeAndVolume(mRingerVolumeListener, AudioProfileListener.LISTEN_RINGER_VOLUME_CHANGED);
        // Listen Vibrate setting change to avoid CTS fail
        mAudioManager.listenRingerModeAndVolume(mVibrateSettingListener,
                AudioProfileListener.LISTEN_VIBRATE_SETTING_CHANGED);

        // Observer ringtone and notification changed
        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.RINGTONE), false,
                mRingtoneObserver);
        mContentResolver.registerContentObserver(Settings.System.getUriFor(Settings.System.NOTIFICATION_SOUND), false,
                mNotificationObserver);

        // Initial mShouldSyncToSystem
        readShouldSyncToSystem();

        // Register media scan finish receiver for When first boot a phone, AudioProfileService are
        // initial before media scanner, so we should update the default ringtone when media scanner
        // finish initial database.
        Uri defRingUri = mDefaultRingtone.get(DEFAULT_RINGER_INDEX);
        

        IntentFilter scanerFilter = new IntentFilter();
        scanerFilter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
        scanerFilter.addDataScheme("file");
        mContext.registerReceiver(mUpgradeReceiver, scanerFilter);

        // Register boot complete to persist profile settings to system when device reboot.
        IntentFilter bootFilter = new IntentFilter();
        bootFilter.addAction(Intent.ACTION_BOOT_COMPLETED);
        mContext.registerReceiver(mBootCompleteReceiver, bootFilter);

        if (null == defRingUri) {
            mShouldGetDefaultRingtoneAfterScanFinish = true;
        } else if (null == RingtoneManager.getActualDefaultRingtoneUri(mContext, AudioProfileManager.TYPE_RINGTONE)) {
            // Check actual ringtone, if it is null, set to default ringtone.
            persistRingtoneUriToSystem(AudioProfileManager.TYPE_RINGTONE);
        } else if (null == RingtoneManager.getActualDefaultRingtoneUri(mContext, AudioProfileManager.TYPE_VIDEO_CALL)) {
            persistRingtoneUriToSystem(AudioProfileManager.TYPE_VIDEO_CALL);
        } else if (null == RingtoneManager.getActualDefaultRingtoneUri(mContext, AudioProfileManager.TYPE_SIP_CALL)) {
            persistRingtoneUriToSystem(AudioProfileManager.TYPE_SIP_CALL);
        }

        mExt = MediatekClassFactory.createInstance(IAudioProfileExtension.class);
        // maybe buggy here, passing this reference in constructor
        mExt.init(this, mContext);

        if (mExt.shouldCheckDefaultProfiles()) {
            // Check the four default profiles' settings whether match it's type.
            checkDefaultProfiles();
        }
        Log.v(TAG, "Initial AudioProfileService end");
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Set active profile, add profile, delete profile and reset profiles
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // ezpt pp
    /**
     * Sets the active profile with given profile key.
     * 
     * @param profileKey The key of the profile that set to be active.
     * @param shouldSetRingerMode Whether need to set ringer mode, if ringer mode change by other
     *  appshouldSetRingerMode is false, otherwise it is true.
     */
    public void setActiveProfile(String profileKey, boolean shouldSetRingerMode) {

        String oldProfileKey = getActiveProfileKey();
        String newProfileKey = profileKey;
        Log.d(TAG, "setActiveProfile>>>: oldProfileKey = " + oldProfileKey + ", newProfileKey = " + newProfileKey
                + ", shouldSetRingerMode = " + shouldSetRingerMode);

        // We only set active profile when set to new profile
        if (newProfileKey.equals(oldProfileKey)) {
            Log.w(TAG, "setActiveProfile with same profile key with active profile, do nothing!");
            return;
        }

        synchronized (mActiveProfileKey) {
            setActiveKey(newProfileKey);
            boolean overrideSystem = true;

            IAudioProfileExtension.IActiveProfileChangeInfo apcInfo =
                    mExt.getActiveProfileChangeInfo(shouldSetRingerMode, oldProfileKey, newProfileKey,
                            mIsLastCustomActiveProfileDeleted);

            if (apcInfo == null) {
                Scenario newScenario = AudioProfileManager.getScenario(newProfileKey);// ez
                Scenario oldScenario = AudioProfileManager.getScenario(oldProfileKey);// ez
                // Set RingerMode to match different AudioProfiles to different RingerModes
                int ringerMode = mAudioManager.getRingerMode();// ez
                switch (newScenario) {
                    case SILENT:
                        // Set RingerMode:
                        setRingerModeMatchProfile(shouldSetRingerMode, AudioManager.RINGER_MODE_SILENT);

                        // Persisted settings to override system
                        setOldProfileSettings(oldScenario, oldProfileKey);
                        overrideSystem = false;
                        break;

                    case MEETING:
                        // Set RingerMode:
                        setRingerModeMatchProfile(shouldSetRingerMode, AudioManager.RINGER_MODE_VIBRATE);

                        // Persisted settings to override system
                        setOldProfileSettings(oldScenario, oldProfileKey);
                        overrideSystem = false;
                        break;

                    default:
                        // Set RingerMode:
                        setRingerModeMatchProfile(shouldSetRingerMode, AudioManager.RINGER_MODE_NORMAL);

                        // New plan can set general and custom volume to 0 and ask automatically
                        // change volume to 1 when it is set to be active profile.
                        if (getProfileState(newProfileKey).mRingerVolume == 0) {
                            int volume = mAudioManager.getStreamVolume(AudioProfileManager.STREAM_RING);
                            syncRingerVolumeToProfile(newProfileKey, volume);
                            Log.d(TAG, "setActiveProfile: profile volume is 0, change to " + volume);
                        }
                        // Persisted settings to override system
                        if ((Scenario.SILENT.equals(oldScenario) || Scenario.MEETING.equals(oldScenario))
                                && newProfileKey.equals(mLastActiveProfileKey)) {
                            // If the new profile is same as last active profile when set
                            // profile from silent(or meeting) to other profiles, we should
                            // not overridesystem except the last active profile key has been
                            // reset causedby custom active profile delete.
                            overrideSystem = mIsLastCustomActiveProfileDeleted || mShouldOverrideSystem;

                            // In this case, when user changed the last active profile volumes
                            // and ringtones, we should persist it to system
                            syncVolumeToSystem();
                            syncRingtoneToSystem();
                        }
                        mShouldOverrideSystem = Scenario.OUTDOOR.equals(newScenario);
                        break;
                }
            } else {
                int ringerModeToUpdate = apcInfo.getRingerModeToUpdate();
                if (ringerModeToUpdate != IAudioProfileExtension.IActiveProfileChangeInfo.RINGER_MODE_UPDATE_NONE) {
                    mRingerMode = ringerModeToUpdate;
                    mAudioManager.setRingerMode(ringerModeToUpdate);
                    Log.d(TAG, "CMCC: setActiveProfile: RingerMode now set " + ringerModeToUpdate);
                }

                if (apcInfo.shouldSetLastActiveKey()) {
                    // Persisted settings to override system
                    boolean lastActiveChanged = setLastActiveKey(oldProfileKey);
                    if (lastActiveChanged && mIsLastCustomActiveProfileDeleted) {
                        setLastCustomActiveDeleted(false);
                    }
                }

                if (apcInfo.shouldSyncToSystem()) {
                    // In this case, when user changed the last active
                    // profile volumes
                    // and ringtones, we should persist it to system
                    syncVolumeToSystem();
                    syncRingtoneToSystem();
                }
                overrideSystem = apcInfo.shouldOverrideSystem();
            }

            // Override system
            persistValues(overrideSystem);
            // Notify AudioProfile changed
            notifyAudioProfileChanged();
            // Notify ringer volume changed
            int ringerVolume = getProfileState(newProfileKey).mRingerVolume;
            notifyRingerVolumeChanged(ringerVolume, ringerVolume, newProfileKey);
            Log.d(TAG, "setActiveProfile<<<");
        }
    }

    /**
     * Set old profile to be last active profile and refresh last custom active delete value.
     * 
     * @param oldScenario
     * @param oldProfileKey
     */
    private void setOldProfileSettings(Scenario oldScenario, String oldProfileKey) {
        // Persisted settings to override system
        if (Scenario.GENERAL.equals(oldScenario) || Scenario.CUSTOM.equals(oldScenario)
                || (Scenario.OUTDOOR.equals(oldScenario) && !mExt.shouldSyncGeneralRingtoneToOutdoor())) {
            // If set profile from general or custom to silent or meeting or
            // outdoor(if outdoor editable) set it to be last active key.
            boolean lastActiveChanged = setLastActiveKey(oldProfileKey);
            if (lastActiveChanged && mIsLastCustomActiveProfileDeleted) {
                setLastCustomActiveDeleted(false);
            }
        }
    }

    private void setRingerModeMatchProfile(boolean shouldSetRingerMode, int expectRingerMode) {
        int actualRingerMode = mAudioManager.getRingerMode();
        if (shouldSetRingerMode && (actualRingerMode != expectRingerMode)) {
            mRingerMode = expectRingerMode;
            mAudioManager.setRingerMode(expectRingerMode);
        }
        Log.d(TAG, "setRingerModeMatchProfile: actual = " + actualRingerMode + ", expect = " + expectRingerMode);
    }

    /**
     * set the active profile with given profile key.
     * 
     * @param profileKey
     *            The key of the profile that set to be active.
     */
    public void setActiveProfile(String profileKey) {
        setActiveProfile(profileKey, true);
    }

    private boolean setActiveKey(String profileKey) {
        final long token = Binder.clearCallingIdentity();           

        boolean succeed = Settings.System.putString(mContentResolver, AudioProfileManager.KEY_ACTIVE_PROFILE, profileKey);

        mActiveProfileKey = profileKey;
        Log.d(TAG, "setActiveKey: succeed = " + succeed + ", profileKey = " + profileKey);
         
        Binder.restoreCallingIdentity(token);                        
     
        return succeed;
    }

    private boolean setLastActiveKey(String profileKey) {
        final long token = Binder.clearCallingIdentity();  
        boolean succeed = Settings.System.putString(mContentResolver, AudioProfileManager.LAST_ACTIVE_PROFILE, profileKey);
        mLastActiveProfileKey = profileKey;
        Binder.restoreCallingIdentity(token);   
        // Last active profile key changed, reset mShouldSyncToSystem to default.
        final int sizeOfShouldSyncToSystem = mShouldSyncToSystem.size();
        for (int i = 0; i < sizeOfShouldSyncToSystem; i++) {
            mShouldSyncToSystem.set(i, false);
        }
        Log.d(TAG, "setLastActiveKey: succeed = " + succeed + ", profileKey = " + profileKey);
        return succeed;
    }

    private boolean setLastCustomActiveDeleted(boolean deleted) {
        boolean succeed =
                Settings.System.putString(mContentResolver, AudioProfileManager.LAST_ACTIVE_CUSTOM_DELETED,
                        String.valueOf(deleted));
        mIsLastCustomActiveProfileDeleted = deleted;
        Log.d(TAG, "setCustomActiveDeleted: changed = " + succeed);
        return succeed;
    }

    /**
     * Gets the key of active profile.
     * 
     * @return The key of the active profile.
     * 
     */
    public String getActiveProfileKey() {
        synchronized (mActiveProfileKey) {
            Log.d(TAG, "getActiveProfile: profileKey = " + mActiveProfileKey);
            return mActiveProfileKey;
        }
    }

    /**
     * Gets the key of previous non-silent active profile.
     * 
     * @return The key of last non-silent active profile.
     */
    public String getLastActiveProfileKey() {
        synchronized (mActiveProfileKey) {
            Log.d(TAG, "getLastActiveProfileKey: profileKey = " + mLastActiveProfileKey);
            return mLastActiveProfileKey;
        }
    }

    /**
     * Notify the change of the audio profile.
     */
    private void notifyAudioProfileChanged() {
        if (mActiveProfileKey == null) {
            Log.e(TAG, "notifyAudioProfileChanged falled, because active profile key is null!");
            return;
        }

        if (mRecords.isEmpty()) {
            Log.w(TAG, "notifyAudioProfileChanged falled, because there are no listener!");
            return;
        }

        Log.d(TAG, "notifyAudioProfileChanged: New profile = " + mActiveProfileKey + ", clients = " + mRecords.size());
        synchronized (mRecords) {
            Iterator<Record> iterator = mRecords.iterator();
            while (iterator.hasNext()) {
                Record record = (Record) iterator.next();
                if (record.mEvent == AudioProfileListener.LISTEN_AUDIOPROFILE_CHANGEG) {
                    try {
                        record.mCallback.onAudioProfileChanged(mActiveProfileKey);
                    } catch (RemoteException e) {
                        iterator.remove();
                        Log.e(TAG, "Dead object in notifyAudioProfileChanged,"
                                + " remove listener's callback: record.mBinder = " + record.mBinder + ", clients = "
                                + mRecords.size() + ", exception = " + e);
                    }
                }
            }
        }
    }

    // ezpt pp
    /**
     * Notify the ringer volume chenge of the audio profile.
     */
    public void notifyRingerVolumeChanged(int oldVolume, int newVolume, String profileKey) {
        if (mActiveProfileKey == null) {
            Log.e(TAG, "notifyRingerVolumeChanged falled, because active profile key is null!");
            return;
        }

        if (mRecords.isEmpty()) {
            Log.w(TAG, "notifyRingerVolumeChanged falled, because there are no listener!");
            return;
        }

        Log.d(TAG, "notifyRingerVolumeChanged: oldVolume = " + oldVolume + ", newVolume = " + newVolume + ", profile = "
                + profileKey + ", client = " + mRecords.size());
        synchronized (mRecords) {
            Iterator<Record> iterator = mRecords.iterator();
            while (iterator.hasNext()) {
                Record record = (Record) iterator.next();
                if (record.mEvent == AudioProfileListener.LISTEN_RINGER_VOLUME_CHANGED) {
                    try {
                        record.mCallback.onRingerVolumeChanged(oldVolume, newVolume, profileKey);
                    } catch (RemoteException e) {
                        iterator.remove();
                        Log.e(TAG, "Dead object in notifyAudioProfileChanged,"
                                + " remove listener's callback: record.mBinder = " + record.mBinder + ", clients = "
                                + mRecords.size() + ", exception = " + e);
                    }
                }
            }
        }
    }

    /**
     * Adds a new {@link Scenario#CUSTOM} type profile.
     * 
     * @return The new profile key.
     */
    public String addProfile() {
        if (getProfileCount() >= AudioProfileManager.MAX_PROFILES_COUNT) {
            Log.e(TAG, "addProfile: Number of custom audio profile has reached upper limit!");
            return null;
        }

        // general a custom profile key and init it profile state with general default settings.
        String newKey = genCustomKey();
        AudioProfileState defaultState = AudioProfileManager.getDefaultState(newKey);
        AudioProfileState newProfileState = new AudioProfileState.Builder(newKey)
            .ringtone(mDefaultRingtone.get(DEFAULT_RINGER_INDEX),
                mDefaultRingtone.get(DEFAULT_NOTIFICATION_INDEX),
                mDefaultRingtone.get(DEFAULT_VIDEOCALL_INDEX),
                mDefaultRingtone.get(DEFAULT_SIPCALL_INDEX))
            .volume(defaultState.mRingerVolume, defaultState.mNotificationVolume, defaultState.mAlarmVolume)
            .vibration(defaultState.mVibrationEnabled)
            .dtmfTone(defaultState.mDtmfToneEnabled)
            .soundEffect(defaultState.mSoundEffectEnbled)
            .lockScreenSound(defaultState.mLockScreenSoundEnabled)
            .hapticFeedback(defaultState.mHapticFeedbackEnabled)
            .build();

        // Put the custom profilekey's key to database
        String name = AudioProfileManager.getKey(newKey);
        boolean succeed = Settings.System.putString(mContentResolver, name, newKey);
        // If put the profile key to database successes,
        if (succeed) {
            synchronized (mProfileStates) {
                mKeys.add(newKey);
                mProfileStates.put(newKey, newProfileState);
            }
            Log.d(TAG, "addProfile: key = " + newKey + ", state = " + newProfileState.toString());
        } else {
            Log.e(TAG, "addProfile: Failed!");
        }
        return newKey;
    }

    /**
     * Deletes a {@link Scenario#CUSTOM} type profile.
     * 
     * @param profileKey The key of the profile that to be deleted.
     * @return True if delete succeed, otherwise false.
     */
    public boolean deleteProfile(String profileKey) {
        if (profileKey == null) {
            Log.e(TAG, "deleteProfile: Null key!");
            return false;
        }

        //M: Multi user support {
        if (UserManager.supportsMultipleUsers() && mUserId != UserHandle.USER_OWNER) {
            mDeleteProfileTmp.add(profileKey);
            
            Settings.Global.putString(mContentResolver, mDeleteProfileName+"_"+mDeleteCount, profileKey);
            mDeleteCount ++;
            Settings.Global.putInt(mContentResolver, mDeleteCountName, mDeleteCount);
			
            synchronized (mProfileStates) {
                mKeys.remove(profileKey);
                mProfileStates.remove(profileKey);
                mCustomProfileName.remove(profileKey);
            }
			
            return true;
        }
        //}

        List<String> keyList = AudioProfileManager.getAllKeys(profileKey);
        StringBuilder sb = new StringBuilder();
        sb.append(Settings.System.NAME);
        sb.append(" in (");
        int size = keyList.size();
        for (int i = 0; i < size - 1; i++) {
            sb.append("?,");
        }
        sb.append("?)");
        String where = sb.toString();
        int deleted = mContentResolver.delete(Settings.System.CONTENT_URI, where, keyList.toArray(new String[size]));
        Log.d(TAG, "deleteProfile: where = " + where + ", deleted = " + deleted);
        if (deleted > 0) {
            synchronized (mProfileStates) {
               if (mKeys.contains(profileKey)) {
                    mKeys.remove(profileKey);
               }
               if (mProfileStates.get(profileKey) != null) {
                    mProfileStates.remove(profileKey);
               }
               if (mCustomProfileName.get(profileKey) != null) {
                    mCustomProfileName.remove(profileKey);
               }
            }
            if (profileKey.equals(mLastActiveProfileKey)) {
                Log.d(TAG, "deleteProfile: Custom active deleted and set to default.");
                setLastCustomActiveDeleted(true);
                setLastActiveKey(mPredefinedKeys.get(Scenario.GENERAL.ordinal()));
            }
            Log.d(TAG, "deleteProfile: mKeys = " + mKeys + ", mCustomProfileName = " + mCustomProfileName);
            return true;
        } else {
            Log.e(TAG, "deleteProfile: Failed to delete " + profileKey);
            return false;
        }
    }
    
    /**
     * Reset all profiles.
     */
    public void reset() {
        Log.d(TAG, "reset start!");
        mResetFlag = true; 
        String generalKey = mPredefinedKeys.get(Scenario.GENERAL.ordinal());
        boolean isGeneralActive = isActive(generalKey);
        synchronized (mActiveProfileKey) {
            // First restore general profile to default value and set general to be active profile,
            // if the active profile is general, just persist default value to system, otherwise 
            // set general to be active profile.

            restoreToDefaultValues(generalKey);
            if (isGeneralActive) {
                persistValues(true);
            } else {
                setActiveKey(generalKey);
                int ringerMode = mAudioManager.getRingerMode();
                if (ringerMode != AudioManager.RINGER_MODE_NORMAL) {
                    mAudioManager.setRingerMode(AudioManager.RINGER_MODE_NORMAL);
                }
                persistValues(true);
                notifyAudioProfileChanged();
            }
            Log.d(TAG, "reset: profileKey = " + generalKey + ", state = "
                    + mProfileStates.get(generalKey));

            // Second restore other three predefine profile to default and delete custom 
            // profile if exist.
            List<String> allKeys = new ArrayList<String>();
            allKeys.addAll(mKeys);
            allKeys.remove(generalKey);
            for (String profileKey : allKeys) {
                Scenario scenaria = AudioProfileManager.getScenario(profileKey);
                if (Scenario.CUSTOM.equals(scenaria)) {
                    deleteProfile(profileKey);
                } else {
                    restoreToDefaultValues(profileKey);
                }
                Log.d(TAG, "reset: profileKey = " + profileKey + ", state = "
                        + mProfileStates.get(profileKey));
            }
            Log.d(TAG, "reset end!");
        }
        mResetFlag = false; 
    }
    
    /**
     * Generates a new unique key for a custom profile.
     * 
     * @return The newly generated unique key for custom profile.
     */
    private String genCustomKey() {
        int maxCustom = AudioProfileManager.MAX_PROFILES_COUNT - AudioProfileManager.PREDEFINED_PROFILES_COUNT;
        Random rand = new Random(System.currentTimeMillis());
        String key = null;
        do {
            int customNo = rand.nextInt() % maxCustom;
            customNo = Math.abs(customNo);
            key = AudioProfileManager.PROFILE_PREFIX 
                    + Scenario.CUSTOM.toString().toLowerCase() 
                    + "_" + String.valueOf(customNo);
        } while (mKeys.contains(key));
        Log.v(TAG, "genCustomKey: newKey = " + key);
        return key;
    }
    /**
     * Checks out whether the name existed.
     * 
     * @param name The name to be checked.
     * @return True if the specified name had existed or if the name 
     *         is null or empty, false otherwise.
     */
    public boolean isNameExist(String name) {
        boolean isExisted = mCustomProfileName.containsValue(name);
        Log.d(TAG, "isNameExist: name = " + name + ", isExisted = " + isExisted 
                + ", mCustomProfileName = " + mCustomProfileName);
        return isExisted;
    }
    
    /**
     * Gets the number of current existing profiles. 
     * Include the predefined and custom ones.
     * 
     * @return The number of existing profiles.
     */
    public int getProfileCount() {
        synchronized (mProfileStates) {
            int count = mKeys.size();
            Log.d(TAG, "getProfileCount: count = " + count);
            return count;
        }
    }
    
    /**
     * Gets the existed profiles' keys.
     * 
     * @return The existed profiles' keys.
     */
    public List<String> getAllProfileKeys() {
        synchronized (mProfileStates) {
            //M: Multi user support{
            if (UserManager.supportsMultipleUsers() && mUserId == UserHandle.USER_OWNER) {
                deleteCacheProfiles();
            }
            //}
            ArrayList<String> allKeys = new ArrayList<String>(mKeys.size());
            allKeys.addAll(mKeys);
            Log.d(TAG, "getAllProfileKeys: keys = " + allKeys);
            return allKeys;
        }
    }

    /**
     * M: Delete the cache profiles.
     */
    private void deleteCacheProfiles() {
        for (int i=0;i<mDeleteProfileTmp.size();i++) {
            deleteProfile(mDeleteProfileTmp.get(i));
        }
		
        int delCount = 0;
        try {
            delCount = Settings.Global.getInt(mContentResolver, mDeleteCountName);
        } catch (Settings.SettingNotFoundException e) {
            delCount = 0;
        }
		
        if (delCount > 0) {
            for (int i=0;i<delCount;i++) {
                String tmpProfileKey = Settings.Global
                    .getString(mContentResolver, mDeleteProfileName+"_"+i);
                deleteProfile(tmpProfileKey);
            }

            StringBuilder sb = new StringBuilder();
            sb.append(Settings.Global.NAME);
            sb.append(" like '");
            sb.append(mDeleteProfileName+"%'");
            String where = sb.toString();
            int deleted = mContentResolver.delete(Settings.Global.CONTENT_URI, where, null);
            Log.d(TAG, "delete profiles cache : " + where + " deleted : " + deleted);
        }

    }    
    
    /**
     * Gets predefined profiles' keys.
     * 
     * @return The predefined profiles' keys.
     */
    public List<String> getPredefinedProfileKeys() {
        List<String> predefinedKeys = new ArrayList<String>();
        predefinedKeys.addAll(mPredefinedKeys);
        Log.d(TAG, "getPredefinedProfileKeys: keys = " + predefinedKeys);
        return predefinedKeys;
    }
    
    /**
     * Gets customized profiles' keys.
     * 
     * @return The customized profiles' keys.
     */
    public List<String> getCustomizedProfileKeys() {
        // If profiles count is equal PREDEFINED_PROFILES_COUNT, there are no custom profiles
        if (getProfileCount() <= AudioProfileManager.PREDEFINED_PROFILES_COUNT) {
            return null;
        }
        // Remove all the predefined profile keys from all keys list and return it to be customs
        List<String> customizedProfileKeys = new ArrayList<String>();
        customizedProfileKeys.addAll(mKeys);
        customizedProfileKeys.removeAll(mPredefinedKeys);
        Log.d(TAG, "getCustomizedProfileKeys: " + customizedProfileKeys);
        return customizedProfileKeys;
    }
    
    /**
     * Persists profile's settings.
     * 
     * @param overrideSystem Whether override volume and ringtone to system .
     */
    private void persistValues(boolean overrideSystem) {
        AudioProfileState activeProfileState = mProfileStates.get(mActiveProfileKey);
        if (null == activeProfileState) {
            Log.e(TAG, "persistValues error with no " + mActiveProfileKey + " in " + mProfileStates);
        } else {
            Log.d(TAG, "persistValues: override = " + overrideSystem + ", state = " + activeProfileState.toString());
        }

        // First persist vibration to avoid CTS fail
        persistVibrationToSystem();

        if (overrideSystem) {
            persistStreamVolumeToSystem(AudioProfileManager.STREAM_RING);
            persistStreamVolumeToSystem(AudioProfileManager.STREAM_NOTIFICATION);
            persistStreamVolumeToSystem(AudioProfileManager.STREAM_ALARM);
            persistRingtoneUriToSystem(AudioProfileManager.TYPE_RINGTONE);
            persistRingtoneUriToSystem(AudioProfileManager.TYPE_NOTIFICATION);
            persistRingtoneUriToSystem(AudioProfileManager.TYPE_VIDEO_CALL);
            persistRingtoneUriToSystem(AudioProfileManager.TYPE_SIP_CALL);
        }

        persistDtmfToneToSystem();
        persistSoundEffectToSystem();
        persistLockScreenToSystem();
        persistHapticFeedbackToSystem();
    }
    
    /**
     * Restores the given profile to default values.
     * 
     * @param profileKey The key of the profile to be restored.
     */
    private void restoreToDefaultValues(String profileKey) {
        // Refresh the profile state in mProfileStates with default profile state, and delete all
        // value exist in database refer to the given profile
        AudioProfileState defaultState = AudioProfileManager.getDefaultState(profileKey);
        AudioProfileState profileState = mProfileStates.get(profileKey);
        synchronized (mProfileStates) {
            profileState.mRingerStream = mDefaultRingtone.get(DEFAULT_RINGER_INDEX);
            profileState.mNotificationStream = mDefaultRingtone.get(DEFAULT_NOTIFICATION_INDEX);
            profileState.mVideoCallStream = mDefaultRingtone.get(DEFAULT_VIDEOCALL_INDEX);
            profileState.mSIPCallStream = mDefaultRingtone.get(DEFAULT_SIPCALL_INDEX);

            profileState.mRingerVolume = defaultState.mRingerVolume;
            profileState.mNotificationVolume = defaultState.mNotificationVolume;
            profileState.mAlarmVolume = defaultState.mAlarmVolume;

            profileState.mVibrationEnabled = defaultState.mVibrationEnabled;
            profileState.mDtmfToneEnabled = defaultState.mDtmfToneEnabled;
            profileState.mSoundEffectEnbled = defaultState.mSoundEffectEnbled;
            profileState.mLockScreenSoundEnabled = defaultState.mLockScreenSoundEnabled;
            profileState.mHapticFeedbackEnabled = defaultState.mHapticFeedbackEnabled;
        }
        List<String> keyList = AudioProfileManager.getAllKeys(profileKey);
        StringBuilder sb = new StringBuilder();
        sb.append(Settings.System.NAME);
        sb.append(" in (");
        int size = keyList.size();
        for (int i = 1; i < size; i++) {
            sb.append("?,");
        }
        sb.append("?)");
        String where = sb.toString();
        mContentResolver.delete(Settings.System.CONTENT_URI, where, keyList.toArray(new String[size]));
        Log.d(TAG, "restoreToDefaultValues: profileKey = " + profileKey);
    }

    /**
     * sync the three type volumes to system that has been changed in last active profile's settings
     * One of {@link #STREAM_RING}, {@link #STREAM_NOTIFICATION}  
     *                   or {@link #STREAM_ALARM}.
     * @see #setStreamVolume(String, int, int)
     * @see #getStreamVolume(String, int)
     */
    private void syncVolumeToSystem() {
        int systemVolume = 0;
        int profileVolume = 0;
        // Ringer volume
        if (mShouldSyncToSystem.get(ProfileSettings.ringer_volume.ordinal())) {
            systemVolume = mAudioManager.getStreamVolume(AudioProfileManager.STREAM_RING);
            profileVolume = getStreamVolume(mActiveProfileKey, AudioProfileManager.STREAM_RING);
            mShouldSyncToSystem.set(ProfileSettings.ringer_volume.ordinal(), false);
            if (profileVolume != systemVolume) {
                persistStreamVolumeToSystem(AudioProfileManager.STREAM_RING);
                Log.d(TAG, "syncVolumeToSystem: profileKey = " + mActiveProfileKey
                        + ", streamType = " + AudioProfileManager.STREAM_RING + ", volume = "
                        + profileVolume);
            }
        }
        
        // Notification volume
        if (mShouldSyncToSystem.get(ProfileSettings.notification_volume.ordinal())) {
            systemVolume = mAudioManager.getStreamVolume(AudioProfileManager.STREAM_NOTIFICATION);
            profileVolume = getStreamVolume(mActiveProfileKey,
                    AudioProfileManager.STREAM_NOTIFICATION);
            mShouldSyncToSystem.set(ProfileSettings.notification_volume.ordinal(), false);
            if (profileVolume != systemVolume) {
                persistStreamVolumeToSystem(AudioProfileManager.STREAM_NOTIFICATION);
                Log.d(TAG, "syncVolumeToSystem: profileKey = " + mActiveProfileKey
                        + ", streamType = " + AudioProfileManager.STREAM_NOTIFICATION
                        + ", volume = " + profileVolume);
            }
        }
        if (mShouldSyncToSystem.get(ProfileSettings.alarm_volume.ordinal())) {
            systemVolume = mAudioManager.getStreamVolume(AudioProfileManager.STREAM_ALARM);
            profileVolume = getStreamVolume(mActiveProfileKey, AudioProfileManager.STREAM_ALARM);
            mShouldSyncToSystem.set(ProfileSettings.alarm_volume.ordinal(), false);
            if (profileVolume != systemVolume) {
                persistStreamVolumeToSystem(AudioProfileManager.STREAM_ALARM);
                Log.d(TAG, "syncVolumeToSystem: profileKey = " + mActiveProfileKey
                        + ", streamType = " + AudioProfileManager.STREAM_ALARM + ", volume = "
                        + profileVolume);
            }
        }
    }
    
    /**
     * sync the three type ringtones to system that has been changed in last active profile's
     * settings One of {@link #TYPE_RINGTONE}, {@link #TYPE_NOTIFICATION},
     * or {@link #TYPE_VIDEO_CALL}.
     * 
     * @see #setRingtoneUri(String, int, Uri)
     * @see #getRingtoneUri(String, int)
     */
    private void syncRingtoneToSystem() {
        Uri systemUri = null;
        Uri profileUri = null;
        
        // Ringtone
        if (mShouldSyncToSystem.get(DEFAULT_RINGER_INDEX)) {
            systemUri = RingtoneManager.getActualDefaultRingtoneUri(mContext,
                    AudioProfileManager.TYPE_RINGTONE);
            profileUri = getRingtoneUri(mActiveProfileKey, AudioProfileManager.TYPE_RINGTONE);
            mShouldSyncToSystem.set(DEFAULT_RINGER_INDEX, false);
            if ((profileUri != null && !profileUri.equals(systemUri))
                    || (profileUri == null && systemUri != null)) {
                persistRingtoneUriToSystem(AudioProfileManager.TYPE_RINGTONE);
                Log.d(TAG, "syncRingtoneToSystem: profileKey = " + mActiveProfileKey + ", type = "
                        + AudioProfileManager.TYPE_RINGTONE + ", Uri = " + profileUri);
            }
        }
        
        // Notification
        if (mShouldSyncToSystem.get(DEFAULT_NOTIFICATION_INDEX)) {
            systemUri = RingtoneManager.getActualDefaultRingtoneUri(mContext,
                    AudioProfileManager.TYPE_NOTIFICATION);
            profileUri = getRingtoneUri(mActiveProfileKey, AudioProfileManager.TYPE_NOTIFICATION);
            mShouldSyncToSystem.set(DEFAULT_NOTIFICATION_INDEX, false);
            if ((profileUri != null && !profileUri.equals(systemUri))
                    || (profileUri == null && systemUri != null)) {
                persistRingtoneUriToSystem(AudioProfileManager.TYPE_NOTIFICATION);
                Log.d(TAG, "syncRingtoneToSystem: profileKey = " + mActiveProfileKey + ", type = "
                        + AudioProfileManager.TYPE_NOTIFICATION + ", Uri = " + profileUri);
            }
        }
        
        // Vediocall
        if (mShouldSyncToSystem.get(DEFAULT_VIDEOCALL_INDEX)) {
            systemUri = RingtoneManager.getActualDefaultRingtoneUri(mContext,
                    AudioProfileManager.TYPE_VIDEO_CALL);
            profileUri = getRingtoneUri(mActiveProfileKey, AudioProfileManager.TYPE_VIDEO_CALL);
            mShouldSyncToSystem.set(DEFAULT_VIDEOCALL_INDEX, false);
            if ((profileUri != null && !profileUri.equals(systemUri))
                    || (profileUri == null && systemUri != null)) {
                persistRingtoneUriToSystem(AudioProfileManager.TYPE_VIDEO_CALL);
                Log.d(TAG, "syncRingtoneToSystem: profileKey = " + mActiveProfileKey + ", type = "
                        + AudioProfileManager.TYPE_VIDEO_CALL + ", Uri = " + profileUri);
            }
        }
        
        // SIPcall
        if (mShouldSyncToSystem.get(DEFAULT_SIPCALL_INDEX)) {
            systemUri = RingtoneManager.getActualDefaultRingtoneUri(mContext,
                    AudioProfileManager.TYPE_SIP_CALL);
            profileUri = getRingtoneUri(mActiveProfileKey, AudioProfileManager.TYPE_SIP_CALL);
            mShouldSyncToSystem.set(DEFAULT_SIPCALL_INDEX, false);
            if ((profileUri != null && !profileUri.equals(systemUri))
                    || (profileUri == null && systemUri != null)) {
                persistRingtoneUriToSystem(AudioProfileManager.TYPE_SIP_CALL);
                Log.d(TAG, "syncRingtoneToSystem: profileKey = " + mActiveProfileKey + ", type = "
                        + AudioProfileManager.TYPE_SIP_CALL + ", Uri = " + profileUri);
            }
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Get methods to get profile settings with given profile key from persisted profile states
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Gets the {@link Uri} of the default sound for a given ring tone type.
     * 
     * @param profileKey
     *            The profile key ringtone uri is returned.
     * @param type
     *            The type whose default sound should be set. One of {@link #TYPE_RINGTONE},
     *            {@link #TYPE_NOTIFICATION} or {@link #TYPE_VIDEO_CALL}.
     * @returns A {@link Uri} pointing to the default sound to set.
     * @see #setRingtoneUri(Context, int, Uri)
     */
    public Uri getRingtoneUri(String profileKey, int type) {
        if (null == profileKey) {
            Log.e(TAG, "getRingtoneUri with null profile key!");
            return null;
        }

        Uri uri = null;
        switch (type) {
            case AudioProfileManager.TYPE_RINGTONE:
                uri = getProfileState(profileKey).mRingerStream;
                break;

            case AudioProfileManager.TYPE_NOTIFICATION:
                uri = getProfileState(profileKey).mNotificationStream;
                break;

            case AudioProfileManager.TYPE_VIDEO_CALL:
                uri = getProfileState(profileKey).mVideoCallStream;
                break;
                
            case AudioProfileManager.TYPE_SIP_CALL:
                uri = getProfileState(profileKey).mSIPCallStream;
                break;

            default:
                Log.e(TAG, "getRingtoneUri with unsupport type!");
                return null;
        }
        if (SILENT_NOTIFICATION_URI.equals(uri)) {
            // If the uri is special SILENT_NOTIFICATION_URI, return null to make ringtonepicker
            // select silent on checked.
            uri = null;
        } else if ((null == uri) || !isRingtoneExist(uri)) {
            // When the uri is null or not exist, use default ringtone.
            uri = getDefaultRingtone(type);
        }
        Log.d(TAG, "getRingtoneUri: profileKey = " + profileKey + ", type = " + type + ", uri = " + uri);
        return uri;
    }

    public Uri getRingtoneUriWithSIM(String profileKey, int type, long simId) {
        if (null == profileKey) {
            Log.e(TAG, "getRingtoneUri with null profile key!");
            return null;
        }
        mSimId = simId;
        Uri uri = null;
        switch (type) {
            case AudioProfileManager.TYPE_RINGTONE:
                uri = getProfileState(profileKey, simId).mRingerStream;
                break;

            case AudioProfileManager.TYPE_NOTIFICATION:
                uri = getProfileState(profileKey, simId).mNotificationStream;
                break;

            case AudioProfileManager.TYPE_VIDEO_CALL:
                uri = getProfileState(profileKey, simId).mVideoCallStream;
                break;

            case AudioProfileManager.TYPE_SIP_CALL:
                uri = getProfileState(profileKey, simId).mSIPCallStream;
                break;
                
            default:
                Log.e(TAG, "getRingtoneUri with unsupport type!");
                return null;
        }
        if (SILENT_NOTIFICATION_URI.equals(uri)) {
            // If the uri is special SILENT_NOTIFICATION_URI, return null to make ringtonepicker
            // select silent on checked.
            uri = null;
        } else if ((null == uri) || !isRingtoneExist(uri)) {
            // When the uri is null or not exist, use default ringtone.
            uri = getDefaultRingtone(type);
        }
        Log.d(TAG, "getRingtoneUriWithSIM: profileKey = " + profileKey + ", type = " + type + ", uri = " + uri + ", simId = " + simId);
        return uri;

    }

    /**
     * Returns the maximum volume index for a particular stream.
     * 
     * @param streamType
     *            The stream type whose maximum volume index is returned. Currently only
     *            {@link #STREAM_RING}, {@link #STREAM_NOTIFICATION} and {@link #STREAM_ALARM} are
     *            supported.
     * @return The maximum valid volume index for the stream.
     * @see #getStreamVolume(int)
     */
    public int getStreamMaxVolume(int streamType) {
        return mAudioManager.getStreamMaxVolume(streamType);
    }

    /**
     * Ensures the stream volume is in valid range.
     * 
     * @param streamType
     *            The stream type.
     * @param volume
     *            The stream volume.
     * @return
     */
    private int getStreamValidVolume(int streamType, int volume) {
        int max = this.getStreamMaxVolume(streamType);
        int validVolume = volume;
        if (volume < 0) {
            validVolume = 0;
        } else if (volume > max) {
            validVolume = max;
        }
        return validVolume;
    }

    /**
     * Returns the current volume index for a particular stream.
     * 
     * @param profileKey
     *            The profile key whose volume index is returned.
     * @param streamType
     *            The stream type whose volume index is returned. One of {@link #STREAM_RING},
     *            {@link #STREAM_NOTIFICATION} or {@link #STREAM_ALARM}.
     * 
     * @return The current volume index for the stream.
     * @see #getStreamMaxVolume(int)
     * @see #setStreamVolume(int, int, int)
     */
    public int getStreamVolume(String profileKey, int streamType) {

        if (null == profileKey) {
            Log.e(TAG, "getStreamVolume with null profile key!");
            return 0;
        }

        int volume = 0;
        switch (streamType) {
            case AudioProfileManager.STREAM_RING:
                volume = getProfileState(profileKey).mRingerVolume;
                break;

            case AudioProfileManager.STREAM_NOTIFICATION:
                volume = getProfileState(profileKey).mNotificationVolume;
                break;

            case AudioProfileManager.STREAM_ALARM:
                volume = getProfileState(profileKey).mAlarmVolume;
                break;

            default:
                Log.e(TAG, "getStreamVolume with unsupport type!");
                return AudioProfileManager.UNSUPPORT_STREAM_VOLUME;
        }
        int validVolume = getStreamValidVolume(streamType, volume);
        Log.d(TAG, "getStreamVolume: profileKey = " + profileKey + ", streamType = " + streamType + ", volume = "
                + validVolume);
        return validVolume;
    }

    /**
     * Gets whether the phone should vibrate for incoming calls.
     * 
     * @param profileKey
     *            The profile key whose DtmfTone whether enabled is returned.
     * @return The current vibration status, if enabled return true, otherwise false.
     * @see #setVibrationEnabled(boolean)
     */
    public boolean getVibrationEnabled(String profileKey) {
        boolean enabled = getProfileState(profileKey).mVibrationEnabled;
        Log.d(TAG, "getVibrationEnabled: profileKey = " + profileKey + ", enabled = " + enabled);
        return enabled;
    }

    /**
     * Gets whether tone should be played when using dial pad with the given profile key.
     * 
     * @param profileKey
     *            The profile key whose DtmfTone whether enabled is returned.
     * @return The current DtmfTone status, if enabled return true, otherwise false.
     * @see #setDtmfToneEnabled(boolean)
     */
    public boolean getDtmfToneEnabled(String profileKey) {
        boolean enabled = getProfileState(profileKey).mDtmfToneEnabled;
        Log.d(TAG, "getDtmfToneEnabled: profileKey = " + profileKey + ", enabled = " + enabled);
        return enabled;
    }

    /**
     * Gets whether sound should be played when making screen selection.
     * 
     * @param profileKey
     *            The profile key whose SoundEffect whether enabled is returned.
     * @return The current SoundEffect status, if enabled return true, otherwise false.
     * @see #setSoundEffectEnabled(boolean)
     */
    public boolean getSoundEffectEnabled(String profileKey) {
        boolean enabled = getProfileState(profileKey).mSoundEffectEnbled;
        Log.d(TAG, "getSoundEffectEnabled: profileKey = " + profileKey + ", enabled = " + enabled);
        return enabled;
    }

    /**
     * Gets whether sound should be played when lock or unlock screen.
     * 
     * @param profileKey
     *            The profile key whose LockScreen whether enabled is returned.
     * @return The current LockScreen status, if enabled return true, otherwise false.
     * @see #setLockScreenEnabled(String, boolean)
     */
    public boolean getLockScreenEnabled(String profileKey) {
        boolean enabled = getProfileState(profileKey).mLockScreenSoundEnabled;
        Log.d(TAG, "getLockScreenEnabled: profileKey = " + profileKey + ", enabled = " + enabled);
        return enabled;
    }

    /**
     * Gets whether the phone should vibrate when pressing soft keys and on certain UI interactions.
     * 
     * @param profileKey
     *            The profile key whose HapticFeedback whether enabled is returned.
     * @return The current HapticFeedback status, if enabled return true, otherwise false.
     * @see #setHapticFeedbackEnabled(boolean)
     */
    public boolean getHapticFeedbackEnabled(String profileKey) {
        boolean enabled = getProfileState(profileKey).mHapticFeedbackEnabled;
        Log.d(TAG, "getHapticFeedbackEnabled: profileKey = " + profileKey + ", enabled = " + enabled);
        return enabled;
    }

    // ezpt pp
    /**
     * Gets the profile state from profile states hash map with given profile key.
     * 
     * @param profileKey
     *            The profile key.
     * @return The current profile state referred to given profile key.
     * 
     */
    public AudioProfileState getProfileState(String profileKey, long simId) {
        synchronized (mProfileStates) {
            if (simId != mSingleSIM) {
                return readNewProfileState(profileKey, simId);
            }
            
            AudioProfileState profileState = mProfileStates.get(profileKey);
            if (profileState == null) {
                // New a new profile state to add to profile state
                Log.w(TAG, "getProfileState of " + profileKey + "is null, so create new one instead!");
                readNewProfileState(profileKey, simId);
                profileState = mProfileStates.get(profileKey);
            }
            return profileState;
        }
    }
    
    private AudioProfileState readNewProfileState(String profileKey, long simId) {
        // New a new profile state to add to profile state
        Log.w(TAG, "getProfileState of " + profileKey + " is null, so create new one instead!");
        readPersistedSettings(profileKey, simId);
        return mProfileStates.get(profileKey);
    }
    
    /**
     * Gets the profile state from profile states hash map with given profile key.
     * 
     * @param profileKey
     *            The profile key.
     * @return The current profile state referred to given profile key.
     * 
     */
    public AudioProfileState getProfileState(String profileKey) {
        return getProfileState(profileKey, mSingleSIM);
    }

    /**
     * Gets a string list of the profile state from profile states hash map with given profile key.
     * 
     * @param profileKey
     *            The profile key.
     * @return A string list of the current profile state referred to given profile key.
     * 
     */
    public List<String> getProfileStateString(String profileKey) {
        AudioProfileState profileState = mProfileStates.get(profileKey);
        int size = ProfileSettings.values().length;
        List<String> state = new ArrayList<String>(size);
        state.add(DEFAULT_RINGER_INDEX, profileState.mRingerStream.toString());
        state.add(DEFAULT_NOTIFICATION_INDEX, profileState.mNotificationStream.toString());
        state.add(DEFAULT_VIDEOCALL_INDEX, profileState.mVideoCallStream.toString());
        state.add(DEFAULT_SIPCALL_INDEX, profileState.mSIPCallStream.toString());

        state.add(ProfileSettings.ringer_volume.ordinal(), String.valueOf(profileState.mRingerVolume));
        state.add(ProfileSettings.notification_volume.ordinal(), String.valueOf(profileState.mNotificationVolume));
        state.add(ProfileSettings.alarm_volume.ordinal(), String.valueOf(profileState.mAlarmVolume));

        state.add(ProfileSettings.vibration_enabled.ordinal(), String.valueOf(profileState.mVibrationEnabled));
        state.add(ProfileSettings.dtmftone_enabled.ordinal(), String.valueOf(profileState.mDtmfToneEnabled));
        state.add(ProfileSettings.soundeffect_enbled.ordinal(), String.valueOf(profileState.mSoundEffectEnbled));
        state.add(ProfileSettings.lockscreensound_enabled.ordinal(), String.valueOf(profileState.mLockScreenSoundEnabled));
        state.add(ProfileSettings.hapticfeedback_enabled.ordinal(), String.valueOf(profileState.mHapticFeedbackEnabled));

        Log.d(TAG, "getProfileStateString for profileKey = " + profileKey + ": " + state);
        return state;
    }

    /**
     * Returns the name of given custom profile.
     * 
     * @param profileKey
     *            The custom profile key.
     * @return profile name
     * @see #setProfileName(String, String)
     */
    public String getProfileName(String profileKey) {
        String profileName = mCustomProfileName.get(profileKey);
        Log.d(TAG, "getProfileName: profileKey = " + profileKey + ", profileName = " + profileName);
        return profileName;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Set methods to set profile setting to database with given profile key
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Sets the {@link Uri} of the default sound for a given ring tone type and profile key.
     * 
     * @param profileKey
     *            The profile key given to set ringtone uri.
     * @param type
     *            The type whose default sound should be set. One of {@link #TYPE_RINGTONE},
     *            {@link #TYPE_NOTIFICATION}, or {@link #TYPE_VIDEO_CALL}.
     * @param ringtoneUri
     *            A {@link Uri} pointing to the default sound to set.
     * @see #getRingtoneUri(Context, int)
     */
    public void setRingtoneUri(String profileKey, int type, Uri ringtoneUri) {
        setRingtoneUri(profileKey, type, mSingleSIM, ringtoneUri);
    }
    

    /**
     * Sets the {@link Uri} of the default sound for a given ring tone type and profile key.
     * 
     * @param profileKey
     *            The profile key given to set ringtone uri.
     * @param type
     *            The type whose default sound should be set. One of {@link #TYPE_RINGTONE},
     *            {@link #TYPE_NOTIFICATION}, or {@link #TYPE_VIDEO_CALL}.
     * @param ringtoneUri
     *            A {@link Uri} pointing to the default sound to set.
     * @see #getRingtoneUri(Context, int)
     */
    public void setRingtoneUri(String profileKey, int type, long simId, Uri ringtoneUri) {
        AudioProfileState profileState = getProfileState(profileKey);
        if (profileState == null) {
            Log.e(TAG, "setRingtoneUri profile state not exist!");
            return;
        }
        
        mSimId = simId;

        if (!isValidRingtoneType(type)) {
            Log.e(TAG, "setRingtoneUri with undefined stream type!");
        }

        Uri newRingtoneUri = ringtoneUri;
        if (!isRingtoneUriChanged(profileState, type, newRingtoneUri)) {
            Log.v(TAG, "setRingtoneUri with ringtone uri unchanged!");
        }

        // Set notification uri to be SILENT_NOTIFICATION_URI
        // when the notification uri to be set is null
        if (AudioProfileManager.TYPE_NOTIFICATION == type) {
            if (null == newRingtoneUri) {
                newRingtoneUri = SILENT_NOTIFICATION_URI;
            }
        }

        // Actually set the new ringtone uri into db, hash map and the system
        doRingtoneUriSetting(profileKey, type, simId, newRingtoneUri);

        // If the profile is the last active profile, we should make a flag in order to
        // sync the newest uri to system when the profile was set active later
        if (profileKey.equals(mLastActiveProfileKey)) {
            setShouldSyncToSystemFlag(type, true);
        }

        // Synchronize general to outdoor when necessary
        if (mExt.shouldSyncGeneralRingtoneToOutdoor()) {
            syncGeneralRingtoneToOutdoor(profileKey, type, newRingtoneUri, simId);
        }

        Log.d(TAG, "setRingtoneUri: profileKey = " + profileKey + ", type = " + type + ", uri = " + newRingtoneUri);
    }

    private boolean isValidRingtoneType(int type) {
        return (AudioProfileManager.TYPE_RINGTONE == type) || (AudioProfileManager.TYPE_NOTIFICATION == type)
                || (AudioProfileManager.TYPE_VIDEO_CALL == type) || (AudioProfileManager.TYPE_SIP_CALL == type);
    }

    private boolean isRingtoneUriChanged(AudioProfileState profileState, int type, Uri newRingtoneUri) {
        switch (type) {
            case AudioProfileManager.TYPE_RINGTONE:
                return isNotEqual(profileState.mRingerStream, newRingtoneUri);

            case AudioProfileManager.TYPE_NOTIFICATION:
                return isNotEqual(profileState.mNotificationStream, newRingtoneUri);

            case AudioProfileManager.TYPE_VIDEO_CALL:
                return isNotEqual(profileState.mVideoCallStream, newRingtoneUri);
                
            case AudioProfileManager.TYPE_SIP_CALL:
                return isNotEqual(profileState.mSIPCallStream, newRingtoneUri);
            default:
                return true;
        }
    }

    /**
     * Set whether need sync the profile setting to system.
     * 
     * @param type ringtone type
     * @param shouldSync whether need to sync the given type ringtone to system
     */
    public void setShouldSyncToSystemFlag(int type, boolean shouldSync) {
        switch (type) {
            case AudioProfileManager.TYPE_RINGTONE:
                mShouldSyncToSystem.set(DEFAULT_RINGER_INDEX, shouldSync);
                break;

            case AudioProfileManager.TYPE_NOTIFICATION:
                mShouldSyncToSystem.set(DEFAULT_NOTIFICATION_INDEX, shouldSync);
                break;

            case AudioProfileManager.TYPE_VIDEO_CALL:
                mShouldSyncToSystem.set(DEFAULT_VIDEOCALL_INDEX, shouldSync);
                break;
                
            case AudioProfileManager.TYPE_SIP_CALL:
                mShouldSyncToSystem.set(DEFAULT_SIPCALL_INDEX, shouldSync);
                break;
                
            default:
                break;
        }
    }

    private void syncGeneralRingtoneToOutdoor(String profileKey, int type, Uri newRingtoneUri) {
        syncGeneralRingtoneToOutdoor(profileKey, type, newRingtoneUri, mSingleSIM);
    }

    private void syncGeneralRingtoneToOutdoor(String profileKey, int type, Uri newRingtoneUri, long simId) {
        // New plan in non CMCC project ask synchronize general and outdoor
        // profile's settings, so if profile is general, we should set value
        // to outdoor at the same time.
        Scenario scenario = AudioProfileManager.getScenario(profileKey);
        if (Scenario.GENERAL.equals(scenario)) {
            String outdoorKey = mPredefinedKeys.get(Scenario.OUTDOOR.ordinal());
            doRingtoneUriSetting(outdoorKey, type, simId, newRingtoneUri);
            Log.v(TAG, "synchronize general to outdoor! ringtone type = " + type);
        }
    }

    private void doRingtoneUriSetting(String profileKey, int type, Uri uri) {
        doRingtoneUriSetting(profileKey, type, mSingleSIM, uri);
    }
    
    private void doRingtoneUriSetting(String profileKey, int type, long simid, Uri uri) {
        // Firstly, persist to database
        persistRingtoneUriToDatabase(profileKey, type, simid, uri);

        // Secondly, update the hash map
        AudioProfileState state = getProfileState(profileKey);
        synchronized (mProfileStates) {
            setRingtoneUriInStateMap(state, type, uri, simid);
        }

        // Finally, if the profile is active, persist it to system
        if (isActive(profileKey)) {
            persistRingtoneUriToSystem(type);
        }
    }

    private void setRingtoneUriInStateMap(AudioProfileState state, int type, Uri value) {
        setRingtoneUriInStateMap(state, type, value, mSingleSIM);
    }
    
    private void setRingtoneUriInStateMap(AudioProfileState state, int type, Uri value, long simId) {
        switch (type) {
            case AudioProfileManager.TYPE_RINGTONE:
                state.mRingerStream = value;
                break;

            case AudioProfileManager.TYPE_NOTIFICATION:
                state.mNotificationStream = value;
                break;

            case AudioProfileManager.TYPE_VIDEO_CALL:
                state.mVideoCallStream = value;
                break;
                
            case AudioProfileManager.TYPE_SIP_CALL:
                state.mSIPCallStream = value;
                break;

            default:
                break;
        }
        
        if (simId != mSingleSIM) {
            state.mSimId = simId;
        }
    }

    /**
     * Check the given two objs not equal.
     * 
     * @param obj1
     * @param obj2
     * @return If not equal, return true, otherwise false.
     */
    private boolean isNotEqual(Object obj1, Object obj2) {
        return (((obj1 == null) && (obj2 != null)) || ((obj1 != null) && !obj1.equals(obj2)));
    }

    /**
     * Sets the volume index for a particular stream to database.
     * 
     * @param profileKey
     *            The profile key given to set stream volume.
     * @param streamType
     *            The stream whose volume index should be set. One of {@link #STREAM_RING},
     *            {@link #STREAM_NOTIFICATION} or {@link #STREAM_ALARM}.
     * @param index
     *            The volume index to set.
     * @see #getStreamMaxVolume(int)
     * @see #getStreamVolume(int)
     */
    public void setStreamVolume(String profileKey, int streamType, int index) {
        int validIndex = this.getStreamValidVolume(streamType, index);
        AudioProfileState profileState = getProfileState(profileKey);
        if (profileState == null) {
            Log.e(TAG, "setStreamVolume profile state not exist!");
            return;
        }

        switch (streamType) {
            case AudioProfileManager.STREAM_RING:
                if (profileState.mRingerVolume != validIndex) {
                    // If the profile's volume is different from given volume, set the volume to be
                    // profile volume into database and save this change to profile states hash map
                    persistStreamVolumeToDatabase(profileKey, streamType, validIndex);
                    synchronized (mProfileStates) {
                        profileState.mRingerVolume = validIndex;
                    }

                    // If the profile is last active profile, we should save user change and sync
                    // to system when the profile was set to active profile.
                    if (profileKey.equals(mLastActiveProfileKey)) {
                        mShouldSyncToSystem.set(ProfileSettings.ringer_volume.ordinal(), true);
                    }
                }
                break;

            case AudioProfileManager.STREAM_NOTIFICATION:
                if (profileState.mNotificationVolume != validIndex) {
                    // If the profile's volume is different from given volume, set the volume to be
                    // profile volume into database and save this change to profile states hash map
                    persistStreamVolumeToDatabase(profileKey, streamType, validIndex);
                    synchronized (mProfileStates) {
                        profileState.mNotificationVolume = validIndex;
                    }

                    // If the profile is last active profile, we should save user change and sync
                    // to system when the profile was set to active profile.
                    if (profileKey.equals(mLastActiveProfileKey)) {
                        mShouldSyncToSystem.set(ProfileSettings.notification_volume.ordinal(), true);
                    }
                }
                break;

            case AudioProfileManager.STREAM_ALARM:
                if (profileState.mAlarmVolume != validIndex) {
                    // If the profile's volume is different from given volume, set the volume to be
                    // profile volume into database and save this change to profile states hash map
                    persistStreamVolumeToDatabase(profileKey, streamType, validIndex);
                    synchronized (mProfileStates) {
                        profileState.mAlarmVolume = validIndex;
                    }

                    // If the profile is last active profile, we should save user change and sync
                    // to system when the profile was set to active profile.
                    if (profileKey.equals(mLastActiveProfileKey)) {
                        mShouldSyncToSystem.set(ProfileSettings.alarm_volume.ordinal(), true);
                    }
                }
                break;

            default:
                Log.e(TAG, "setStreamVolume with undefind stream type!");
                break;
        }
        Log.d(TAG, "setStreamVolume: profileKey = " + profileKey + ", streamType = " + streamType + ", volume = "
                + validIndex);
    }

    /**
     * Sets whether the phone should vibrate for incoming calls.
     * 
     * @param profileKey
     *            The profile key given to set vibration enabled.
     * @param enabled
     *            Whether vibration enabled.
     * @see #getVibrationEnabled()
     */
    public void setVibrationEnabled(String profileKey, boolean enabled) {
        // Call from settings AudioProfile, should set ringermode to match vibration.
        setVibrationEnabled(profileKey, enabled, true);
    }

    /**
     * Sets whether the phone should vibrate for incoming calls, and need check whether we need set ringermode to match
     * vibration change. We need not set ringermode when this method is called trigger by ringermode changed.
     * 
     * @param shouldSetRingerMode
     *            Whether need to set ringermode.
     * @see #getVibrationEnabled()
     * @see #setVibrationEnabled(String, boolean)
     */
    public void setVibrationEnabled(String profileKey, boolean enabled, boolean shouldSetRingerMode) {
        AudioProfileState profileState = getProfileState(profileKey);
        if (profileState == null) {
            Log.e(TAG, "setVibrationEnabled profile state not exist!");
            return;
        }

        if (profileState.mVibrationEnabled != enabled) {
            // Only persist silent profile's vibrate_in_silent parameter.
            // For it only has impacts in RINGER_MODE_SILENT and RINGER_MODE_VIBRATE.
            if (Scenario.SILENT.equals(AudioProfileManager.getScenario(profileKey))) {
                Settings.System.putInt(mContentResolver, Settings.System.VIBRATE_IN_SILENT, enabled ? 1 : 0);
                // If the profile is silent and is active,change ringermode
                if (isActive(profileKey) && shouldSetRingerMode) {
                    if (enabled) {
                        mAudioManager.setRingerMode(AudioManager.RINGER_MODE_VIBRATE);
                        mRingerMode = AudioManager.RINGER_MODE_VIBRATE;
                        Log.d(TAG, "setVibrationEnabled true,change RingerMode to VIBRATE");
                    } else {
                        mAudioManager.setRingerMode(AudioManager.RINGER_MODE_SILENT);
                        mRingerMode = AudioManager.RINGER_MODE_SILENT;
                        Log.d(TAG, "setVibrationEnabled false,change RingerMode to SILENT");
                    }
                }
            }
            // Save this change to profile states hash map
            persistVibrationToDatabase(profileKey, enabled);
            synchronized (mProfileStates) {
                profileState.mVibrationEnabled = enabled;
            }
            // If the profile is active profile, persist it to system
            if (isActive(profileKey)) {
                persistVibrationToSystem();
            }
            Log.d(TAG, "setVibrationEnabled: profileKey = " + profileKey + ", enabled = " + enabled
                    + ", shouldSetRingerMode = " + shouldSetRingerMode);
        }
    }

    /**
     * Gets whether tone should be played when using dial pad.
     * 
     * @param profileKey
     *            The profile key given to set vibration enabled.
     * @param enabled
     *            Whether DtmfTone enabled.
     * @see #setDtmfToneEnabled(boolean)
     */
    public void setDtmfToneEnabled(String profileKey, boolean enabled) {
        AudioProfileState profileState = getProfileState(profileKey);
        if (profileState == null) {
            Log.e(TAG, "setDtmfToneEnabled profile state not exist!");
            return;
        }

        if (profileState.mDtmfToneEnabled != enabled) {
            // Save this change to profile states hash map
            persistDtmfToneToDatabase(profileKey, enabled);
            synchronized (mProfileStates) {
                profileState.mDtmfToneEnabled = enabled;
            }
            // If the profile is active profile, persist it to system
            if (isActive(profileKey)) {
                persistDtmfToneToSystem();
            }
            Log.d(TAG, "setDtmfToneEnabled: profileKey = " + profileKey + ", enabled = " + enabled);
        }
    }

    /**
     * Sets whether sound should be played when making screen selection.
     * 
     * @param profileKey
     *            The profile key given to set vibration enabled.
     * @param enabled
     *            Whether SoundEffect enabled.
     * @see #getSoundEffectEnabled()
     */
    public void setSoundEffectEnabled(String profileKey, boolean enabled) {
        AudioProfileState profileState = getProfileState(profileKey);
        if (profileState == null) {
            Log.e(TAG, "setSoundEffectEnabled profile state not exist!");
            return;
        }

        if (profileState.mSoundEffectEnbled != enabled) {
            // Save this change to profile states hash map
            persistSoundEffectToDatabase(profileKey, enabled);
            synchronized (mProfileStates) {
                profileState.mSoundEffectEnbled = enabled;
            }
            // If the profile is active profile, persist it to system
            if (isActive(profileKey)) {
                persistSoundEffectToSystem();
            }
            Log.d(TAG, "setSoundEffectEnabled: profileKey = " + profileKey + ", enabled = " + enabled);
        }
    }

    /**
     * Sets whether sound should be played when lock or unlock screen.
     * 
     * @param profileKey
     *            The profile key given to set vibration enabled.
     * @param enabled
     *            Whether LockScreen sound enabled.
     * @see #getLockScreenEnabled(String)
     */
    public void setLockScreenEnabled(String profileKey, boolean enabled) {
        AudioProfileState profileState = getProfileState(profileKey);
        if (profileState == null) {
            Log.e(TAG, "setLockScreenEnabled profile state not exist!");
            return;
        }

        if (profileState.mLockScreenSoundEnabled != enabled) {
            // Save this change to profile states hash map
            persistLockScreenToDatabase(profileKey, enabled);
            synchronized (mProfileStates) {
                profileState.mLockScreenSoundEnabled = enabled;
            }
            // If the profile is active profile, persist it to system
            if (isActive(profileKey)) {
                persistLockScreenToSystem();
            }
            Log.d(TAG, "setLockScreenEnabled: profileKey = " + profileKey + ", enabled = " + enabled);
        }
    }

    /**
     * Sets whether the phone should vibrate when pressing soft keys and on certain UI interactions.
     * 
     * @param profileKey
     *            The profile key given to set vibration enabled.
     * @param enabled
     *            Whether HapticFeedback enabled.
     * @see #getHapticFeedbackEnabled(String)
     */
    public void setHapticFeedbackEnabled(String profileKey, boolean enabled) {
        AudioProfileState profileState = getProfileState(profileKey);
        if (profileState == null) {
            Log.e(TAG, "setHapticFeedbackEnabled profile state not exist!");
            return;
        }

        if (profileState.mHapticFeedbackEnabled != enabled) {
            // Save this change to profile states hash map
            persistHapticFeedbackToDatabase(profileKey, enabled);
            synchronized (mProfileStates) {
                profileState.mHapticFeedbackEnabled = enabled;
            }
            // If the profile is active profile, persist it to system
            if (isActive(profileKey)) {
                persistHapticFeedbackToSystem();
            }
            Log.d(TAG, "setHapticFeedbackEnabled: profileKey = " + profileKey + ", enabled = " + enabled);
        }
    }

    /**
     * Sets the given profile's name.
     * 
     * @param profileKey
     *            The key of the profile.
     * @param newName
     *            The new name to be set.
     * @see #getProfileName(String)
     */
    public void setProfileName(String profileKey, String newName) {
        String profileName = mCustomProfileName.get(profileKey);
        if ((profileName != null && !profileName.equals(newName)) || (profileName == null && newName != null)) {
            // Save this change to profile states hash map
            persistProfileNameToDatabase(profileKey, newName);
            mCustomProfileName.put(profileKey, newName);
            Log.d(TAG, "setProfileName: profileKey = " + profileKey + ", newName = " + newName);
        } else {
            Log.e(TAG, "setProfileName with Null name!");
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // PersistToDatabase methods to persist settings to database with given profile key and value
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Persist the ringtone uri of particular stream to database.
     * 
     * @param profileKey
     *            The key of the profile to be persist.
     * @param type
     *            The type whose default sound should be set. One of
     *            {@link AudioProfileManager#TYPE_RINGTONE},
     *            {@link AudioProfileManager#TYPE_NOTIFICATION}, or
     *            {@link AudioProfileManager#TYPE_VIDEO_CALL}.
     * @param uri
     *            The uri to be persist to database
     */
    public void persistRingtoneUriToDatabase(String profileKey, int type, Uri uri) {
        persistRingtoneUriToDatabase(profileKey, type, mSingleSIM, uri);
    }

	private void persistRingtoneUriToDatabase(String profileKey, int type, long simId, Uri uri) {
        Message msg;
        //add for Sim slot ringtone difference
        String name = null;
        if (simId == mSingleSIM) {
            name = AudioProfileManager.getStreamUriKey(profileKey, type);
        } else {
            name = AudioProfileManager.getStreamUriKey(profileKey, type, simId);
        }
		
        Bundle bundle = new Bundle(BUNDLE_DEFAULT_SIZE);
        bundle.putString(name, (uri == null ? null : uri.toString()));
        switch (type) {
            case AudioProfileManager.TYPE_RINGTONE:
                msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_VOICECALL_RINGTONE_TO_DATABASE, -1, -1, name);
                break;

            case AudioProfileManager.TYPE_NOTIFICATION:
                msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_NOTIFICATION_RINGTONE_TO_DATABASE, -1, -1, name);
                break;

            case AudioProfileManager.TYPE_VIDEO_CALL:
                msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_VIDEOCALL_RINGTONE_TO_DATABASE, -1, -1, name);
                break;
                
            case AudioProfileManager.TYPE_SIP_CALL:
                msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_SIPCALL_RINGTONE_TO_DATABASE, -1, -1, name);
                break;

            default:
                Log.e(TAG, "persistRingtoneUriToDatabase with undefined stream type!");
                return;
        }
        msg.setData(bundle);
        msg.sendToTarget();
    }

    /**
     * Persist the ringtone volume of particular stream to database.
     * 
     * @param profileKey
     *            The key of the profile to be persist.
     * @param streamType
     *            The stream type whose volume index to be persisted. One of
     *            {@link AudioProfileManager#STREAM_RING},
     *            {@link AudioProfileManager#STREAM_NOTIFICATION}, or
     *            {@link AudioProfileManager#STREAM_ALARM}.
     * @param value
     *            The volume value to be persist to database
     */
    private void persistStreamVolumeToDatabase(String profileKey, int streamType, int value) {
        Message msg;
        String name = AudioProfileManager.getStreamVolumeKey(profileKey, streamType);
        switch (streamType) {
            case AudioProfileManager.STREAM_RING:
                msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_RINGER_VOLUME_TO_DATABASE, value, -1, name);
                break;

            case AudioProfileManager.STREAM_NOTIFICATION:
                msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_NOTIFICATION_VOLUME_TO_DATABASE, value, -1, name);
                break;

            case AudioProfileManager.STREAM_ALARM:
                msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_ALARM_VOLUME_TO_DATABASE, value, -1, name);
                break;

            default:
                Log.e(TAG, "persistStreamVolumeToDatabase with undefined stream type!");
                return;
        }
        msg.sendToTarget();
    }

    /**
     * Persist the setting to database that indicates whether phone should vibrate for incoming
     * calls.
     * 
     * @param profileKey
     *            The key of the profile to be persist.
     * @param enabled
     *            The vibration status to be persist to database
     */
    private void persistVibrationToDatabase(String profileKey, boolean enabled) {
        Bundle bundle = new Bundle(BUNDLE_DEFAULT_SIZE);
        String name = AudioProfileManager.getVibrationKey(profileKey);
        bundle.putString(name, String.valueOf(enabled).toString());
        bundle.putString("Vibration", String.valueOf(enabled));
        Message msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_VIBRATION_TO_DATABASE, -1, -1, name);
        msg.setData(bundle);
        msg.sendToTarget();
    }

    /**
     * Persist the setting to database that indicates whether sound should be played when using dial
     * pad.
     * 
     * @param profileKey
     *            The key of the profile to be persist.
     * @param enabled
     *            The DtmfTone status to be persist to database
     */
    private void persistDtmfToneToDatabase(String profileKey, boolean enabled) {
        Bundle bundle = new Bundle(BUNDLE_DEFAULT_SIZE);
        String name = AudioProfileManager.getDtmfToneKey(profileKey);
        bundle.putString(name, String.valueOf(enabled));
        Message msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_DTMF_TONE_TO_DATABASE, -1, -1, name);
        msg.setData(bundle);
        msg.sendToTarget();
    }

    /**
     * Persist the setting to database that indicates whether sound should be played when making
     * screen selection.
     * 
     * @param profileKey
     *            The key of the profile to be persist.
     * @param enabled
     *            The SoundEffect status to be persist to database
     */
    private void persistSoundEffectToDatabase(String profileKey, boolean enabled) {
        Bundle bundle = new Bundle(BUNDLE_DEFAULT_SIZE);
        String name = AudioProfileManager.getSoundEffectKey(profileKey);
        bundle.putString(name, String.valueOf(enabled));
        Message msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_SOUND_EFFECT_TO_DATABASE, -1, -1, name);
        msg.setData(bundle);
        msg.sendToTarget();
    }

    /**
     * Persist the setting to database that indicates whether to play sounds when the keyguard is
     * shown and dismissed.
     * 
     * @param profileKey
     *            The key of the profile to be persist.
     * @param enabled
     *            The LockScreenSound status to be persist to database
     */
    private void persistLockScreenToDatabase(String profileKey, boolean enabled) {
        Bundle bundle = new Bundle(BUNDLE_DEFAULT_SIZE);
        String name = AudioProfileManager.getLockScreenKey(profileKey);
        bundle.putString(name, String.valueOf(enabled));
        Message msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_LOCKSCREEN_SOUND_TO_DATABASE, -1, -1, name);
        msg.setData(bundle);
        msg.sendToTarget();
    }

    /**
     * Persist the setting to database that indicates whether the phone should vibrate when pressing
     * soft keys and on certain UI interactions.
     * 
     * @param profileKey
     *            The key of the profile to be persist.
     * @param enabled
     *            The LockScreenSound status to be persist to database
     */
    private void persistHapticFeedbackToDatabase(String profileKey, boolean enabled) {
        Bundle bundle = new Bundle(BUNDLE_DEFAULT_SIZE);
        String name = AudioProfileManager.getHapticKey(profileKey);
        bundle.putString(name, String.valueOf(enabled));
        Message msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_HAPTIC_FEEDBACK_TO_DATABASE, -1, -1, name);
        msg.setData(bundle);
        msg.sendToTarget();
    }

    /**
     * Persist the profile name to database.
     * 
     * @param profileKey
     *            The key of the profile to be persist.
     * @param profileName
     *            The profile name to be persist to database
     */
    private void persistProfileNameToDatabase(String profileKey, String profileName) {
        Bundle bundle = new Bundle(BUNDLE_DEFAULT_SIZE);
        String name = AudioProfileManager.getProfileNameKey(profileKey);
        bundle.putString(name, profileName);
        Message msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_PROFILE_NAME_TO_DATABASE, -1, -1, name);
        msg.setData(bundle);
        msg.sendToTarget();
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // PersistToSystem methods to persist active profile settings to system
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Persist the active profile ringtone uri to system to make it effective immediately with given
     * type
     * 
     * @param type
     *            The type whose default sound should be set. One of
     *            {@link AudioProfileManager#TYPE_RINGTONE},
     *            {@link AudioProfileManager#TYPE_NOTIFICATION}, or
     *            {@link AudioProfileManager#TYPE_VIDEO_CALL}.
     */
    private void persistRingtoneUriToSystem(int type) {
        Message msg;
        Bundle bundle = new Bundle(BUNDLE_DEFAULT_SIZE);
        String name = null;
        Uri uri = null;

        switch (type) {
            case AudioProfileManager.TYPE_RINGTONE:
                name = String.valueOf(type);
                uri = getProfileState(mActiveProfileKey).mRingerStream;
                msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_VOICECALL_RINGTONE_TO_SYSTEM, -1, -1, name);
                break;

            case AudioProfileManager.TYPE_NOTIFICATION:
                name = String.valueOf(type);
                uri = getProfileState(mActiveProfileKey).mNotificationStream;
                // If uri is the special SILENT_NOTIFICATION_URI, use null to persist it to system
                if (SILENT_NOTIFICATION_URI.equals(uri)) {
                    uri = null;
                }
                msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_NOTIFICATION_RINGTONE_TO_SYSTEM, -1, -1, name);
                break;

            case AudioProfileManager.TYPE_VIDEO_CALL:
                name = String.valueOf(type);
                uri = getProfileState(mActiveProfileKey).mVideoCallStream;
                msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_VIDEOCALL_RINGTONE_TO_SYSTEM, -1, -1, name);
                break;
                
            case AudioProfileManager.TYPE_SIP_CALL:
                name = String.valueOf(type);
                uri = getProfileState(mActiveProfileKey).mSIPCallStream;
                msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_SIPCALL_RINGTONE_TO_SYSTEM, -1, -1, name);
                break;

            default:
                Log.e(TAG, "persistRingtoneUriToSystem with unsupport type!");
                return;
        }
        bundle.putString(name, (uri == null ? null : uri.toString()));
        msg.setData(bundle);
        msg.sendToTarget();
    }

    /**
     * Persist the active profile volume to system to make it effective immediately with given type
     * 
     * @param type
     *            The stream type whose volume index to be persisted. One of
     *            {@link AudioProfileManager#STREAM_RING},
     *            {@link AudioProfileManager#STREAM_NOTIFICATION}, or
     *            {@link AudioProfileManager#STREAM_ALARM}.
     */
    private void persistStreamVolumeToSystem(int streamType) {
        if (mExt.persistStreamVolumeToSystem(streamType)) {
            return;
        }

        long ident = Binder.clearCallingIdentity();

        try {
            int flags = 0;
            int volume = 0;
            switch (streamType) {
                case AudioProfileManager.STREAM_RING:
                    volume = getProfileState(mActiveProfileKey).mRingerVolume;
                    mAudioManager.setStreamVolume(AudioManager.STREAM_RING, volume, flags);
                    break;
                case AudioProfileManager.STREAM_NOTIFICATION:
                    volume = getProfileState(mActiveProfileKey).mNotificationVolume;
                    mAudioManager.setStreamVolume(AudioManager.STREAM_NOTIFICATION, volume, flags);
                    break;
                case AudioProfileManager.STREAM_ALARM:
                    volume = getProfileState(mActiveProfileKey).mAlarmVolume;
                    mAudioManager.setStreamVolume(AudioManager.STREAM_ALARM, volume, flags);
                    break;
                default:
                    Log.e(TAG, "persistStreamVolumeToSystem with unsupport type!");
                    return;
            }
            Log.d(TAG, "persistStreamVolumeToSystem: streamType = " + streamType + ", volume = " + volume);
        } finally {
            Binder.restoreCallingIdentity(ident);
        }
    }

    /**
     * Persist the active profile vibration status to system to make it effective immediately.
     */
    private void persistVibrationToSystem() {
        // Avoid CTS fail, so when test CTS delay to set vibrate
        int vibratinRinger = mAudioManager.getVibrateSetting(AudioManager.VIBRATE_TYPE_RINGER);
        int vibratinNotification = mAudioManager.getVibrateSetting(AudioManager.VIBRATE_TYPE_NOTIFICATION);
        Log.d(TAG, "persistVibrationToSystem current vibrate status: ringer = " + vibratinRinger + ", notification = "
                + vibratinNotification);

        if (vibratinRinger != vibratinNotification) {
            Log.d(TAG, "persistVibrationToSystem different vibrate settings,"
                    + " so CTS test running, delay 10 sec to set vibration!");
            mAudioProfileHandler.removeMessages(MSG_DELAY_SET_VIBRATE_AVOID_CTS_FAIL);
            mAudioProfileHandler.sendEmptyMessageDelayed(MSG_DELAY_SET_VIBRATE_AVOID_CTS_FAIL, DELAY_TIME_AVOID_CTS_FAIL);
            mDelaySetVibrate = true;
            return;
        }
        if (mDelaySetVibrate) {
            Log.d(TAG, "persistVibrationToSystem: CTS test running,delay 20 sec to set vibration!");
            return;
        }

        // If vibrate on for ringer has been checked,use VIBRATE_SETTING_ON
        // Otherwise use VIBRATE_SETTING_ONLY_SILENT.
        int vibrationStatus = getProfileState(mActiveProfileKey).mVibrationEnabled ? AudioManager.VIBRATE_SETTING_ON
                        : AudioManager.VIBRATE_SETTING_ONLY_SILENT;

        mAudioManager.setVibrateSetting(AudioManager.VIBRATE_TYPE_RINGER, vibrationStatus);
        mAudioManager.setVibrateSetting(AudioManager.VIBRATE_TYPE_NOTIFICATION, vibrationStatus);
        Log.d(TAG, "persistVibrationToSystem set ringer and notification vibrate to: " + vibrationStatus);
    }

    /**
     * Persist the active profile DtmfTone status to system to make it effective immediately.
     */
    private void persistDtmfToneToSystem() {
        String name = Settings.System.DTMF_TONE_WHEN_DIALING;
        int enabled = getProfileState(mActiveProfileKey).mDtmfToneEnabled ? 1 : 0;
        Message msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_DTMF_TONE_TO_SYSTEM, enabled, -1, name);
        msg.sendToTarget();
    }

    /**
     * Persist the active profile SoundEffect status to system to make it effective immediately.
     */
    private void persistSoundEffectToSystem() {
        String name = Settings.System.SOUND_EFFECTS_ENABLED;
        int enabled = getProfileState(mActiveProfileKey).mSoundEffectEnbled ? 1 : 0;
        Message msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_SOUND_EFFECT_TO_SYSTEM, enabled, -1, name);
        msg.sendToTarget();
    }

    /**
     * Persist the active profile LockScreen status to system to make it effective immediately.
     */
    private void persistLockScreenToSystem() {
        String name = Settings.System.LOCKSCREEN_SOUNDS_ENABLED;
        int enabled = getProfileState(mActiveProfileKey).mLockScreenSoundEnabled ? 1 : 0;
        Message msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_LOCKSCREEN_SOUND_TO_SYSTEM, enabled, -1, name);
        msg.sendToTarget();
    }

    /**
     * Persist the active profile HapticFeedback status to system to make it effective immediately.
     */
    private void persistHapticFeedbackToSystem() {
        String name = Settings.System.HAPTIC_FEEDBACK_ENABLED;
        int enabled = getProfileState(mActiveProfileKey).mHapticFeedbackEnabled ? 1 : 0;
        Message msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_HAPTIC_FEEDBACK_TO_SYSTEM, enabled, -1, name);
        msg.sendToTarget();
    }

    /**
     * Persist profile settings to system by batch.
     * 
     * @param ops
     *            The contentValues to be persisted.
     */
    private void persistValuesToSystemByBatch(ArrayList<ContentProviderOperation> ops) {
        Message msg = mAudioProfileHandler.obtainMessage(MSG_PERSIST_VALUES_TO_SYSTEM_BY_BATCH, ops);
        msg.sendToTarget();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Other methods
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Checks whether this given profile is the active one.
     * 
     * @param profileKey
     *            The profile key .
     * @return True if the given profile is active, otherwise false.
     */
    public boolean isActive(String profileKey) {
        synchronized (mActiveProfileKey) {
            boolean actived = ((null != profileKey) && profileKey.equals(mActiveProfileKey));
            Log.d(TAG, "isActive: profileKey = " + profileKey + ", actived = " + actived);
            return actived;
        }
    }

    /**
     * Check the given uri whether exist.
     * 
     * @param uri
     * @return If the uri exist, return true, otherwise false.
     */
    public boolean isRingtoneExist(Uri uri) {
        try {
            AssetFileDescriptor fd = mContentResolver.openAssetFileDescriptor(uri, "r");
            if (fd == null) {
                return false;
            } else {
                fd.close();
                return true;
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return false;
        } catch (IOException e) {
            e.printStackTrace();
            return true;
        }
    }

    /**
     * Returns the default ringtone for a particular stream.
     * 
     * @param type
     *            The type whose default sound should be set. One of {@link #TYPE_RINGTONE},
     *            {@link #TYPE_NOTIFICATION} or {@link #TYPE_VIDEO_CALL}.
     * @return The default ringtone uri.
     * @see #setRingtoneUri(String, int, Uri)
     */
    public Uri getDefaultRingtone(int type) {
        Uri uri = null;
        switch (type) {
            case AudioProfileManager.TYPE_RINGTONE:
                uri = mDefaultRingtone.get(DEFAULT_RINGER_INDEX);
                break;

            case AudioProfileManager.TYPE_NOTIFICATION:
                uri = mDefaultRingtone.get(DEFAULT_NOTIFICATION_INDEX);
                break;

            case AudioProfileManager.TYPE_VIDEO_CALL:
                uri = mDefaultRingtone.get(DEFAULT_VIDEOCALL_INDEX);
                break;

            case AudioProfileManager.TYPE_SIP_CALL:
                uri = mDefaultRingtone.get(DEFAULT_SIPCALL_INDEX);
                break;
                
            default:
                Log.e(TAG, "getRingtoneUri with unsupport type!");
                return null;
        }
        Log.d(TAG, "getDefaultRingtone: type = " + type + ", default uri = " + uri);
        return uri;
    }

    /**
     * Read all the existed profiles' keys to mKey list from database.
     */
    private void readAllProfileKeys() {
        // Gets the predefined profiles' keys.
        mKeys.addAll(mPredefinedKeys);

        // Gets the custom profiles's keys.
        String nameColumn = Settings.System.NAME;
        String valueColumn = Settings.System.VALUE;

        String[] projection = new String[] { Settings.System._ID, valueColumn };

        String customPrefix = AudioProfileManager.getProfileKey(Scenario.CUSTOM);
        StringBuffer selection = new StringBuffer();
        selection.append(nameColumn).append(" like '").append(customPrefix).append("_%")
                .append(AudioProfileManager.SUFFIX_KEY).append("'");
        Log.d(TAG, "readProfileKeys: selection = " + selection.toString());

        ///M: CR ALPS01055175, query the profile record as add order.
        String sortOrder = Settings.System._ID + " asc";
        Cursor cursor =
                mContentResolver.query(Settings.System.CONTENT_URI, projection, selection.toString(), null, sortOrder);

        if (cursor == null) {
            Log.w(TAG, "getProfileKeys: Null custom cursor!");
            return;
        }
        int count = cursor.getCount();
        int valueIndex = cursor.getColumnIndex(valueColumn);
        String key;
        cursor.moveToFirst();
        for (int i = 0; i < count; i++) {
            key = cursor.getString(valueIndex);
            if (key != null && !key.isEmpty()) {
                mKeys.add(key);
            } else {
                Log.e(TAG, "readProfileKeys: Null custom key!");
            }
            cursor.moveToNext();
        }
        cursor.close();
        Log.d(TAG, "readProfileKeys: finised");
    }

    /**
     * Read four predefined profile keys: general, silent, meeting and outdoor
     */
    private void readPredefinedProfileKeys() {
        mPredefinedKeys.add(Scenario.GENERAL.ordinal(), AudioProfileManager.getProfileKey(Scenario.GENERAL));
        mPredefinedKeys.add(Scenario.SILENT.ordinal(), AudioProfileManager.getProfileKey(Scenario.SILENT));
        mPredefinedKeys.add(Scenario.MEETING.ordinal(), AudioProfileManager.getProfileKey(Scenario.MEETING));
        mPredefinedKeys.add(Scenario.OUTDOOR.ordinal(), AudioProfileManager.getProfileKey(Scenario.OUTDOOR));
        Log.d(TAG, "readPredefindProfileKeys: " + mPredefinedKeys);
    }

    /**
     * Reads the persisted settings to {@link mProfileStates} hash map.
     * 
     * @param profileKey
     *            The profile key.
     */
    private void readPersistedSettings(String profileKey, long simId) {
        if (profileKey == null) {
            Log.e(TAG, "readPersistedSettings with Null profile key!");
            return;
        }
        // query all value refer to profile key in database to be the init profile states values
        String[] projection = new String[] { Settings.System._ID, Settings.System.NAME, Settings.System.VALUE };
        String selection = Settings.System.NAME + " like '" + profileKey + "%'";
        Cursor cursor = mContentResolver.query(Settings.System.CONTENT_URI, projection, selection, null, null);
        HashMap<String, String> initValues = new HashMap<String, String>();
        if (cursor != null && cursor.moveToFirst()) {
            int nameIndex = cursor.getColumnIndex(Settings.System.NAME);
            int valueIndex = cursor.getColumnIndex(Settings.System.VALUE);
            do {
                String name = cursor.getString(nameIndex);
                String value = cursor.getString(valueIndex);
                initValues.put(name, value);
            } while (cursor.moveToNext());
        } else {
            Log.w(TAG, "readPersistedSettings: No value for " + profileKey);
        }

        if (cursor != null) {
            cursor.close();
        }

        String name = null;
        AudioProfileState defaultState = AudioProfileManager.getDefaultState(profileKey);
        // first get persisted Ringer,video Call and Notification uri
        // modify for sim different ringtone  @add {
        name = AudioProfileManager.getStreamUriKey(profileKey, AudioProfileManager.TYPE_RINGTONE, simId);
        // }
        Uri voiceCallUri = getPersistedValue(name, initValues,
                mDefaultRingtone.get(DEFAULT_RINGER_INDEX));

        name = AudioProfileManager.getStreamUriKey(profileKey, AudioProfileManager.TYPE_NOTIFICATION);
        Uri notificationUri = getPersistedValue(name, initValues,
                mDefaultRingtone.get(DEFAULT_NOTIFICATION_INDEX));

        name = AudioProfileManager.getStreamUriKey(profileKey, AudioProfileManager.TYPE_VIDEO_CALL, simId);
        Uri videoCallUri = getPersistedValue(name, initValues,
                mDefaultRingtone.get(DEFAULT_VIDEOCALL_INDEX));
        
        name = AudioProfileManager.getStreamUriKey(profileKey, AudioProfileManager.TYPE_SIP_CALL, simId);
        Uri sipCallUri = getPersistedValue(name, initValues,
                mDefaultRingtone.get(DEFAULT_SIPCALL_INDEX));

        // Second get persisted Ringer,Notification and alarm volume
        name = AudioProfileManager.getStreamVolumeKey(profileKey, AudioProfileManager.STREAM_RING);
        int ringerVolume = getPersistedValue(name, initValues, defaultState.mRingerVolume);

        name = AudioProfileManager.getStreamVolumeKey(profileKey, AudioProfileManager.STREAM_NOTIFICATION);
        int notificationVolume = getPersistedValue(name, initValues, defaultState.mNotificationVolume);

        name = AudioProfileManager.getStreamVolumeKey(profileKey, AudioProfileManager.STREAM_ALARM);
        int alarmVolume = getPersistedValue(name, initValues, defaultState.mAlarmVolume);

        // Third get persisted vibration,sound effect,dtmf tone,haptic feedback and lock screen
        // sound whether enabled
        name = AudioProfileManager.getVibrationKey(profileKey);
        boolean vibration = getPersistedValue(name, initValues, defaultState.mVibrationEnabled);

        name = AudioProfileManager.getDtmfToneKey(profileKey);
        boolean dtmfTone = getPersistedValue(name, initValues, defaultState.mDtmfToneEnabled);

        name = AudioProfileManager.getSoundEffectKey(profileKey);
        boolean soundEffect = getPersistedValue(name, initValues, defaultState.mSoundEffectEnbled);

        name = AudioProfileManager.getLockScreenKey(profileKey);
        boolean lockScreensound = getPersistedValue(name, initValues, defaultState.mLockScreenSoundEnabled);

        name = AudioProfileManager.getHapticKey(profileKey);
        boolean hapticFeedback = getPersistedValue(name, initValues, defaultState.mHapticFeedbackEnabled);

        // Put persisted state to mProfileStates
        AudioProfileState persistedState = new AudioProfileState.Builder(profileKey)
            .ringtone(voiceCallUri, notificationUri, videoCallUri, sipCallUri)
            .volume(ringerVolume, notificationVolume, alarmVolume)
            .vibration(vibration)
            .dtmfTone(dtmfTone)
            .soundEffect(soundEffect)
            .lockScreenSound(lockScreensound)
            .hapticFeedback(hapticFeedback)
            .simId(simId)
            .build();
        mProfileStates.put(profileKey, persistedState);
        // If profile is custom, put it profile name to mCustomProfileName
        if (Scenario.CUSTOM.equals(AudioProfileManager.getScenario(profileKey))) {
            name = AudioProfileManager.getProfileNameKey(profileKey);
            String profileName = initValues.get(name);
            mCustomProfileName.put(profileKey, profileName);
        }

        Log.d(TAG, "readPersistedSettings with " + profileKey + ": " + persistedState.toString());
    }
    /**
     * Reads the persisted settings to {@link mProfileStates} hash map.
     * 
     * @param profileKey
     *            The profile key.
     */
    private void readPersistedSettings(String profileKey) {
        readPersistedSettings(profileKey, mSingleSIM);
    }

    /**
     * Reads three type default ringtones: mDefaultRingtone[0] voice ringtone mDefaultRingtone[1]
     * notification ringtone mDefaultRingtone[2] video ringtone
     */
    private void readDefaultRingtones() {
        if (mDefaultRingtone.isEmpty()) {
            for (int i = 0; i < DEFAULT_RINGTONE_TYPE_CONUT; i++) {
                mDefaultRingtone.add(i, null);
            }
        }
        // mDefaultRingtone[0] voice ringtone
        String uriString = Settings.System.getString(mContentResolver, AudioProfileManager.KEY_DEFAULT_RINGTONE);
        Uri uri = (uriString == null ? null : Uri.parse(uriString));
        mDefaultRingtone.set(DEFAULT_RINGER_INDEX, uri);

        // mDefaultRingtone[1] notification ringtone
        uriString = Settings.System.getString(mContentResolver, AudioProfileManager.KEY_DEFAULT_NOTIFICATION);
        uri = (uriString == null ? null : Uri.parse(uriString));
        mDefaultRingtone.set(DEFAULT_NOTIFICATION_INDEX, uri);

        // mDefaultRingtone[2] video ringtone
        uriString = Settings.System.getString(mContentResolver, AudioProfileManager.KEY_DEFAULT_VIDEO_CALL);
        uri = (uriString == null ? null : Uri.parse(uriString));
        mDefaultRingtone.set(DEFAULT_VIDEOCALL_INDEX, uri);
        
        // mDefaultRingtone[3] sip ringtone
        uriString = Settings.System.getString(mContentResolver, AudioProfileManager.KEY_DEFAULT_SIP_CALL);
        uri = (uriString == null ? null : Uri.parse(uriString));
        mDefaultRingtone.set(DEFAULT_SIPCALL_INDEX, uri);
        
        Log.d(TAG, "readDefaultRingtones: " + mDefaultRingtone);
    }

    /**
     * Initial mShouldSyncToSystem, when initial AudioProfileService.if active profile is silent or
     * meeting, we should initial mShouldSyncToSystem to make sure it enable after power up again
     * 
     * @see #mShouldSyncToSystem
     */
    private void readShouldSyncToSystem() {
        final int size = ProfileSettings.values().length;
        for (int i = 0; i < size; i++) {
            mShouldSyncToSystem.add(false);
        }
        Scenario activeScenario = AudioProfileManager.getScenario(mActiveProfileKey);

        // We only need init mShouldSyncToSystem when active profile is silent or meeting(not CMCC)
        // and the profile's volumes(or ringtones) don't equal to system.
        if (Scenario.SILENT.equals(activeScenario) || (Scenario.MEETING.equals(activeScenario))) {
            // Volume
            int systemVolume = mAudioManager.getStreamVolume(AudioProfileManager.STREAM_RING);
            int profileVolume = getProfileState(mActiveProfileKey).mRingerVolume;
            if (profileVolume != systemVolume) {
                mShouldSyncToSystem.set(ProfileSettings.ringer_volume.ordinal(), true);
            }

            systemVolume = mAudioManager.getStreamVolume(AudioProfileManager.STREAM_NOTIFICATION);
            profileVolume = getProfileState(mActiveProfileKey).mNotificationVolume;
            if (profileVolume != systemVolume) {
                mShouldSyncToSystem.set(ProfileSettings.notification_volume.ordinal(), true);
            }

            systemVolume = mAudioManager.getStreamVolume(AudioProfileManager.STREAM_ALARM);
            profileVolume = getProfileState(mActiveProfileKey).mNotificationVolume;
            if (profileVolume != systemVolume) {
                mShouldSyncToSystem.set(ProfileSettings.alarm_volume.ordinal(), true);
            }

            // Ringtone
            Uri systemUri = RingtoneManager.getActualDefaultRingtoneUri(mContext, AudioProfileManager.TYPE_RINGTONE);
            Uri profileUri = getProfileState(mActiveProfileKey).mRingerStream;
            if ((profileUri != null && !profileUri.equals(systemUri)) || (profileUri == null && systemUri != null)) {
                mShouldSyncToSystem.set(DEFAULT_RINGER_INDEX, true);
            }

            systemUri = RingtoneManager.getActualDefaultRingtoneUri(mContext, AudioProfileManager.TYPE_NOTIFICATION);
            profileUri = getProfileState(mActiveProfileKey).mNotificationStream;
            if ((profileUri != null && !profileUri.equals(systemUri)) || (profileUri == null && systemUri != null)) {
                mShouldSyncToSystem.set(DEFAULT_NOTIFICATION_INDEX, true);
            }

            systemUri = RingtoneManager.getActualDefaultRingtoneUri(mContext, AudioProfileManager.TYPE_VIDEO_CALL);
            profileUri = getProfileState(mActiveProfileKey).mVideoCallStream;
            if ((profileUri != null && !profileUri.equals(systemUri)) || (profileUri == null && systemUri != null)) {
                mShouldSyncToSystem.set(DEFAULT_VIDEOCALL_INDEX, true);
            }
            
            systemUri = RingtoneManager.getActualDefaultRingtoneUri(mContext, AudioProfileManager.TYPE_SIP_CALL);
            profileUri = getProfileState(mActiveProfileKey).mSIPCallStream;
            if ((profileUri != null && !profileUri.equals(systemUri)) || (profileUri == null && systemUri != null)) {
                mShouldSyncToSystem.set(DEFAULT_SIPCALL_INDEX, true);
            }
        }
        Log.d(TAG, "readShouldSyncToSystem: mShouldSyncToSystem = " + mShouldSyncToSystem);
    }

    /**
     * Check the default profiles' settings. In FDD branch silent and meeting must be default value
     * and outdoor must be same as general except volume and vibration(outdoor has max volume and
     * always vibrate)
     */
    private void checkDefaultProfiles() {
        Log.d(TAG, "checkDefaultProfiles>>>");
        // Silent and meeting can not edit, restore it to be default
        restoreToDefaultValues(mPredefinedKeys.get(Scenario.SILENT.ordinal()));
        restoreToDefaultValues(mPredefinedKeys.get(Scenario.MEETING.ordinal()));

        // If outdoor can not edit, sync ringtones from general and set others with default values
        if (mExt.shouldSyncGeneralRingtoneToOutdoor()) {
            // Restore to default
            restoreToDefaultValues(mPredefinedKeys.get(Scenario.OUTDOOR.ordinal()));
            AudioProfileState generalState = mProfileStates.get(mPredefinedKeys.get(Scenario.GENERAL.ordinal()));
            AudioProfileState outdoorState = mProfileStates.get(mPredefinedKeys.get(Scenario.OUTDOOR.ordinal()));
            // Sync three type ringtones from general to outdoor
            outdoorState.mRingerStream = generalState.mRingerStream;
            outdoorState.mNotificationStream = generalState.mNotificationStream;
            outdoorState.mVideoCallStream = generalState.mVideoCallStream;
            outdoorState.mSIPCallStream = generalState.mSIPCallStream;
        }

        // Make sure notification volume is the same as ringer volume
        for (String profileKey : mKeys) {
            mProfileStates.get(profileKey).mNotificationVolume = mProfileStates.get(profileKey).mRingerVolume;
        }
        Log.d(TAG, "checkDefaultProfiles<<<");
    }

    // ezpt pp
    /**
     * Sync the ringer(also has notification) volume to profile. Because when other app change
     * system volume to non-zero at silent or meeting profile, it will cause ringermode change and
     * will make AudioProfileService to change profile to match this ringermode, so we should sync
     * the volume to profile volume to make profile's volume equal to system's. One of
     * {@link #TYPE_RINGTONE}, {@link #TYPE_NOTIFICATION}.
     * 
     * @param profileKey
     *            The profile key to sync volume.
     * @param volume
     *            The volume to sync to profile.
     */
    public void syncRingerVolumeToProfile(String profileKey, int volume) {
        if (getProfileState(profileKey).mRingerVolume != volume) {
            mAudioProfileHandler.removeMessages(MSG_PERSIST_RINGER_VOLUME_TO_DATABASE);
            mAudioProfileHandler.removeMessages(MSG_PERSIST_NOTIFICATION_VOLUME_TO_DATABASE);
            persistStreamVolumeToDatabase(profileKey, AudioProfileManager.STREAM_RING, volume);
            persistStreamVolumeToDatabase(profileKey, AudioProfileManager.STREAM_NOTIFICATION, volume);
            getProfileState(profileKey).mRingerVolume = volume;
            getProfileState(profileKey).mNotificationVolume = volume;
            Log.d(TAG, "syncRingerVolumeToProfile: profileKey = " + profileKey + ", volume = " + volume);
        }
    }

    /**
     * Gets the uri of particular stream from database.
     * 
     * @param name
     *            The name of the setting to be retrieved.
     * @param initValues
     *            A container that holds the settings' values of a profile get from database.
     * @param defaultUri
     *            The uri to be returned when the setting does't exist.
     * @return The persisted value if the setting exists in database, otherwise the defaultValue.
     */
    private Uri getPersistedValue(String name, HashMap<String, String> initValues, Uri defaultUri) {
        if (name != null) {
            String uriString = initValues.get(name);
            return (uriString == null ? defaultUri : Uri.parse(uriString));
        }
        return defaultUri;
    }

    /**
     * Gets the persisted setting from database.
     * 
     * @param name
     *            The name of the setting to be retrieved.
     * @param initValues
     *            A container that holds the settings' values of a profile get from database.
     * @param defaultValue
     *            The value to be returned when the setting does't exist.
     * @return The persisted value if the setting exists in database, otherwise the defaultValue.
     */
    private int getPersistedValue(String name, HashMap<String, String> initValues, int defaultValue) {
        if (name != null) {
            String value = initValues.get(name);
            return (value == null ? defaultValue : Integer.valueOf(value));
        }
        return defaultValue;
    }

    /**
     * Gets the persisted setting from database.
     * 
     * @param name
     *            The name of the setting to be retrieved.
     * @param initValues
     *            A container that holds the settings' values of a profile get from database.
     * @param defaultValue
     *            The value to be returned when the setting does't exist.
     * @return The persisted value if the setting exists in database, otherwise the defaultValue.
     */
    private boolean getPersistedValue(String name, HashMap<String, String> initValues, boolean defaultValue) {
        if (name != null) {
            String value = initValues.get(name);
            return (value == null ? defaultValue : Boolean.valueOf(value));
        }
        return defaultValue;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Register a AudiopPofile listener callback to AudioProfileService
    ///////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Register the IAudioProfileListener callback to AudioProfileService to listen AudioProfile
     * changed.
     * 
     * @param callback
     *            AudioProfileListener callback.
     * @param event
     *            The event for listener.
     * 
     * */
    public void listenAudioProfie(IAudioProfileListener callback, int event) {
        if (event == 0) {
            remove(callback.asBinder());
            Log.d(TAG, "listenAudioProfie with LISTEN_NONE, so remove this listener's callback!");
            return;
        }

        synchronized (mRecords) {
            // register callback in AudioProfileService, if the callback is exist,
            // just replace the event.
            Record record = null;
            find_and_add: {
                IBinder binder = callback.asBinder();
                int size = mRecords.size();
                for (int i = 0; i < size; i++) {
                    record = mRecords.get(i);
                    if (binder == record.mBinder) {
                        break find_and_add;
                    }
                }
                record = new Record();
                record.mBinder = binder;
                record.mCallback = callback;
                mRecords.add(record);
            }
            record.mEvent = event;

            if (event == AudioProfileListener.LISTEN_AUDIOPROFILE_CHANGEG) {
                try {
                    record.mCallback.onAudioProfileChanged(mActiveProfileKey);
                } catch (RemoteException e) {
                    remove(record.mBinder);
                    Log.e(TAG, "Dead object in listenAudioProfie, remove listener's callback!" + e);
                }
            }
            /// APP need not notify the change when it register the listener.
            /* else if (event == AudioProfileListener.LISTEN_RINGER_VOLUME_CHANGED) {
                try {
                    record.mCallback.onRingerVolumeChanged(getProfileState(mActiveProfileKey).mRingerVolume,
                            getProfileState(mActiveProfileKey).mRingerVolume, mActiveProfileKey);
                } catch (RemoteException e) {
                    remove(record.mBinder);
                    Log.e(TAG, "Dead object in listenAudioProfie, remove listener's callback!" + e);
                }
            }*/
            Log.d(TAG, "listenAudioProfie with event = " + event + " sucessed, record.mBinder = " + record.mBinder
                    + " ,clients = " + mRecords.size());
        }

    }

    /**
     * M: Set current uid.
     * @param userId Uid
     *
     */
    public void setUserId (int userId) {
        mUserId = userId;
    }

    private void remove(IBinder binder) {
        synchronized (mRecords) {
            Iterator<Record> iterator = mRecords.iterator();
            while (iterator.hasNext()) {
                Record record = (Record) iterator.next();
                if (record.mBinder.equals(binder)) {
                    iterator.remove();
                    Log.d(TAG, "removed AudioProfile change listener for: record.mBinder = " + record.mBinder
                            + ", clients = " + mRecords.size());
                }
            }
        }
    }

    private static class Record {

        IBinder mBinder;

        IAudioProfileListener mCallback;

        int mEvent;
    }

    // /////////////////////////////////////////////////////////////////////////////////////////////
    // AudoProfile handler to handle persisted message
    // /////////////////////////////////////////////////////////////////////////////////////////////

    /** Thread that handles persist values override system. */
    private class OverrideSystemThread extends Thread {
        OverrideSystemThread() {
            super("AudioProfileService");
        }

        @Override
        public void run() {
            // Set this thread up so the handler will work on it
            Looper.prepare();

            synchronized (AudioProfileService.this) {
                mAudioProfileHandler = new AudioProfileHandler();

                // Notify that the handler has been created
                AudioProfileService.this.notify();
            }

            // Listen for volume change requests that are set by VolumePanel
            Looper.loop();
        }
    }

    private void createOverrideSystemThread() {
        OverrideSystemThread overrideSystemThread = new OverrideSystemThread();
        overrideSystemThread.start();

        synchronized (this) {
            while (mAudioProfileHandler == null) {
                try {
                    // Wait for mAudioProfileHandler to be set by the other thread
                    wait();
                } catch (InterruptedException e) {
                    Log.e(TAG, "Interrupted while waiting on AudioProfileHandler.");
                }
            }
        }

    }

    class AudioProfileHandler extends Handler {

        @Override
        public void handleMessage(Message msg) {
            String name = (String) msg.obj;
            int valueInt = msg.arg1;
            String valueSting = null;
            int simId = mSingleSIM;
            Bundle bundle = msg.getData();
            if (null != bundle) {
                valueSting = bundle.getString(name);
                simId = bundle.getInt(name);
            }
            Log.d(TAG, "handleMessage what = " + msg.what + ", name = " + name + ", value = " + valueInt + " or "
                    + valueSting);
            switch (msg.what) {
                case MSG_PERSIST_VOICECALL_RINGTONE_TO_SYSTEM:
                    RingtoneManager.setActualDefaultRingtoneUri(mContext, AudioProfileManager.TYPE_RINGTONE,
                            (valueSting == null ? null : Uri.parse(valueSting)));
                    break;

                case MSG_PERSIST_NOTIFICATION_RINGTONE_TO_SYSTEM:
                    RingtoneManager.setActualDefaultRingtoneUri(mContext, AudioProfileManager.TYPE_NOTIFICATION,
                            (valueSting == null ? null : Uri.parse(valueSting)));
                    break;

                case MSG_PERSIST_VIDEOCALL_RINGTONE_TO_SYSTEM:
                    RingtoneManager.setActualDefaultRingtoneUri(mContext, AudioProfileManager.TYPE_VIDEO_CALL,
                            (valueSting == null ? null : Uri.parse(valueSting)));
                    break;

                case MSG_PERSIST_DTMF_TONE_TO_SYSTEM:
                    Settings.System.putInt(mContentResolver, name, msg.arg1);
                    break;

                case MSG_PERSIST_SOUND_EFFECT_TO_SYSTEM:
                    Settings.System.putInt(mContentResolver, name, msg.arg1);
                    break;

                case MSG_PERSIST_LOCKSCREEN_SOUND_TO_SYSTEM:
                    Settings.System.putInt(mContentResolver, name, msg.arg1);
                    break;

                case MSG_PERSIST_HAPTIC_FEEDBACK_TO_SYSTEM:
                    Settings.System.putIntForUser(mContentResolver, name, msg.arg1,UserHandle.USER_CURRENT);
                    break;

                // Persist value to database in handler to avoid ANR.
                case MSG_PERSIST_RINGER_VOLUME_TO_DATABASE:
                    Settings.System.putInt(mContentResolver, name, valueInt);
                    break;

                case MSG_PERSIST_NOTIFICATION_VOLUME_TO_DATABASE:
                    Settings.System.putInt(mContentResolver, name, valueInt);
                    break;

                case MSG_PERSIST_ALARM_VOLUME_TO_DATABASE:
                    Settings.System.putInt(mContentResolver, name, valueInt);
                    break;

                case MSG_PERSIST_VOICECALL_RINGTONE_TO_DATABASE:
                    Settings.System.putString(mContentResolver, name, valueSting);
                    Settings.System.putString(mContentResolver, AudioProfileManager.getDataKey(name),
                            getExternalUriData(valueSting));
                    break;

                case MSG_PERSIST_NOTIFICATION_RINGTONE_TO_DATABASE:
                    Settings.System.putString(mContentResolver, name, valueSting);
                    Settings.System.putString(mContentResolver, AudioProfileManager.getDataKey(name),
                            getExternalUriData(valueSting));
                    break;

                case MSG_PERSIST_VIDEOCALL_RINGTONE_TO_DATABASE:
                    Settings.System.putString(mContentResolver, name, valueSting);
                    Settings.System.putString(mContentResolver, AudioProfileManager.getDataKey(name),
                            getExternalUriData(valueSting));
                    break;

                case MSG_PERSIST_VIBRATION_TO_DATABASE:
                    Settings.System.putString(mContentResolver, name, valueSting);
                    break;

                case MSG_PERSIST_DTMF_TONE_TO_DATABASE:
                    Settings.System.putString(mContentResolver, name, valueSting);
                    break;

                case MSG_PERSIST_SOUND_EFFECT_TO_DATABASE:
                    Settings.System.putString(mContentResolver, name, valueSting);
                    break;

                case MSG_PERSIST_LOCKSCREEN_SOUND_TO_DATABASE:
                    Settings.System.putString(mContentResolver, name, valueSting);
                    break;

                case MSG_PERSIST_HAPTIC_FEEDBACK_TO_DATABASE:
                    Settings.System.putString(mContentResolver, name, valueSting);
                    break;

                case MSG_PERSIST_PROFILE_NAME_TO_DATABASE:
                    Settings.System.putString(mContentResolver, name, valueSting);
                    break;

                case MSG_DELAY_SET_VIBRATE_AVOID_CTS_FAIL:
                    mDelaySetVibrate = false;
                    int vibrationStatus = getProfileState(mActiveProfileKey).mVibrationEnabled ?
                            AudioManager.VIBRATE_SETTING_ON : AudioManager.VIBRATE_SETTING_ONLY_SILENT;

                    mAudioManager.setVibrateSetting(AudioManager.VIBRATE_TYPE_RINGER, vibrationStatus);
                    mAudioManager.setVibrateSetting(AudioManager.VIBRATE_TYPE_NOTIFICATION, vibrationStatus);
                    Log.d(TAG, "CTS test finish, set vibrate again to make function normal!");
                    break;
                    
                case MSG_PERSIST_SIPCALL_RINGTONE_TO_SYSTEM:
                    RingtoneManager.setActualDefaultRingtoneUri(mContext, AudioProfileManager.TYPE_SIP_CALL,
                            (valueSting == null ? null : Uri.parse(valueSting)));
                    break;

                case MSG_PERSIST_SIPCALL_RINGTONE_TO_DATABASE:
                    Settings.System.putString(mContentResolver, name, valueSting);
                    Settings.System.putString(mContentResolver, AudioProfileManager.getDataKey(name),
                            getExternalUriData(valueSting));
                    break;
                    
                default:
                    Log.e(TAG, "Unsupport handle message: what = " + msg.what);
                    break;
            }
        }
    }

    /**
     * Get the uri data path.
     * 
     * @param uri
     * @return
     */
    private String getExternalUriData(String uriString) {
        if (null == uriString) {
            return null;
        }

        String data = null;
        Cursor cursor = mContentResolver.query(
                Uri.parse(uriString),
                new String[] { MediaStore.Audio.Media.DATA },
                null,
                null,
                null);
        try {
            if (null != cursor && cursor.moveToFirst()) {
                data = cursor.getString(0);
            }
        } finally {
            if (null != cursor) {
                cursor.close();
                cursor = null;
            }
        }
        Log.d(TAG, "getExternalUriData for " + uriString + " with data: " + data);
        return data;
    }
}
