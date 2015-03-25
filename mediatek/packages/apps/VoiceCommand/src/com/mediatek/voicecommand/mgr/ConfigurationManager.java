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

package com.mediatek.voicecommand.mgr;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Locale;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import android.os.FileUtils;
import android.util.Log;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.cfg.ConfigurationXML;
import com.mediatek.voicecommand.cfg.VoiceCustomization;
import com.mediatek.voicecommand.cfg.VoiceKeyWordInfo;
import com.mediatek.voicecommand.cfg.VoiceLanguageInfo;
import com.mediatek.voicecommand.cfg.VoiceProcessInfo;
import com.mediatek.voicecommand.service.VoiceCommandManagerStub;

public class ConfigurationManager {

    // static final String TAG = "VoiceConfigurationManager";
    private static ConfigurationManager sCfgMgr;
    private static byte[] sInstanceLock = new byte[0];
    private final Context mContext;
    private int mCurrentLanguageIndex = -1;
    //private boolean mVoiceControlKeyEnabled = false;
    private final HashMap<String, String> mPaths = new HashMap<String, String>();
    private final HashMap<String, VoiceProcessInfo> mVoiceProcessInfos = new HashMap<String, VoiceProcessInfo>();
    private final HashMap<String, VoiceKeyWordInfo> mVoiceKeyWordInfos = new HashMap<String, VoiceKeyWordInfo>();
    private ArrayList<VoiceLanguageInfo> mLanguageList = new ArrayList<VoiceLanguageInfo>();
    private ArrayList<String> mVoiceUiFeatureNames = new ArrayList<String>();
    private ConfigurationXML mConfigurationXML;

    private String mRecogPatternPath = "RecogPattern";
    private String mUIPatternPath = "UIPattern";
    private String mPswdFilePath = "PswdFile";
    private String mContactsdbFilePath = "ContactsdbFile";
    private String mFeatureFilePath = "FeatureFile";
    private String mUBMFilePath = "UBMFile";
    private String mModeFilePath = "ModeFile";
    private String mContactsModeFilePath = "ContactsModeFile";
    private final String mRecogPath = "/training/recogpattern/";
    private final String mfeaturePath = "/training/featurefile/";
    private final String mpasswordPath = "/training/passwordfile/";
    private final String mContactsdbPath = "/contacts/";
    private boolean isCfgPrepared = true;

    private String mVoiceUiCacheFile = "com.mediatek.voicecommand_preferences";
    //private String mVoiceControlCacheFile = "Voice_Control_Enabled";
    private String mVoiceLanguageCacheFile = "Voice_Language";
    private String mCurLanguageKey = "CurLanguageIndex";
    private String mIsFirstBoot = "IsFirstBoot";
    private String mCurSystemLanguageIndex = "CurSystemLanguageIndex";
    //private String mVoiceControlKey = "VoiceControlEnabled";
    private boolean isSystemLanguage = false;
    private VoiceCustomization mVoiceCustomization = new VoiceCustomization();
    /*
     * ConfigurationManager for control configuration
     *
     * @param context
     */
    private ConfigurationManager(Context context) {
        mContext = context.getApplicationContext();
        mConfigurationXML = new ConfigurationXML(context);
        mConfigurationXML.readVoiceProcessInfoFromXml(mVoiceProcessInfos,
                mVoiceUiFeatureNames);
        mConfigurationXML.readVoiceFilePathFromXml(mPaths);
        mCurrentLanguageIndex = mConfigurationXML
                .readVoiceLanguangeFromXml(mLanguageList);
        String dataDir = context.getApplicationInfo().dataDir;
        mPaths.put(mRecogPatternPath, dataDir + mRecogPath);
        mPaths.put(mFeatureFilePath, dataDir + mfeaturePath);
        mPaths.put(mPswdFilePath, dataDir + mpasswordPath);
        mPaths.put(mContactsdbFilePath, dataDir + mContactsdbPath);

        mConfigurationXML.readCustomizationFromXML(mVoiceCustomization);
        isSystemLanguage = mVoiceCustomization.mIsSystemLanguage;
        Log.i(VoiceCommandManagerStub.TAG, "ConfigurationManager isSystemLanguage = " + isSystemLanguage);
        if (isSystemLanguage) {
            // Register Local change receiver.
            IntentFilter filter = new IntentFilter();
            filter.addAction(Intent.ACTION_LOCALE_CHANGED);
            context.registerReceiver(mSystemLanguageReceiver, filter);
        }

        if (!makeDirForPath(mPaths.get(mRecogPatternPath))
                || !makeDirForPath(mPaths.get(mFeatureFilePath))
                || !makeDirForPath(mPaths.get(mPswdFilePath))
                || !makeDirForPath(mPaths.get(mContactsdbFilePath))) {
            isCfgPrepared = false;
        }
        if (isCfgPrepared) {
            checkVoiceCachePref();
            if (mCurrentLanguageIndex >= 0) {
                mConfigurationXML.readKeyWordFromXml(mVoiceKeyWordInfos,
                        mLanguageList.get(mCurrentLanguageIndex).mFilePath);
            }
        }
    }

