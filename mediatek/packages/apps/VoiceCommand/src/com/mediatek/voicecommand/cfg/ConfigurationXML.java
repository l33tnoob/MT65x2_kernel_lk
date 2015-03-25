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
package com.mediatek.voicecommand.cfg;

//import java.io.FileInputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import android.content.Context;
import android.content.res.AssetManager;

import android.util.Log;
import android.util.Xml;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.R;

public class ConfigurationXML {

    static final String TAG = "ConfigurationXML";
    private final Context mContext;
    private String mVoiceInfoID = "ID";
    private String mKeyWord = "KeyWord";
    private String mVoiceInfoCommandID = "CommandID";
    private String mVoiceInfoPermissionID = "PermissionID";
    private String mVoiceInfoEnable = "Enable";
    // private String mVoiceLanguage = "Language";
    // private String mVoiceCurrentLanguage = "CurrentLanguage";
    private String mVoiceProcessName = "ProcessName";
    private String mVoiceFeatureName = "FeatureName";
    public static final String sPublicFeatureName = "android.mediatek.feature";

    // private String mVoicePath = "Path";

    public ConfigurationXML(Context context) {
        mContext = context;
    }

    /*
     * read Keyword from res
     *
     * @param voiceKeyWordInfos
     *
     * @param fileName
     */
    public void readKeyWordFromXml(HashMap<String, VoiceKeyWordInfo> voiceKeyWordInfos,
            String fileName) {
        Log.v(TAG, "readKeyWordFromXml from file:" + fileName);
        InputStream in = null;
        AssetManager assetManager = mContext.getAssets();
        try {
            int xmlEventType;
            String keyWord = null;
            String processName = null;
            String path = null;
            in = assetManager.open(fileName);
            // fin = new FileInputStream(in);
            XmlPullParser parser = Xml.newPullParser();
            parser.setInput(in, "UTF-8");
            while ((xmlEventType = parser.next()) != XmlPullParser.END_DOCUMENT) {
                String name = parser.getName();
                if (xmlEventType == XmlPullParser.START_TAG
                        && "KeyWordInfo".equals(name)) {
                    processName = parser.getAttributeValue(null,
                            mVoiceProcessName);
                    keyWord = parser.getAttributeValue(null, mKeyWord);
                    path = parser.getAttributeValue(null, "Path");

                } else if (xmlEventType == XmlPullParser.END_TAG
                        && "KeyWordInfo".equals(name)) {
                    if (processName != null && keyWord != null) {
                        Log.v(TAG, "readEnglishKeyWordFromXml processName   = "
                                + processName + "  KeyWord =" + keyWord);
                        String[] keyWordArray = keyWord.split(",");
                        VoiceKeyWordInfo voiceKeyWordInfo = new VoiceKeyWordInfo(keyWordArray, path);
                        voiceKeyWordInfos.put(processName, voiceKeyWordInfo);
                    } else {
                        Log.v(TAG, "Error processName or keyWord " + keyWord);
                    }
                }
            }
        } catch (XmlPullParserException e) {
            Log.v(TAG, "Got execption parsing permissions.", e);
        } catch (FileNotFoundException e) {
            Log.v(TAG, "Got execption finding file.", e);
        } catch (IOException e) {
            Log.v(TAG, "Got execption parsing permissions.", e);
        } finally {
            try {
                if (in != null) {
                    in.close();
                }
            } catch (IOException e) {
                Log.v(TAG, "Got execption parsing permissions.", e);
            }
        }
    }

