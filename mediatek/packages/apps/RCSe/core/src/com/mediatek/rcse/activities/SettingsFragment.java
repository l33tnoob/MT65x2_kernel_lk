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

package com.mediatek.rcse.activities;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.ProgressDialog;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.content.res.Resources;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.SystemClock;
import android.os.Vibrator;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceCategory;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.preference.RingtonePreference;
import android.text.InputFilter;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.fragments.RcseSystemSettingsFragment;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.plugin.apn.RcseOnlyApnUtils;
import com.mediatek.rcse.plugin.message.IpNotificationsManager;
import com.mediatek.rcse.plugin.message.PluginUtils;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.utils.PhoneUtils;

import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Settings fragment to setup settings.
 */
public class SettingsFragment extends PreferenceFragment implements
        OnSharedPreferenceChangeListener {

    private static final String TAG = "SettingsFragment";
    public static final String RCS_REMIND = "rcse_large_file_reminder";
    private CheckBoxPreference mRemindCheckBox = null;
    private CheckBoxPreference mDisableJoyn = null;
    private PreferenceScreen mClearHistoryItem = null;
    private PreferenceScreen mJoynUserAlias = null;
    private PreferenceScreen mBlockItem = null;
    private CheckBoxPreference ftAutoPreRoaming = null;
    private CheckBoxPreference ftAutoPreNonRoaming = null;    
    private CheckBoxPreference mNotificationCheckBox = null;
    private RingtonePreference mNotificationRingtone = null;
    private CheckBoxPreference mVibrateCheckBox = null;
    private CheckBoxPreference mSendReadReceiptCheckBox = null;
    private static final String RCS_BLOCK_ITEM = "rcs_block_list";
    private static final String RCS_CLEAR_HISTORY = "rcse_clear_history";
    private static final String RCS_DISABLE_JOYN = "rcse_disable_joyn";
    private static final String JOYN_USER_ALIAS = "joyn_user_alias";
    private static final String RCS_NOTIFY = "rcse_notify";
    private static final String RCS_NOTIFICATION = "rcse_notification";
    private static final String RCS_RINGTONE = "chat_invitation_ringtone";
    private static final String RCS_VIBRATE = "chat_invitation_vibration";
    private static final String RCS_GENERAL = "rcse_general";
    private static final String RCS_MESSAGE_SETTINGS = "rcse_message_settings";
    private static final String RCS_CHAT_WALLPAPER = "rcse_chat_wallpaper";
    private static final String RCS_PRIVACY = "rcse_privacy";
    public static final String RCS_SEND_READ_RECEIPT = "rcse_send_read_receipt";
    private static final int CLEAR_HISTORY_TIME_OUT = 1000;
    private static final int CLEAR_HISTORY_MESSAGE_ID = 0;
    private static final String RCSE_COMPRESS_IMAGE = "rcse_compress_image";
    private static final int INVALID_VALUE = -1;
    private static final int FIRST_POSITION = 0;
    private static final String EXTRA_KEY_FROM_MMS = "extraKeyFromMms";
    private static final String EXTRA_VALUE_FROM_MMS = "extraValueFromMms";
    private static final String PREFERENCE_FT_ACCEPT_ROAMING = "rcse_ftautoaccept_roaming";
    private static final String PREFERENCE_FT_ACCEPT_NOROAMING = "rcse_ftautoaccept_nonroaming";
    public static String alias_Name= "";
    public static final String SETTINGS_PREFERENCES = "UserPreferences";
    private static int ONLY_USE_XMS_OFF = 0;
    private static int ONLY_USE_XMS = 1;
    

    public String getAlias_Name() {
    	String aliasName;
    	if(alias_Name.equals(""))
    	{
    		aliasName=  mSettings.getString(JOYN_USER_ALIAS, "");
    		Logger.d(TAG, "setAlias_Name(): " + aliasName);
    		return aliasName;
    	}
    	else
    		return alias_Name;
		
	}

	public void setAlias_Name(String alias_name) {	
		alias_Name = alias_name;
		Logger.d(TAG, "setAlias_Name(): " + alias_name);
		//mSettings = PreferenceManager.getDefaultSharedPreferences(AndroidFa);
		mSettings.edit().putString(JOYN_USER_ALIAS, alias_name).apply();		
		RcsSettings.getInstance().setJoynUserAlies(alias_name);
	}


    /**
     * M: Add for APN test tools. @{
     */
    private static final String RCS_APN_TOOL = "rcse_apn_enable";
    private static final String RCS_FORCEUSE_ONLYAPN = "roaming_enable";
    private static final String PREFERENCE_TEST_TOOL = "rcse_apn_test_tools";
    private CheckBoxPreference mApnCheckBox = null;
    private CheckBoxPreference mRoamingCheckBox = null;
    /**
     * @}
     */
    
    private ConfirmDialogOfClearhistory mDialog = new ConfirmDialogOfClearhistory();
    public static final AtomicBoolean IS_NOTIFICATION_CHECKED = new AtomicBoolean(true);
    private ClearHistoryProgress mClearHistoryProgress;
    private ClearHistoryTask mClearHistoryTask = null;
    private SharedPreferences mSettings = null;
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(android.os.Message msg) {
            if (msg.what == CLEAR_HISTORY_MESSAGE_ID) {
                if (mClearHistoryTask != null && mClearHistoryProgress != null
                        && !mClearHistoryTask.isCancelled()) {
                    mClearHistoryProgress.dismissAllowingStateLoss();
                    Activity activity = getActivity();
                    if (null != activity) {
                        Toast.makeText(activity.getApplicationContext(),
                                getString(R.string.rcse_clear_chat_history_completed),
                                Toast.LENGTH_SHORT).show();
                    } else {
                        Logger.e(TAG, "mHandler->handleMessage() activity is null");
                    }
                } else {
                    Logger.d(TAG, "handleMessage, the task has been canceled!");
                }
            } else {
                Logger.d(TAG, "handleMessage, msg.what = " + msg.what);
            }
        };
    };

    private CheckBoxPreference mCompressImageCheckBox = null;
    private final RcseSettingContentObserver mRcseSettingContentObserver = new RcseSettingContentObserver();

    private class RcseSettingContentObserver extends ContentObserver {
        @Override
        public void onChange(boolean selfChange) {
            Logger.d(TAG, "onChange(): " + selfChange);
            new ChangeCompressStatusTask().execute();
        }

        public RcseSettingContentObserver() {
            super(new Handler());
        }
    }

    public SettingsFragment() {

    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        Logger.v(TAG, "onActivityCreated() entry");
        super.onActivityCreated(savedInstanceState);

        Bundle bundle = getActivity().getIntent().getExtras();
        if (null != bundle) {
            if (EXTRA_VALUE_FROM_MMS.equals(bundle.getString(EXTRA_KEY_FROM_MMS))) {
                Logger.d(TAG, "onActivityCreated() intent from mms");
                removeUnusePreferences();
            } else {
                Logger.d(TAG, "onActivityCreated() intent not from mms");
            }
        } else {
            Logger.d(TAG, "onActivityCreated() intent bundle is null");
        }

        Logger.v(TAG, "onActivityCreated() exit");
    }

    private void removeUnusePreferences() {
        Logger.d(TAG, "removeUnusePreferences() entry");
        PreferenceCategory notifyCategory = (PreferenceCategory) findPreference(RCS_NOTIFY);
        notifyCategory.removeAll();
        getPreferenceScreen().removePreference(notifyCategory);

        
        PreferenceCategory generalCategory = (PreferenceCategory) findPreference(RCS_GENERAL);
        //getPreferenceScreen().removePreference(generalCategory);
        generalCategory.removePreference(findPreference(RCS_CHAT_WALLPAPER));

        PreferenceCategory privacyCategory = (PreferenceCategory) findPreference(RCS_PRIVACY);
        privacyCategory.removePreference(findPreference(RCS_BLOCK_ITEM));
        privacyCategory.removePreference(findPreference(RCS_CLEAR_HISTORY));
        Logger.d(TAG, "removeUnusePreferences() exit");
    }

    @Override
    public void onDestroy() {
        Logger.d(TAG, "onDestroy()");
        super.onDestroy();
        Activity activity = getActivity();
        Logger.e(TAG, "activity = " + activity);
        if (activity == null) {
            Logger.e(TAG, "return since activity is null");
            return;
        }
        ContentResolver resolver = activity.getContentResolver();
        Logger.e(TAG, "resolver = " + resolver);
        if (resolver == null) {
            Logger.e(TAG, "return since resolver is null");
            return;
        }
        resolver.unregisterContentObserver(mRcseSettingContentObserver);
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        Logger.d(TAG, "onHiddenChanged()  hidden is " + hidden);
        super.onHiddenChanged(hidden);
        if (!hidden) {
            new GetBlockNum().execute();
        }
    }

    public void disableAllPreferences()
    {
    	 mRemindCheckBox.setEnabled(false);
    	 mDisableJoyn.setEnabled(false);
    	 mBlockItem.setEnabled(false);
    	 mClearHistoryItem.setEnabled(false);
    	 mJoynUserAlias.setEnabled(false);
    	 mNotificationCheckBox.setEnabled(false);
    	 mNotificationRingtone.setEnabled(false);
    	 mSendReadReceiptCheckBox.setEnabled(false);  
    	 mCompressImageCheckBox.setEnabled(false);
    	 mSendReadReceiptCheckBox.setEnabled(false);
    	 if (PhoneUtils.sIsApnDebug) {
    	 	mApnCheckBox.setEnabled(false);
    	 	mRoamingCheckBox.setEnabled(false);
    	 }
    	 mCompressImageCheckBox.setEnabled(false);
    	 mVibrateCheckBox.setEnabled(false);
    	 ftAutoPreNonRoaming.setEnabled(false);
    	 ftAutoPreRoaming.setEnabled(false);    	 
    }
    
    public void enableAllPreferences()
    {
    	 mRemindCheckBox.setEnabled(true);
    	 mDisableJoyn.setEnabled(true);
    	 mBlockItem.setEnabled(true);
    	 mClearHistoryItem.setEnabled(true);
    	 mNotificationCheckBox.setEnabled(true);
    	 mJoynUserAlias.setEnabled(true);
    	 mNotificationRingtone.setEnabled(true);
    	 mSendReadReceiptCheckBox.setEnabled(true); 
    	 mCompressImageCheckBox.setEnabled(true);
    	 mSendReadReceiptCheckBox.setEnabled(true);
    	 if (PhoneUtils.sIsApnDebug) {
    	 	mApnCheckBox.setEnabled(true);
    	 	mRoamingCheckBox.setEnabled(true);
    	 }    	 
    	 mCompressImageCheckBox.setEnabled(true);
    	 mVibrateCheckBox.setEnabled(true);
    	 ftAutoPreNonRoaming.setEnabled(true);
    	 ftAutoPreRoaming.setEnabled(true);    	 
    	 
    }
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Logger.d(TAG, "onCreate() entry");
        mSettings = getActivity().getSharedPreferences(SETTINGS_PREFERENCES, Activity.MODE_APPEND);
       // mSettings = PreferenceManager.getDefaultSharedPreferences(getActivity());
        addPreferencesFromResource(R.xml.preference_rcs_settings);

        /*Added for Disable Joyn in case of Full Integrated Mode*/
        mDisableJoyn = (CheckBoxPreference) findPreference(RCS_DISABLE_JOYN);
        mDisableJoyn.setOnPreferenceClickListener(new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                boolean retValue = false;
                if (preference == null) {
                    Logger.d(TAG, "onPreferenceClick()-preference is null");
                } else {
                    String key = preference.getKey();
                    if (key == null) {
                        Logger.d(TAG, "onPreferenceClick()-key is null");
                    } else {
                        if (RCS_DISABLE_JOYN.equals(key)) {
                            if (mDisableJoyn != null) {
                                boolean isChecked = mDisableJoyn.isChecked();
                                Logger.d(TAG,
                                        "onPreferenceClick(), the status of mDisableJoyn is "
                                                + isChecked);
                                mSettings.edit().putBoolean(RCS_DISABLE_JOYN, isChecked)
                                        .apply();
                                RcsSettings.getInstance().setJoynMessagingDisabledFullyIntegrated(isChecked);
                                retValue = true;
                                Intent it = new Intent();
                                it.setAction(IpMessageConsts.IsOnlyUseXms.ACTION_ONLY_USE_XMS);
                                if(isChecked)
                                    it.putExtra("status", ONLY_USE_XMS);
                                else 
                                	it.putExtra("status",ONLY_USE_XMS_OFF);
                                AndroidFactory.getApplicationContext().sendStickyBroadcast(it);
                            } else {
                                Logger.d(TAG,
                                        "onPreferenceClick(),mDisableJoyn is null");
                            }
                        } else {
                            Logger.d(TAG,
                                    "onPreferenceClick()-key is not equal RCS_DISABLE_JOYN");
                        }
                    }
                }
                return retValue;
            }
        });       
       
		if (PluginUtils.getMessagingMode() == 0) {
			Logger.d(TAG,
                    "Converged mode remove Disbale joyn Messaging");
			PreferenceCategory messageSettingsCategory = (PreferenceCategory) findPreference(RCS_MESSAGE_SETTINGS);
			messageSettingsCategory.removeAll();
		    getPreferenceScreen().removePreference(messageSettingsCategory);
		}
       
        mRemindCheckBox = (CheckBoxPreference) findPreference(RCS_REMIND);
        mBlockItem = (PreferenceScreen) findPreference(RCS_BLOCK_ITEM);
        mClearHistoryItem = (PreferenceScreen) findPreference(RCS_CLEAR_HISTORY);
        mJoynUserAlias = (PreferenceScreen) findPreference(JOYN_USER_ALIAS); 
        String alias = mSettings.getString(JOYN_USER_ALIAS, "");
        mJoynUserAlias.setSummary(alias);
        mNotificationCheckBox = (CheckBoxPreference) findPreference(RCS_NOTIFICATION);
        mNotificationRingtone = (RingtonePreference) findPreference(RCS_RINGTONE);
        /**
         * M: FT Auto accept settings and their preference click listener @{
         */ 
        PreferenceCategory generalCategory = (PreferenceCategory) findPreference(RCS_GENERAL);
        ftAutoPreRoaming = ((CheckBoxPreference) findPreference(PREFERENCE_FT_ACCEPT_ROAMING));
        ftAutoPreNonRoaming = (CheckBoxPreference)findPreference(PREFERENCE_FT_ACCEPT_NOROAMING);
        if(!RcsSettings.getInstance().isFileTransferAutoAccepted()){//if ftAuto disable should hide
            Logger.d(TAG, "isFileTransferAutoAccepted false");
            generalCategory.removePreference(ftAutoPreRoaming);
            generalCategory.removePreference(ftAutoPreNonRoaming);
        }else{
        	Logger.d(TAG, "isFileTransferAutoAccepted true");
              ftAutoPreRoaming.setChecked(RcsSettings.getInstance()
                        .isEnableFtAutoAcceptWhenRoaming());
              ftAutoPreNonRoaming.setChecked(RcsSettings.getInstance()
                           .isEnableFtAutoAcceptWhenNoRoaming());
         ftAutoPreRoaming.setOnPreferenceClickListener(new OnPreferenceClickListener() {
                    @Override
                    public boolean onPreferenceClick(Preference preference) {
                 final boolean checked = ftAutoPreRoaming.isChecked();
                                RcsSettings.getInstance().setEnableFtAutoAcceptWhenRoaming(checked);
                    return true;
                            }
                        });
         ftAutoPreNonRoaming.setOnPreferenceClickListener(new OnPreferenceClickListener() {
              @Override
              public boolean onPreferenceClick(Preference preference) {
                 final boolean checked = ftAutoPreNonRoaming.isChecked();
                                    RcsSettings.getInstance().setEnableFtAutoAcceptWhenNoRoaming(checked);
                        return true;
                    }
                });
         }
        /**
         * @}
         */
        mSendReadReceiptCheckBox = (CheckBoxPreference) findPreference(RCS_SEND_READ_RECEIPT);
        mSendReadReceiptCheckBox.setOnPreferenceClickListener(new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                boolean retValue = false;
                if (preference == null) {
                    Logger.d(TAG, "onPreferenceClick()-preference is null");
                } else {
                    String key = preference.getKey();
                    if (key == null) {
                        Logger.d(TAG, "onPreferenceClick()-key is null");
                    } else {
                        if (RCS_SEND_READ_RECEIPT.equals(key)) {
                            if (mSendReadReceiptCheckBox != null) {
                                boolean isChecked = mSendReadReceiptCheckBox.isChecked();
                                Logger.d(TAG,
                                        "onPreferenceClick(), the status of mSendReadReceiptCheckBox is "
                                                + isChecked);
                                mSettings.edit().putBoolean(RCS_SEND_READ_RECEIPT, isChecked)
                                        .apply();
                                retValue = true;
                            } else {
                                Logger.d(TAG,
                                        "onPreferenceClick(),mSendReadReceiptCheckBox is null");
                            }
                        } else {
                            Logger.d(TAG,
                                    "onPreferenceClick()-key is not equal RCS_SEND_READ_RECEIPT");
                        }
                    }
                }
                return retValue;
            }
        });
        /**
         * M: Add for RCS-e only APN test tools.@{
         */
        if (PhoneUtils.sIsApnDebug) {
            mApnCheckBox = (CheckBoxPreference) findPreference(RCS_APN_TOOL);
            mApnCheckBox.setOnPreferenceClickListener(new OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference preference) {
                    final boolean checked = mApnCheckBox.isChecked();
                    RcsSettings.getInstance().setRcseOnlyApnState(checked);
                    return true;
                }
            });
            mRoamingCheckBox = (CheckBoxPreference) findPreference(RCS_FORCEUSE_ONLYAPN);
            boolean forceUse = mSettings.getBoolean("forceUse", false);
            Context context = getActivity();
            Intent onlyApnIntent = new Intent();
            onlyApnIntent.setAction(LauncherUtils.DEBUG_FORCEUSE_ONLYAPN_ACTION);
            onlyApnIntent.putExtra("forceUse", forceUse);
            Logger.d(TAG, "mRoamingCheckBox context: " + context + " forceUse: " + forceUse);
            if (context != null) {
                context.getApplicationContext().sendBroadcast(onlyApnIntent);
            }
            mRoamingCheckBox.setOnPreferenceClickListener(new OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference preference) {
                    Context context = getActivity();
                    final boolean checked = mRoamingCheckBox.isChecked();
                    Intent onlyApnIntent = new Intent();
                    onlyApnIntent.setAction(LauncherUtils.DEBUG_FORCEUSE_ONLYAPN_ACTION);
                    onlyApnIntent.putExtra("forceUse", checked);
                    Logger.d(TAG, "mRoamingCheckBox context: " + context + " checked: " + checked);
                    if (context != null) {
                        context.getApplicationContext().sendBroadcast(onlyApnIntent);
                    }
                    // because it will be used in both UI process and core
                    // process, so we should make sure the status is correct in
                    // both processes.
                    mSettings.edit().putBoolean("forceUse", checked).apply();
                    return true;
                }
            });
        } else {
            Preference testPreference = getPreferenceManager().findPreference(PREFERENCE_TEST_TOOL);
            getPreferenceScreen().removePreference(testPreference);
        }
        
        /**
         * @}
         */
        boolean isSendReadReceiptChecked = mSettings.getBoolean(RCS_SEND_READ_RECEIPT, true);
        mSendReadReceiptCheckBox.setChecked(isSendReadReceiptChecked);
        SharedPreferences prefs =
                mNotificationRingtone.getPreferenceManager().getSharedPreferences();
        prefs.edit().putString(RCS_RINGTONE, RcsSettings.getInstance().getChatInvitationRingtone())
                .apply();

        mNotificationRingtone.setOnPreferenceClickListener(new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Logger.d(TAG, "onPreferenceClick()-preference is " + preference);
                boolean retValue = false;
                if (preference != null) {
                    String key = preference.getKey();
                    Logger.d(TAG, "onPreferenceClick()-key is " + key);
                    if (RCS_RINGTONE.equals(key)) {
                        
                        SharedPreferences prefs = 
                                mNotificationRingtone.getPreferenceManager().getSharedPreferences();
                        String uri = prefs.getString(RCS_RINGTONE, "");
                        RcsSettings.getInstance().setChatInvitationRingtone(uri);
                        retValue = true;
                    }
                }
                return retValue;
            }
        });

        mVibrateCheckBox = (CheckBoxPreference) findPreference(RCS_VIBRATE);
        Vibrator vibrator = (Vibrator) getActivity().getSystemService(Context.VIBRATOR_SERVICE);
        if (!vibrator.hasVibrator()) {
            Logger.d(TAG, "There is not a vibrator at present. So remove the preference.");
            PreferenceCategory notificationsCategory =
                    (PreferenceCategory) findPreference(RCS_NOTIFY);
            notificationsCategory.removePreference(mVibrateCheckBox);
        } else {
            Logger.d(TAG, "There is a vibrator at present.");
        }
        mCompressImageCheckBox = (CheckBoxPreference) findPreference(RCSE_COMPRESS_IMAGE);
        mCompressImageCheckBox.setChecked(RcsSettings.getInstance()
                                        .isEnabledCompressingImageFromDB());
        mCompressImageCheckBox.setOnPreferenceClickListener(new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Logger.d(TAG, "onPreferenceClick()-preference is " + preference);
                boolean retValue = false;
                if (preference != null) {
                    String key = preference.getKey();
                    Logger.d(TAG, "onPreferenceClick()-key is " + key);
                    if (RCSE_COMPRESS_IMAGE.equals(key)) {
                        final boolean isChecked = mCompressImageCheckBox.isChecked();
                        Logger.d(TAG,
                                "onPreferenceClick(), the status of mSendReadReceiptCheckBox is "
                                        + isChecked);
                        retValue = true;
                        AsyncTask.execute(new Runnable() {
                            @Override
                            public void run() {
                                boolean isEnableCompressedPrev = RcsSettings.getInstance()
                                        .isEnabledCompressingImageFromDB();
                                RcsSettings.getInstance().setCompressingImage(isChecked);
                                Logger.d(TAG, "onPreferenceClick():isChecked " + isChecked
                                        + ",isEnableCompressedPrev = " + isEnableCompressedPrev);
                                if (isEnableCompressedPrev == true
                                        && isChecked != isEnableCompressedPrev) {
                                    RcsSettings.getInstance().saveRemindCompressFlag(false);
                                }
                            }
                        });
                    }
                }
                return retValue;
            }
        });
        new GetCompressStatusTask().execute();
        
        //Added to grey-out rcs settings when joyn is deactivated
        
		if (!mSettings.getBoolean("rcse_master_switch", false)) {
			Logger.d(TAG, "isMasterSwitch OFF() entry");
			disableAllPreferences();
		} else {
			Logger.d(TAG, "isMasterSwitch ON() entry");
			enableAllPreferences();
		}
	
        getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if (preference == mClearHistoryItem) {
            if (mClearHistoryItem != null) {
                mDialog.show(getFragmentManager(), ConfirmDialogOfClearhistory.TAG);
            } else {
                Logger.d(TAG, "onPreferenceTreeClick(), mClearHistory is null");
            }
        }
        if(preference == mJoynUserAlias)
        {
        	if (mJoynUserAlias != null) {
               showAliasDialog();
            } else {
                Logger.d(TAG, "onPreferenceTreeClick(), mJoynUserAlias is null");
            }
        }        	
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    @Override
    public void onResume() {
        Logger.d(TAG, "onResume()");
        new GetBlockNum().execute();
        super.onResume();
    }

    private class GetCompressStatusTask extends AsyncTask<Void, Void, Boolean> {
        @Override
        protected Boolean doInBackground(Void... params) {
            registerObserver(RcsSettingsData.FIRST_USER_ACCOUNT_CONTENT_URI);
            registerObserver(RcsSettingsData.SECOND_USER_ACCOUNT_CONTENT_URI);
            registerObserver(RcsSettingsData.THIRD_USER_ACCOUNT_CONTENT_URI);
            boolean satus = RcsSettings.getInstance().isEnabledCompressingImageFromDB();
            return satus;
        }

        @Override
        protected void onPostExecute(Boolean result) {
            boolean isChecked = mCompressImageCheckBox.isChecked();
            Logger.d(TAG, "RCS-e isEnabledCompressingImage: " + result + ",isChecked: " + isChecked);
            if (isChecked != result) {
                mCompressImageCheckBox.setChecked(result);
            }
        }
        
        private void registerObserver(final Uri uri) {
            Logger.d(TAG, "registerObserver(): " + uri);
            Cursor cursor = null;
            Activity activity = getActivity();
            Logger.e(TAG, "activity = " + activity);
            if (activity == null) {
                Logger.e(TAG, "return since activity is null");
                return;
            }
            ContentResolver resolver = activity.getContentResolver();
            Logger.e(TAG, "resolver = " + resolver);
            if (resolver == null) {
                Logger.e(TAG, "return since resolver is null");
                return;
            }
            try {
                cursor = resolver.query(uri, new String[] {
                    RcsSettingsData.KEY_ID
                }, RcsSettingsData.KEY_KEY + "=?", new String[] {
                    RcsSettingsData.RCSE_COMPRESSING_IMAGE
                }, null);
                int id = INVALID_VALUE;
                if (cursor != null && cursor.moveToNext()) {
                    id = cursor.getInt(FIRST_POSITION);
                    Logger.d(TAG, "id = " + id);
                    cursor.close();
                    cursor = null;
                }
                if (id == INVALID_VALUE) {
                    resolver.registerContentObserver(uri, true, mRcseSettingContentObserver);
                } else {
                    resolver.registerContentObserver(ContentUris.withAppendedId(uri, id), false,
                            mRcseSettingContentObserver);
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                    cursor = null;
                }
            }
        }
    }

    private class ChangeCompressStatusTask extends AsyncTask<Void, Void, Boolean> {
        @Override
        protected Boolean doInBackground(Void... params) {
            boolean satus = RcsSettings.getInstance().isEnabledCompressingImageFromDB();
            return satus;
        }

        @Override
        protected void onPostExecute(Boolean result) {
            boolean isChecked = mCompressImageCheckBox.isChecked();
            Logger.d(TAG, "RCS-e isEnabledCompressingImage: " + result + ",isChecked: " + isChecked);
            if (isChecked != result) {
                mCompressImageCheckBox.setChecked(result);
            }
        }
    }

    private class GetBlockNum extends AsyncTask<Void, Void, Integer> {

        @Override
        protected Integer doInBackground(Void... params) {
            int mBlockNum = 0;
            ContactsManager contactsManager = null;
            Activity activity = getActivity();
            if (activity != null) {
                contactsManager = ContactsManager.getInstance();
                if (contactsManager != null) {
                    Logger.d(TAG, "contactsManager != null");
                } else {
                    ContactsManager.createInstance(activity.getApplicationContext());
                    contactsManager = ContactsManager.getInstance();
                    Logger.e(TAG, "contactsManager is null");
                }
                ArrayList<String> mBlocked =
                        (ArrayList<String>) contactsManager.getImBlockedContacts();
                mBlockNum = mBlocked.size();
                Logger.i(TAG, "doInBackground mBlockNum =" + mBlockNum + "mBlocked");
            } else {
                Logger.e(TAG, "GetBlockNum  doInBackground mActivity is null");
            }
            return mBlockNum;
        }

        @Override
        protected void onPostExecute(Integer result) {
            super.onPostExecute(result);
            Activity activity = getActivity();
            if (activity != null) {
                Resources r = activity.getResources();
                String temp = r.getString(R.string.text_block_contact, Integer.toString(result));
                Logger.d(TAG, "onPostExecute =" + result);
                mBlockItem.setSummary(temp);
            }
        }
    }

    public class ConfirmDialogOfClearhistory extends DialogFragment implements
            DialogInterface.OnClickListener {

        private static final String TAG = "ClearhistoryConfirmDialog";

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            return new AlertDialog.Builder(getActivity(), AlertDialog.THEME_HOLO_LIGHT)
                    .setIconAttribute(android.R.attr.alertDialogIcon).setTitle(
                            R.string.rcse_clear_chat_history_confirm_title).setMessage(
                            getString(R.string.rcse_clear_chat_history_confirm_message))
                    .setPositiveButton(getString(R.string.rcs_settings_label_clear), this)
                    .setNegativeButton(getString(R.string.rcs_dialog_negative_button), this)
                    .create();
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                mClearHistoryProgress = new ClearHistoryProgress();
                mClearHistoryProgress.show(getFragmentManager(), TAG);
                mClearHistoryTask = new ClearHistoryTask();
                mClearHistoryTask.execute();
            }
            dismissAllowingStateLoss();
        }
    }

    public void showAliasDialog() {
        final AlertDialog.Builder alert = new AlertDialog.Builder(getActivity());
        final String subject ;
        int maxLength = 12;
        LinearLayout lila1 = new LinearLayout(getActivity());
        lila1.setOrientation(1); // 1 is for vertical orientation
        final TextView titleText = new TextView(getActivity());
        final EditText input = new EditText(getActivity());
        titleText.setText(" Please enter your alias name");            
        InputFilter[] filterArray = new InputFilter[1];
        filterArray[0] = new InputFilter.LengthFilter(maxLength);
        input.setFilters(filterArray);
        lila1.addView(titleText);
        lila1.addView(input);
        alert.setView(lila1);        
        alert.setTitle("Joyn User Alias");
        alert.setCancelable(false);                

        alert.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
            	setAlias_Name(input.getText().toString());            	
            	mJoynUserAlias.setSummary(getAlias_Name());
            }
        });
        alert.setNegativeButton("Cancel",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                    	dialog.cancel();                       
                    }
                });
        alert.create();   
        alert.show();
    }

    @Override
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
        Logger.d(TAG, "onSharedPreferenceChanged() entry, the key is " + key);
        if (RCS_REMIND.equals(key)) {
            boolean isRemind = sharedPreferences.getBoolean(key, false);
            Logger.d(TAG, "onSharedPreferenceChanged isRemind is " + isRemind);
            mRemindCheckBox.setChecked(isRemind);
        }
        if (RCS_DISABLE_JOYN.equals(key)) {
            boolean isRemind = sharedPreferences.getBoolean(key, false);
            Logger.d(TAG, "onSharedPreferenceChanged isRemind is " + isRemind);
            mDisableJoyn.setChecked(isRemind);
        }       
        if (RCS_NOTIFICATION.equals(key)) {
            if (mNotificationCheckBox != null) {
                if (!mNotificationCheckBox.isChecked()) {
                    IS_NOTIFICATION_CHECKED.set(false);
                } else {
                    IS_NOTIFICATION_CHECKED.set(true);
                }
                Logger.d(TAG,
                        "onSharedPreferenceChanged(), the value of IS_NOTIFICATION_CHECKED is "
                                + IS_NOTIFICATION_CHECKED);
            } else {
                Logger.d(TAG, "onSharedPreferenceChanged(), mNotificationCheckBox is null");
            }
        }
        if (RCS_VIBRATE.equals(key)) {
            if (mVibrateCheckBox != null) {
                boolean isChecked = mVibrateCheckBox.isChecked();
                Logger.d(TAG, "onSharedPreferenceChanged(), the status of mVibrateCheckBox is "
                        + isChecked);
                RcsSettings.getInstance().setPhoneVibrateForChatInvitation(isChecked);
            } else {
                Logger.d(TAG, "onSharedPreferenceChanged(),mVibrateCheckBox is null");
            }
        }
        Logger.d(TAG, "onSharedPreferenceChanged() exit");
    }

    /**
     * Class defined to display the progress dialog while clear the history
     */
    public class ClearHistoryProgress extends DialogFragment {

        private static final String TAG = "ClearHistoryProgress";

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            Activity activity = getActivity();
            if (activity == null) {
                Logger.d(TAG, "onCreateDialog() The activity is null");
                return null;
            } else {
                ProgressDialog dialog = new ProgressDialog(activity);
                dialog.setTitle(activity.getText(R.string.rcse_clear_history_title));
                dialog.setMessage(activity.getText(R.string.rcse_clear_chat_history_clearing));
                dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
                dialog.setCanceledOnTouchOutside(true);
                dialog.setCancelable(true);
                return (Dialog) dialog;
            }
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
            if (mClearHistoryTask != null) {
                mClearHistoryTask.cancel(true);
            } else {
                Logger.d(TAG, "Can't cancel the clear history operation");
            }
        }
    }

    /**
     * Clear history thread
     */
    private class ClearHistoryTask extends AsyncTask<Void, Void, Void> {
        @Override
        protected Void doInBackground(Void... params) {
            Logger.i(TAG, "ClearHistoryTask(), doInBackground entry");
            ((ModelImpl) ModelImpl.getInstance()).clearAllChatHistory();
            SystemClock.sleep(CLEAR_HISTORY_TIME_OUT);
            mHandler.sendEmptyMessage(CLEAR_HISTORY_MESSAGE_ID);
            Logger.i(TAG, "ClearHistoryTask(), doInBackground exit");
            return null;
        }
    }


    @Override
    public void onSaveInstanceState(Bundle outState) {
        // Override this method to workaround a google issue happen when API level > 11
        outState.putString("work_around_tag", "work_around_content");
        super.onSaveInstanceState(outState);
    }
}