    public static ConfigurationManager getInstance(Context context){
        if(sCfgMgr == null){
            synchronized(sInstanceLock){
                if(sCfgMgr == null){
                    sCfgMgr = new ConfigurationManager(context);
                }
            }
        }
        return sCfgMgr;
    }

    /*
     * read Pref in mVoiceProcessInfos and mCurrentLanguageIndex
     */
    private void checkVoiceCachePref() {

        SharedPreferences processPref = mContext.getSharedPreferences(
                mVoiceUiCacheFile, Context.MODE_PRIVATE);
        Map<String, Boolean> enableMap;
        try {
            enableMap = (Map<String, Boolean>) processPref.getAll();
        } catch (NullPointerException ex) {
            enableMap = null;
        }

        if (enableMap != null) {
            Iterator iter = enableMap.entrySet().iterator();
            while (iter.hasNext()) {
                Entry entry = (Entry) iter.next();
                String name = (String) entry.getKey();
                VoiceProcessInfo info = mVoiceProcessInfos.get(name);
                if (info != null) {
                    info.isVoiceEnable = (Boolean) entry.getValue();
                    if (info.mRelationProcessName != null) {
                        VoiceProcessInfo relationInfo = mVoiceProcessInfos.get(info.mRelationProcessName);
                        if (relationInfo != null) {
                            relationInfo.isVoiceEnable = info.isVoiceEnable;
                        }
                    }
                }
            }
        }

        if (isSystemLanguage) {
            useSystemLanguage();
        } else {
            processPref = mContext.getSharedPreferences(mVoiceLanguageCacheFile,
                    Context.MODE_PRIVATE);
            mCurrentLanguageIndex = processPref.getInt(mCurLanguageKey, mCurrentLanguageIndex);
        }
        Log.i(VoiceCommandManagerStub.TAG, "mCurrentLanguageIndex = " + mCurrentLanguageIndex);

        /*processPref = mContext.getSharedPreferences(mVoiceControlCacheFile,
                Context.MODE_PRIVATE);
        mVoiceControlKeyEnabled = processPref.getBoolean(mVoiceControlKey, mVoiceControlKeyEnabled);
        Log.i(VoiceCommandManagerStub.TAG, "mVoiceControlKeyEnabled = "
                + mVoiceControlKeyEnabled);*/
    }