    public int readVoiceLanguangeFromXml(
            ArrayList<VoiceLanguageInfo> languageList) {
        int curIndex = -1;
        Log.v(TAG, "readVoiceLanguangeFromXml ");
        if (languageList == null) {
            return curIndex;
        }

        XmlPullParser parser = mContext.getResources().getXml(
                R.xml.voicelanguage);

        if (parser == null) {
            Log.e(TAG, "readVoiceLanguangeFromXml Got execption");
            return curIndex;
        }
        String curlanguage = null;
        try {
            int xmlEventType;
            String languageName = null;
            String filepath = null;
            String id = null;
            String code = null;
            while ((xmlEventType = parser.next()) != XmlPullParser.END_DOCUMENT) {
                String name = parser.getName();
                if (xmlEventType == XmlPullParser.START_TAG
                        && "Language".equals(name)) {

                    languageName = parser.getAttributeValue(null, "TypeName");
                    filepath = parser.getAttributeValue(null, "FileName");
                    id = parser.getAttributeValue(null, "ID");
                    code = parser.getAttributeValue(null, "Code");

                } else if (xmlEventType == XmlPullParser.END_TAG
                        && "Language".equals(name)) {
                    if (languageName != null && filepath != null && id != null) {
                        VoiceLanguageInfo info = new VoiceLanguageInfo(
                        // languageName, mVoiceKeyWordPath + filepath,
                                languageName, filepath, Integer.valueOf(id), code);
                        languageList.add(info);
                    }
                } else if (xmlEventType == XmlPullParser.START_TAG
                        && "DefaultLanguage".equals(name)) {
                    curlanguage = parser.getAttributeValue(null, "ID");
                }
            }

        } catch (XmlPullParserException e) {
            Log.v(TAG, "Got execption parsing languages.", e);
        } catch (FileNotFoundException e) {
            Log.v(TAG, "Got execption finding file.", e);
        } catch (IOException e) {
            Log.v(TAG, "Got execption parsing languages.", e);
        } catch (Exception e) {
            Log.v(TAG, "readVoiceLanguangeFromXml Got execption. ", e);
        }

        Log.v(TAG, "readVoiceLanguangeFromXml curlanguage:" + curlanguage);
        if (curlanguage != null) {
            // String mCurrentLanguage = currentLanguage;
            int curlanguageID = Integer.valueOf(curlanguage).intValue();
            for (int i = 0; i< languageList.size(); i++) {
                if(languageList.get(i).mLanguageID == curlanguageID) {
                    curIndex = i;
                    break;
                }
            }
            if (curIndex < 0) {
                curIndex = 0;
            }
        }
        Log.v(TAG, "readVoiceLanguangeFromXml curIndex:" + curIndex);
        return curIndex;
    }

    public void readCustomizationFromXML(VoiceCustomization voiceCustomization) {
        Log.v(TAG, "readCustomizationFromXML ");
        boolean isSystemLanguage = false;
        String systemLanguage = null;
        String defaultLanguage = null;

        XmlPullParser parser = mContext.getResources().getXml(
                R.xml.voicecustomization);
        if (parser == null) {
            Log.e(TAG, "readCustomizationFromXML Got execption");
            return;
        }
        try {
            int xmlEventType;
            while ((xmlEventType = parser.next()) != XmlPullParser.END_DOCUMENT) {
                String name = parser.getName();
                if (xmlEventType == XmlPullParser.START_TAG
                        && "VoiceCustomization".equals(name)) {
                    systemLanguage = parser.getAttributeValue(null, "SystemLanguage");
                    defaultLanguage = parser.getAttributeValue(null, "DefaultLanguage");
                }
            }
            Log.d(TAG, "systemLanguage:" + systemLanguage + "defaultLanguage" + defaultLanguage);
        } catch (XmlPullParserException e) {
            Log.v(TAG, "Got execption parsing customization.", e);
        } catch (FileNotFoundException e) {
            Log.v(TAG, "Got execption finding file.", e);
        } catch (IOException e) {
            Log.v(TAG, "Got execption parsing languages.", e);
        } catch (Exception e) {
            Log.v(TAG, "readCustomizationFromXML Got execption. ", e);
        }

        if("TRUE".equals(systemLanguage)) {
            isSystemLanguage = true;
        }
        voiceCustomization.mIsSystemLanguage = isSystemLanguage;
        voiceCustomization.mDefaultLanguage = defaultLanguage;
    }

    public void readVoiceFilePathFromXml(HashMap<String, String> pathMap) {

        XmlPullParser parser = mContext.getResources().getXml(
                R.xml.voicecommandpath);
        if (parser == null) {
            Log.e(TAG, "the package has no voice command path ");
            return;
        }

        try {
            int xmlEventType;
            String processName = null;
            String path = null;
            while ((xmlEventType = parser.next()) != XmlPullParser.END_DOCUMENT) {
                String name = parser.getName();
                if (xmlEventType == XmlPullParser.START_TAG
                        && "Path".equals(name)) {
                    processName = parser.getAttributeValue(null, "Name");
                    path = parser.getAttributeValue(null, "Path");
                } else if (xmlEventType == XmlPullParser.END_TAG
                        && "Path".equals(name)) {
                    if (processName != null & path != null) {
                        pathMap.put(processName, path);
                    } else {
                        Log.v(TAG, "the package has no voice command path ");
                    }
                }
            }
        } catch (XmlPullParserException e) {
            Log.v(TAG, "Got execption parsing paths.", e);
        } catch (IOException e) {
            Log.v(TAG, "Got execption parsing paths.", e);
        } catch (Exception e) {
            Log.v(TAG, "readVoiceFilePathFromXml Got execption. ", e);
        }
    }

