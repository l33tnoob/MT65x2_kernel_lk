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

package com.mediatek.datatransfer.modules;

import android.content.Context;
import android.provider.ContactsContract.Contacts;
import com.android.vcard.VCardComposer;
import com.android.vcard.VCardConfig;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.Constants.ContactType;
import com.mediatek.datatransfer.utils.Constants.ModulePath;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class ContactBackupComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/ContactBackupComposer";
    private int mIndex;
    private VCardComposer mVCardComposer;
    private int mCount;
    FileOutputStream mOutStream;

    public ContactBackupComposer(Context context) {
        super(context);
    }

    public int getModuleType() {
        return ModuleType.TYPE_CONTACT;
    }
    public int getCount() {
        MyLogger.logD(CLASS_TAG, "getCount():" + mCount);
        return mCount;
    }

    public boolean init() {
        boolean result = false;
        mCount = 0;
        mVCardComposer = new VCardComposer(mContext, VCardConfig.VCARD_TYPE_V21_GENERIC, true);
        String condition = getCondition();
        if (mVCardComposer.init(condition, null)) {
            result = true;
            mCount = mVCardComposer.getCount();
        } else {
            mVCardComposer = null;
        }

        MyLogger.logD(CLASS_TAG, "init():" + result + ",count:" + mCount);

        return result;
    }

    public boolean isAfterLast() {
        boolean result = true;
        if (mVCardComposer != null) {
            result = mVCardComposer.isAfterLast();
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }

    protected boolean implementComposeOneEntity() {
        boolean result = false;
        if (mVCardComposer != null && !mVCardComposer.isAfterLast()) {
            String tmpVcard = mVCardComposer.createOneEntry();
            if (tmpVcard != null && tmpVcard.length() > 0) {
                if(mOutStream != null) {
                    try {
                        byte[] buf = tmpVcard.getBytes();
                        mOutStream.write(buf, 0, buf.length);
                        result = true;
                    } catch(IOException e) {
                        if (super.mReporter != null) {
                            super.mReporter.onErr(e);
                        }
                    } catch(Exception e) {
                    }
                }
            }
        }

        MyLogger.logD(CLASS_TAG, "add result:" + result);
        return result;
    }

    /**
     * Describe <code>onStart</code> method here.
     *
     */
    public final void onStart() {
        super.onStart();
        if(getCount() > 0) {
            String path = mParentFolderPath + File.separator + ModulePath.FOLDER_CONTACT;
            File folder = new File(path);
            if(!folder.exists()) {
                folder.mkdirs();
            }

            String fileName = path + File.separator + ModulePath.NAME_CONTACT;
            try {
                mOutStream = new FileOutputStream(fileName);
            } catch(IOException e) {
                mOutStream = null;
            } catch(Exception e) {
            }

        }

    }

    public void onEnd() {
        super.onEnd();
        if (mVCardComposer != null) {
            mVCardComposer.terminate();
            mVCardComposer = null;
        }

        if(mOutStream != null) {
            try {
                mOutStream.flush();
                mOutStream.close();
            } catch(IOException e) {
            } catch(Exception e) {
            }
        }
    }

     private String getCondition() {
         if(mParams != null) {
             List<String> conditionList = new ArrayList<String>();
//             List<SIMInfo> simInfoList = SIMInfo.getInsertedSIMList(mContext);
             List<SimInfoRecord> simInfoList = SimInfoManager.getInsertedSimInfoList(mContext);
             if(simInfoList != null && simInfoList.size() > 0) {
                 for(SimInfoRecord simInfo : simInfoList){
                     if(mParams.contains(simInfo.mDisplayName)) {
                         conditionList.add(Long.toString(simInfo.mSimInfoId));
                     }
                 }
             }

             if(mParams.contains(ContactType.PHONE)) {
                 conditionList.add("-1");
             }

             int len = conditionList.size();
             if(len > 0) {
                 StringBuilder condition = new StringBuilder();
                 condition.append(Contacts.INDICATE_PHONE_SIM + " =" + conditionList.get(0));
                 for(int i = 1; i < len; ++i) {
                     condition.append(" OR " + Contacts.INDICATE_PHONE_SIM + " =" + conditionList.get(i));
                 }
                 return condition.toString();
             }
         }

         return null;
    }
}