    /*
     * handle the case of using the system language
     *
     */
    public void useSystemLanguage() {
        mCurrentLanguageIndex = -1;
        String systemLanguage = Locale.getDefault().getLanguage() + "-"
                + Locale.getDefault().getCountry();
        Log.i(VoiceCommandManagerStub.TAG, "system language = " + systemLanguage);

        SharedPreferences languagePref = mContext.getSharedPreferences(mVoiceLanguageCacheFile,
                Context.MODE_PRIVATE);
        boolean isFirstBoot = languagePref.getBoolean(mIsFirstBoot, true);
        Log.i(VoiceCommandManagerStub.TAG, "isFirstBoot = " + isFirstBoot);
        updateCurLanguageIndex(systemLanguage);
        if (mCurrentLanguageIndex < 0) {
            if (isFirstBoot) {
                updateCurLanguageIndex(mVoiceCustomization.mDefaultLanguage);
            } else {
                mCurrentLanguageIndex = languagePref.getInt(mCurSystemLanguageIndex,
                        mCurrentLanguageIndex);
            }
        }
        if (isFirstBoot) {
            languagePref.edit().putBoolean(mIsFirstBoot, false).apply();
        }
    }
    /*
     * update mCurrentLanguageIndex
     *
     * @param language
     * @return
     */
    public void updateCurLanguageIndex(String language) {
        if (language == null) {
            return;
        }
        for (int i = 0; i < mLanguageList.size(); i++) {
            if (language.equals(mLanguageList.get(i).mLanguageCode)) {
                mCurrentLanguageIndex = i;
                SharedPreferences languagePref = mContext.getSharedPreferences(
                        mVoiceLanguageCacheFile, Context.MODE_PRIVATE);
                languagePref.edit().putInt(mCurSystemLanguageIndex, mCurrentLanguageIndex).apply();
                break;
            }
        }
    }
    /*
     * check whether configuration file is prepared
     */
    public boolean isCfgPrepared() {
        Log.i(VoiceCommandManagerStub.TAG, "Voice Command Service CFG OK?"
                + isCfgPrepared);
        return isCfgPrepared;
    }

    public boolean getIsSystemLanguage() {
        return isSystemLanguage;
    }
    /*
     * create dir and file
     *
     * @param path
     * @return
     */
    public static boolean makeDirForPath(String path) {
        if (path == null) {
            return false;
        }
        try {
            File dir = new File(path);
            if (!dir.exists()) {
                dir.mkdirs();
                FileUtils.setPermissions(dir.getPath(), 0775, -1, -1); // dwxrwxr-x
            }
        } catch (NullPointerException ex) {
            return false;
        }
        return true;
    }

    public static boolean makeDirForFile(String file) {
        if (file == null) {
            return false;
        }
        try {
            File f = new File(file);
            File dir = f.getParentFile();
            if (!dir.exists()) {
                dir.mkdirs();
                FileUtils.setPermissions(dir.getPath(), 0775, -1, -1); // dwxrwxr-x
            }
            FileUtils.setPermissions(f.getPath(), 0666, -1, -1); // -rw-rw-rw-
        } catch (NullPointerException ex) {
            return false;
        }

        return true;
    }

    /*
     * check whether process is Allowed to register or not
     *
     * @param processname
     * @return
     */
    public int isAllowProcessRegister(String processname) {
        return mVoiceProcessInfos.containsKey(processname) ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGALPROCESS;
    }

    /*
     * get the numbers of UiFeature
     * @return
     */
    public int getUiFeatureNumber() {
        return mVoiceUiFeatureNames.size();
    }

    /*
     * check whether process has permission to do action or not
     *
     * @param processname
     * @param mainaction
     * @return
     */
    public boolean hasOperationPermission(String featurename, int mainaction) {
        VoiceProcessInfo info = mVoiceProcessInfos.get(featurename);
        if (info == null) {
            return false;
        }
        return info.mPermissionIDList.contains(mainaction);
    }

    /*
     * check whether process has permission to do actions in array or not
     *
     * @param processname
     * @param mainaction
     * @return
     */
    public boolean containOperationPermission(String processname,
            int[] mainaction) {

        if (mainaction == null || mainaction.length == 0) {
            return false;
        }
        VoiceProcessInfo info = mVoiceProcessInfos.get(processname);
        if (info == null) {
            return false;
        }
        for (int i = 0; i < mainaction.length; i++) {
            if (info.mPermissionIDList.contains(mainaction[i])) {
                return true;
            }
        }
        return false;
    }

    /*
     * Get the feature name list and the process enable list
     * 
     * @return
     */
    public String[] getFeatureNameList() {
        int size = mVoiceUiFeatureNames.size();
        if (size == 0) {
            return null;
        }
        return mVoiceUiFeatureNames.toArray(new String[size]);
    }