    public void readVoiceProcessInfoFromXml(
            HashMap<String, VoiceProcessInfo> voiceProcessInfos,
            ArrayList<String> voiceUiList) {
        XmlPullParser parser = mContext.getResources().getXml(R.xml.voiceprocessinfo);
        if (parser == null) {
            Log.v(TAG, "the package has no voice command info");
            return;
        }
        try {
            int xmlEventType;
            String featureName = null;
            VoiceProcessInfo voiceProcessInfo = null;
            while ((xmlEventType = parser.next()) != XmlPullParser.END_DOCUMENT) {

                String name = parser.getName();
                if (xmlEventType == XmlPullParser.START_TAG
                        && "VoiceProcessInfo".equals(name)) {

                    featureName = parser.getAttributeValue(null,
                            mVoiceFeatureName);
                    if (featureName != null) {

                        voiceProcessInfo = new VoiceProcessInfo(featureName);

                        String processName = parser.getAttributeValue(null,
                                mVoiceProcessName);
                        if (processName == null) {
                            Log.v(TAG, " voiceInfo XML processName = NULL");
                            continue;
                        }

                        voiceProcessInfo.mRelationProcessName = parser
                                .getAttributeValue(null, "RelationProcess");

                        String idStr = parser.getAttributeValue(null,
                                mVoiceInfoID);
                        if (idStr == null) {
                            Log.v(TAG, " voiceInfo XML ID = NULL");
                            continue;
                        }
                        voiceProcessInfo.mID = Integer.parseInt(idStr);
                        String commandID = parser.getAttributeValue(null,
                                mVoiceInfoCommandID);
                        if (commandID == null) {
                            Log.v(TAG, " voiceInfo XML commandID = NULL");
                            continue;
                        }

                        String permissionID = parser.getAttributeValue(null,
                                mVoiceInfoPermissionID);
                        if (permissionID == null) {
                            Log.v(TAG, " voiceInfo XML PermissionID = NULL");
                            continue;
                        }

                        String voiceEnable = parser.getAttributeValue(null,
                                mVoiceInfoEnable);
                        if (voiceEnable == null) {
                            Log.v(TAG, " voiceInfo XML voiceEnable1 = NULL");
                            continue;
                        }
                        voiceProcessInfo.isVoiceEnable = voiceEnable
                                .equals("TRUE") ? true : false;

                        Log.v(TAG, "readVoiceInfoFromXml featureName = "
                                + featureName + "processName = " + processName
                                + "  commandID =" + commandID
                                + " permissionID = " + permissionID
                                + " voiceEnable=" + voiceEnable);

                        if (commandID != null) {
                            String[] commandIDTemp = commandID.split(",");
                            for (int i = 0; i < commandIDTemp.length; i++) {
                                // Log.v(TAG,"commandIDTemp[i] = "+Integer.valueOf(commandIDTemp[i]));
                                voiceProcessInfo.mCommandIDList.add(Integer
                                        .valueOf(commandIDTemp[i]));
                            }
                        }

                        if (processName != null) {
                            String[] processNameTemp = processName.split(",");
                            for (int i = 0; i < processNameTemp.length; i++) {
                                Log.v(TAG, "processNameTemp[i] = "
                                        + processNameTemp[i]);
                                voiceProcessInfo.mProcessNameList
                                        .add(processNameTemp[i]);
                            }
                        }

                        if (permissionID != null) {
                            String[] permissionIDTemp = permissionID.split(",");
                            for (int i = 0; i < permissionIDTemp.length; i++) {
                                // Log.v(TAG,"permissionIDTemp[i] = "+Integer.valueOf(permissionIDTemp[i]));
                                int permissionid = Integer
                                        .valueOf(permissionIDTemp[i]);
                                if (permissionid == VoiceCommandListener.ACTION_MAIN_VOICE_UI
                                        && (voiceProcessInfo.mRelationProcessName == null || !voiceProcessInfo.mRelationProcessName
                                                .endsWith(sPublicFeatureName))) {
                                    voiceUiList
                                            .add(voiceProcessInfo.mFeatureName);
                                }
                                voiceProcessInfo.mPermissionIDList.add(Integer
                                        .valueOf(permissionIDTemp[i]));
                            }
                        }

                    } else {
                        Log.v(TAG, "the package has no voice command info ");
                    }
                } else if (xmlEventType == XmlPullParser.END_TAG
                        && "VoiceProcessInfo".equals(name)) {
                    if (featureName != null && voiceProcessInfo != null) {
                        voiceProcessInfos.put(featureName, voiceProcessInfo);
                    }
                }
            }
        } catch (XmlPullParserException e) {
            Log.v(TAG, "Got execption parsing permissions.", e);
        } catch (IOException e) {
            Log.v(TAG, "Got execption parsing permissions.", e);
        } catch (Exception e) {
            Log.v(TAG, "readVoiceProcessInfoFromXml Got execption. ", e);
        }
    }

}