    /*
     * Get the feature enable list
     * 
     * @return
     */
    public int[] getFeatureEnableArray() {

        int size = mVoiceUiFeatureNames.size();
        if (size == 0) {
            return null;
        }
        int[] isEnableArray = new int[size];

        for (int i = 0; i < size; i++) {
            isEnableArray[i] = mVoiceProcessInfos.get(mVoiceUiFeatureNames
                    .get(i)).isVoiceEnable ? 1 : 0;
        }

        return isEnableArray;
    }

    /*
     * judge if all feature is off
     *
     * @return status if it's on or off
     */
    /*private boolean isAllOff() {

        int size = mVoiceUiFeatureNames.size();

        for (int i = 0; i < size; i++) {
            boolean isOn = mVoiceProcessInfos.get(mVoiceUiFeatureNames
                    .get(i)).isVoiceEnable;
            if (isOn) {
                return false;
            }
        }

        return true;
    }*/

    /*
     * Get the voice control enabled
     *
     * @return
     */
/*    public boolean getVoiceControlEnable() {
        return mVoiceControlKeyEnabled;
    }
*/
    /*
     * update Feature enable in mVoiceProcessInfos
     *
     * @param featurename
     * @param enable
     * @return
     */
    public boolean updateFeatureEnable(String featurename, boolean enable) {

        VoiceProcessInfo info = mVoiceProcessInfos.get(featurename);
        if (info == null) {
            return false;
        }

        info.isVoiceEnable = enable;
        if (info.mRelationProcessName != null
                && !info.mRelationProcessName
                        .equals(ConfigurationXML.sPublicFeatureName)) {
            updateFeatureEnable(info.mRelationProcessName, enable);
        }
        updateFeatureListEnableToPref();

        /*if ((enable == true) && (mVoiceControlKeyEnabled == false)) {
            mVoiceControlKeyEnabled = true;
            //updateVoiceControlEnableToPref();
        } else if ((enable == false) && (mVoiceControlKeyEnabled == true)
                && isAllOff()) {
            mVoiceControlKeyEnabled = false;
            //updateVoiceControlEnableToPref();
        }*/
        return true;
    }

    /*
     * save Feature enable to Shared Preferences
     */
    private void updateFeatureListEnableToPref() {
        synchronized (mVoiceProcessInfos) {
            SharedPreferences processPref = mContext.getSharedPreferences(
                    mVoiceUiCacheFile, Context.MODE_PRIVATE);
            for (String featurename : mVoiceUiFeatureNames) {
                Log.i(VoiceCommandManagerStub.TAG,
                        "updateFeatureListEnableToPref featurename"
                                + featurename);
                processPref.edit().putBoolean(featurename,
                        mVoiceProcessInfos.get(featurename).isVoiceEnable)
                        .apply();
            }
            // commit all
            // processPref.edit().apply();
            // mConfigurationXML.changeVoiceProcessInfoInXml(mVoiceProcessInfos);
        }
    }

    /*
     * get language list that this feature support
     *
     * @return
     */
    public String[] getLanguageList() {
        int size = mLanguageList.size();
        if (size == 0) {
            return null;
        }
        String[] language = new String[size];
        // Log.i(VoiceCommandManagerService.TAG, "mLanguageList = "+ size);
        for (int i = 0; i < size; i++) {
            language[i] = mLanguageList.get(i).mName;
        }

        return language;
    }

    /*
     * save Current Language to Shared Preferences
     */
    private void updateCurLanguageToPref() {
        synchronized (mLanguageList) {
            SharedPreferences processPref = mContext.getSharedPreferences(
                    mVoiceLanguageCacheFile, Context.MODE_PRIVATE);
            processPref.edit().putInt(mCurLanguageKey, mCurrentLanguageIndex)
                    .apply();
        }
        updateCurLanguageKeyword();
    }

    /*
     * save voice control enable to Shared Preferences
     */
    /*private void updateVoiceControlEnableToPref() {
        synchronized (mLanguageList) {
            SharedPreferences processPref = mContext.getSharedPreferences(
                    mVoiceControlCacheFile, Context.MODE_PRIVATE);
            processPref.edit().putBoolean(mVoiceControlKey, mVoiceControlKeyEnabled)
                    .apply();
        }
    }*/

    /*
     * get Current Language
     *
     * @return
     */
    public int getCurrentLanguage() {
        return mCurrentLanguageIndex;
    }

    /*
     * get Current Language ID defined in voicelanguage.xml
     *
     * @return
     */
    public int getCurrentLanguageID() {
        Log.i(VoiceCommandManagerStub.TAG, "getCurrentLanguageID() mCurrentLanguageIndex : "
                + mCurrentLanguageIndex);
        return mCurrentLanguageIndex < 0 ? mCurrentLanguageIndex
                : mLanguageList.get(mCurrentLanguageIndex).mLanguageID;
    }

    public void updateCurLanguageKeyword(){
        synchronized (mVoiceKeyWordInfos) {
            mVoiceKeyWordInfos.clear();
            if (mCurrentLanguageIndex >= 0) {
                mConfigurationXML.readKeyWordFromXml(mVoiceKeyWordInfos,
                        mLanguageList.get(mCurrentLanguageIndex).mFilePath);
            }
        }
    }
    /*
     * set Current Language
     *
     * @param languageIndex
     */
    public void setCurrentLanguage(int languageIndex) {
        mCurrentLanguageIndex = languageIndex;
        updateCurLanguageToPref();
    }

    /*
     * get process ID defined in voiceprocessinfo.xml
     *
     * @param languageIndex
     */
    public int getProcessID(String featurename) {
        VoiceProcessInfo processinfo = mVoiceProcessInfos.get(featurename);
        if (processinfo == null) {
            return -1;
        }

        return processinfo.mID;
    }

    /*
     * set Current Language
     *
     * @param languageIndex
     */
    /*public void setVoiceControlEnable(boolean isEnabled) {
        mVoiceControlKeyEnabled = isEnabled;
        //updateVoiceControlEnableToPref();
    }*/

    /*
     * check process is enable or not
     *
     * @param processname
     * @return
     */
    public boolean isProcessEnable(String processname) {
//        if (!mVoiceControlKeyEnabled) {
//            return false;
//        }

        VoiceProcessInfo processinfo = mVoiceProcessInfos.get(processname);
        if (processinfo == null) {
            return false;
        }
        return processinfo.isVoiceEnable;
    }

    /*
     * get VoiceProcessInfo from mVoiceProcessInfos
     *
     * @param processname
     * @return
     */
    public VoiceProcessInfo getProcessInfo(String processname) {
        return mVoiceProcessInfos.get(processname);
    }

    /*
     * get Voice Recognition Pattern File Path from voicecommandpath.xml
     *
     * @return
     */
    public String getVoiceRecognitionPatternFilePath() {
        return mPaths.get(mRecogPatternPath);
    }

    /*
     * get VoiceUI Pattern Path from voicecommandpath.xml
     */
    public String getVoiceUIPatternPath() {
        return mPaths.get(mUIPatternPath);
    }

    /*
     * get Password File Path from voicecommandpath.xml
     *
     * @return
     */
    public String getPasswordFilePath() {
        return mPaths.get(mPswdFilePath);
    }

    /*
     * get Contacts db File Path from voicecommandpath.xml
     *
     * @return
     */
    public String getContactsdbFilePath() {
        return mPaths.get(mContactsdbFilePath);
    }
    /*
     * get Feature File Path from voicecommandpath.xml
     *
     * @return
     */
    public String getFeatureFilePath() {
        return mPaths.get(mFeatureFilePath);
    }

    /*
     * get UBM File Path from voicecommandpath.xml
     *
     * @return
     */
    public String getUBMFilePath() {
        return mPaths.get(mUBMFilePath);
    }

    /*
     * get Model File Path from voicecommandpath.xml
     *
     * @return
     */
    public String getModelFile() {
        return mPaths.get(mModeFilePath);
    }

    /*
     * get Model File Path from voicecommandpath.xml
     *
     * @return
     */
    public String getContactsModelFile() {
        return mPaths.get(mContactsModeFilePath);
    }

    /*
     * get KeyWord refer to processName defined in keyword.xml
     *
     * @param processName
     * @return
     */
    public String[] getKeyWord(String processName) {
        synchronized (mVoiceKeyWordInfos) {
//            return mVoiceKeyWordInfos.get(processName).mKeyWordArray;
            VoiceKeyWordInfo info = mVoiceKeyWordInfos.get(processName);
            if (info == null ) {
                return null;
            }
            return info.mKeyWordArray;
        }
    }

    /*
     * get command path refer to processName and id defined in keyword.xml
     *
     * @param processName
     *
     * @return
     */
    public String getCommandPath(String processName) {
        synchronized (mVoiceKeyWordInfos) {
//            return mVoiceKeyWordInfos.get(processName).mKeyWordPath;
            VoiceKeyWordInfo info = mVoiceKeyWordInfos.get(processName);
            if (info == null ) {
                return null;
            }
            return info.mKeyWordPath;
        }
    }

    /*
     * get KeyWord refer to processName defined in keyword.xml
     *
     * @param processName
     * @return
     */
    public String[] getKeyWordForSettings(String processName) {
        synchronized (mVoiceKeyWordInfos) {
            VoiceProcessInfo info = mVoiceProcessInfos.get(processName);
            if (info != null && info.mRelationProcessName != null) {
                ArrayList<String> keywordList =  new ArrayList<String>();
//                String[] keyword = mVoiceKeyWordInfos.get(processName).mKeyWordArray;
                VoiceKeyWordInfo keyWordInfo = mVoiceKeyWordInfos.get(processName);
                if (keyWordInfo == null) {
                    return null;
                }
                String[] keyword = keyWordInfo.mKeyWordArray;
                if (keyword != null) {
                    for (int i = 0; i < keyword.length; i++) {
                        keywordList.add(keyword[i]);
                    }
                }
                String[] keywordRelation = mVoiceKeyWordInfos.get(info.mRelationProcessName).mKeyWordArray;
                if (keywordRelation != null) {
                    for (int i = 0; i < keywordRelation.length; i++) {
                        if (!keywordList.contains(keywordRelation[i])) {
                            keywordList.add(keywordRelation[i]);
                        }
                    }
                }

                return keywordList.toArray(new String[keywordList.size()]);
            } else {
//                return mVoiceKeyWordInfos.get(processName).mKeyWordArray;
                VoiceKeyWordInfo keyWordInfo = mVoiceKeyWordInfos.get(processName);
                if (keyWordInfo == null) {
                    return null;
                }
                return keyWordInfo.mKeyWordArray;
            }
        }
    }

    /*
     * get Process Name according to featureName
     *
     * @param featureName
     * @return
     */
    public ArrayList<String> getProcessName(String featureName) {
        VoiceProcessInfo processinfo = mVoiceProcessInfos.get(featureName);
        if (processinfo == null) {
            return null;
        }
        return processinfo.mProcessNameList;
    }

    /*
     * Receive Local change broadcast.
     */
    private BroadcastReceiver mSystemLanguageReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            // TODO Auto-generated method stub
            if ((Intent.ACTION_LOCALE_CHANGED).equals(intent.getAction())) {
                String systemLanguage = Locale.getDefault().getLanguage() + "-"
                        + Locale.getDefault().getCountry();
                Log.i(VoiceCommandManagerStub.TAG, "BroadcastReceiver systemLanguage : "
                        + systemLanguage);
                updateCurLanguageIndex(systemLanguage);
                updateCurLanguageKeyword();
            }
        }
    };

    /*
     * ConfigurationManager release when VoiceCommandManagerService destroy.
     */
    public void release() {
        if (isSystemLanguage) {
            mContext.unregisterReceiver(mSystemLanguageReceiver);
            sCfgMgr = null;
        }
    }
}
