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
import android.database.Cursor;
import android.net.Uri;
import android.provider.Telephony.Sms;
import android.telephony.SmsMessage.SubmitPdu;
import android.telephony.SmsMessage;
import android.text.format.DateFormat;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.ArrayList;
import android.provider.Telephony;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.datatransfer.utils.Constants;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.Constants.ModulePath;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

public class SmsBackupComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/SmsBackupComposer";
    private static final String TRICKY_TO_GET_DRAFT_SMS_ADDRESS = "canonical_addresses.address from sms,threads,canonical_addresses where sms.thread_id=threads._id and threads.recipient_ids=canonical_addresses._id and sms.thread_id =";

    private static final String COLUMN_NAME_DATE = "date";
    private static final String COLUMN_NAME_READ = "read";
    private static final String COLUMN_NAME_SEEN = "seen";
    private static final String COLUMN_NAME_TYPE = "type";
    private static final String COLUMN_NAME_SIM_ID = "sim_id";
    private static final String COLUMN_NAME_LOCKED = "locked";
    private static final String COLUMN_NAME_THREAD_ID = "thread_id";
    private static final String COLUMN_NAME_ADDRESS = "address";
    private static final String COLUMN_NAME_SC = "service_center";
    private static final String COLUMN_NAME_BODY = "body";

    private static final Uri[] mSmsUriArray = {
        Sms.Inbox.CONTENT_URI,
        Sms.Sent.CONTENT_URI,
        //Sms.Outbox.CONTENT_URI,
        //Sms.Draft.CONTENT_URI
    };
    private Cursor[] mSmsCursorArray = { null, null };

    private static final String mStoragePath = "sms";
    private static final String mStorageName = "sms.vmsg";
    Writer mWriter = null;

    /**
     * Creates a new <code>SmsBackupComposer</code> instance.
     *
     * @param context a <code>Context</code> value
     */
    public SmsBackupComposer(Context context) {
        super(context);
    }

    @Override
    /**
     * Describe <code>getModuleType</code> method here.
     *
     * @return an <code>int</code> value
     */
    public int getModuleType() {
        return ModuleType.TYPE_SMS;
    }

    @Override
    /**
     * Describe <code>getCount</code> method here.
     *
     * @return an <code>int</code> value
     */
    public int getCount() {
        int count = 0;
        for (Cursor cur : mSmsCursorArray) {
            if (cur != null && !cur.isClosed() && cur.getCount() > 0) {
                count += cur.getCount();
            }
        }

        MyLogger.logD(CLASS_TAG, "getCount():" + count);
        return count;
    }

    @Override
    /**
     * Describe <code>init</code> method here.
     *
     * @return a <code>boolean</code> value
     */
    public boolean init() {
        boolean result = false;
        for (int i = 0; i < mSmsUriArray.length; ++i) {
            mSmsCursorArray[i] = mContext.getContentResolver().query(mSmsUriArray[i], null, null,
                    null, "date ASC");
            if (mSmsCursorArray[i] != null) {
                mSmsCursorArray[i].moveToFirst();
                result = true;
            }
        }

        MyLogger.logD(CLASS_TAG, "init():" + result + ",count:" + getCount());
        return result;
    }
    
    @Override
    /**
     * Describe <code>isAfterLast</code> method here.
     *
     * @return a <code>boolean</code> value
     */
    public boolean isAfterLast() {
        boolean result = true;
        for (Cursor cur : mSmsCursorArray) {
            if (cur != null && !cur.isAfterLast()) {
                result = false;
                break;
            }
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }


    /**
     * Describe <code>implementComposeOneEntity</code> method here.
     *
     * @return a <code>boolean</code> value
     */
    public boolean implementComposeOneEntity() {
        boolean result = false;
        
        for (int i = 0; i < mSmsCursorArray.length; ++i) {
            if (mSmsCursorArray[i] != null && !mSmsCursorArray[i].isAfterLast()) {
                Cursor tmpCur = mSmsCursorArray[i];

                long mtime = tmpCur.getLong(tmpCur.getColumnIndex(COLUMN_NAME_DATE));

                String timeStamp = formatTimeStampString(mContext, mtime);

                int read = tmpCur.getInt(tmpCur.getColumnIndex(COLUMN_NAME_READ));
                String readByte = (read == 0 ? "UNREAD" : "READ");

                String seen = tmpCur.getString(tmpCur.getColumnIndex(COLUMN_NAME_SEEN));


                int box  = tmpCur.getInt(tmpCur.getColumnIndex(COLUMN_NAME_TYPE));
                String boxType = null;
                switch(box) {
                case 1:
                    boxType = "INBOX";
                    break;

                case 2:
                    boxType = "SENDBOX";
                    break;

                default:
                    boxType = "INBOX";
                    break;
                }


                long simid = tmpCur.getLong(tmpCur.getColumnIndex(COLUMN_NAME_SIM_ID));
                String mSlotid = "0";
                if (FeatureOption.MTK_GEMINI_SUPPORT == true && simid >= 0) {
                     /*
                     * M: remove getSlotById method.
                     */
//                    int slot = SimInfoManager.getSlotById(mContext, simid);
                    SimInfoRecord simInfo = SimInfoManager.getSimInfoById(mContext, simid);
                    int slot = -1;
                    if (simInfo != null) {
                        slot = simInfo.mSimSlotId;
                    }
                    mSlotid = String.valueOf(slot + 1);
                }

                int lock = tmpCur.getInt(tmpCur.getColumnIndex(COLUMN_NAME_LOCKED));
                String locked = (lock == 1 ? "LOCKED" : "UNLOCKED");

                String smsAddress = null;
                if (i == 3) {
                    String threadId = tmpCur.getString(tmpCur.getColumnIndex(COLUMN_NAME_THREAD_ID));
                    Cursor draftCursor = mContext
                        .getContentResolver()
                        .query(Uri.parse("content://sms"), 
                               new String[] { TRICKY_TO_GET_DRAFT_SMS_ADDRESS + threadId + " --" },
                               null,
                               null,
                               null);

                    if (draftCursor != null) {
                        if(draftCursor.moveToFirst()) {
                            smsAddress = draftCursor.getString(draftCursor.getColumnIndex(COLUMN_NAME_ADDRESS));
                        }
                        draftCursor.close();
                    }
                } else {
                    smsAddress = tmpCur.getString(tmpCur.getColumnIndex(COLUMN_NAME_ADDRESS));
                }

                if (smsAddress == null) {
                    smsAddress = "";
                }

                String sc = tmpCur.getString(tmpCur.getColumnIndex(COLUMN_NAME_SC));

                String body = tmpCur.getString(tmpCur.getColumnIndex(COLUMN_NAME_BODY));

                if (body != null) {
                    StringBuffer sbf = new StringBuffer(body);
                    int num = 0;
                    num = sbf.indexOf("END:VBODY");
                    do {
                        if (num >= 0) {
                            sbf.insert(num, "/");
                        } else {
                            break;
                        }
                    } while ((num = sbf.indexOf("END:VBODY", num + 1 + "END:VBODY".length())) >= 0);
                    body = sbf.toString();
                } else {
                    body = "";
                }

                try {
                    if(mWriter != null) {
                        mWriter.write(combineVmsg(timeStamp,readByte,boxType,mSlotid,locked,smsAddress,body,seen));
                        result = true;
                    }
                } catch (Exception e) {
                    MyLogger.logE(CLASS_TAG, "mWriter.write() failed");
                } finally {
                    tmpCur.moveToNext();
                }
                break;
            }
        }

        return result;
    }

    /**
     * Describe <code>onStart</code> method here.
     *
     */
    public final void onStart() {
        super.onStart();
        MyLogger.logE(CLASS_TAG, "onStart():mParentFolderPath:" + mParentFolderPath);

        if(getCount() > 0) {
            File path = new File(mParentFolderPath + File.separator + mStoragePath);
            if (!path.exists()) {
                path.mkdirs();
            }

            File file = new File(path.getAbsolutePath() + File.separator + mStorageName);
            if (!file.exists()) {
                try {
                    file.createNewFile();
                } catch (Exception e) {
                    MyLogger.logE(CLASS_TAG, "onStart():file:" + file.getAbsolutePath());
                    MyLogger.logE(CLASS_TAG, "onStart():create file failed");
                }
            }

            try {
                mWriter = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file)));
            } catch (Exception e) {
                MyLogger.logE(CLASS_TAG, "new BufferedWriter failed");
            }
        }
    }


    /**
     * Describe <code>onEnd</code> method here.
     * 
     */
    public final void onEnd() {
        super.onEnd();
        try {
            MyLogger.logD(CLASS_TAG, "SmsBackupComposer onEnd");
            if (mWriter != null) {
                MyLogger.logE(CLASS_TAG, "mWriter.close()");
                mWriter.close();
            }
        } catch (Exception e) {
            MyLogger.logE(CLASS_TAG, "mWriter.close() failed");
        }

        for (Cursor cur : mSmsCursorArray) {
            if (cur != null) {
                cur.close();
                cur = null;
            }
        }
    }

    private static final String UTF = "UTF-8";
    private static final String QUOTED = "QUOTED-PRINTABLE";
    private static final String CHARSET = "CHARSET=";
    private static final String ENCODING = "ENCODING=";
    private static final String VMESSAGE_END_OF_SEMICOLON = ";";
    private static final String VMESSAGE_END_OF_COLON = ":";
    private static final String VMESSAGE_END_OF_LINE = "\r\n";
    private static final String BEGIN_VMSG = "BEGIN:VMSG";
    private static final String END_VMSG = "END:VMSG";
    private static final String VERSION = "VERSION:";
    private static final String BEGIN_VCARD = "BEGIN:VCARD";
    private static final String END_VCARD = "END:VCARD";
    private static final String BEGIN_VBODY = "BEGIN:VBODY";
    private static final String END_VBODY = "END:VBODY";
    private static final String FROMTEL = "FROMTEL:";
    private static final String XBOX = "X-BOX:";
    private static final String XREAD = "X-READ:";
    private static final String XSEEN = "X-SEEN:";
    private static final String XSIMID = "X-SIMID:";
    private static final String XLOCKED = "X-LOCKED:";
    private static final String XTYPE = "X-TYPE:";
    private static final String DATE = "Date:";
    private static final String SUBJECT = "Subject;";

    public static String combineVmsg(String TimeStamp, String ReadByte, String BoxType,
                               String mSlotid, String Locked, String SmsAddress, String body,
            String mseen) {
        StringBuilder mBuilder = new StringBuilder();
        mBuilder.append(BEGIN_VMSG);
        mBuilder.append(VMESSAGE_END_OF_LINE);
        mBuilder.append(VERSION);
        mBuilder.append("1.1");
        mBuilder.append(VMESSAGE_END_OF_LINE);
        mBuilder.append(BEGIN_VCARD);
        mBuilder.append(VMESSAGE_END_OF_LINE);
        mBuilder.append(FROMTEL);
        mBuilder.append(SmsAddress);
        mBuilder.append(VMESSAGE_END_OF_LINE);
        mBuilder.append(END_VCARD);
        mBuilder.append(VMESSAGE_END_OF_LINE);
        mBuilder.append(BEGIN_VBODY);
        mBuilder.append(VMESSAGE_END_OF_LINE);
        mBuilder.append(XBOX);
        mBuilder.append(BoxType);
        mBuilder.append(VMESSAGE_END_OF_LINE);
        mBuilder.append(XREAD);
        mBuilder.append(ReadByte);
        mBuilder.append(VMESSAGE_END_OF_LINE);
        mBuilder.append(XSEEN);
        mBuilder.append(mseen);
        mBuilder.append(VMESSAGE_END_OF_LINE);
        mBuilder.append(XSIMID);
        mBuilder.append(mSlotid);
        mBuilder.append(VMESSAGE_END_OF_LINE);
        mBuilder.append(XLOCKED);
        mBuilder.append(Locked);
        mBuilder.append(VMESSAGE_END_OF_LINE);
        mBuilder.append(XTYPE);
        mBuilder.append("SMS");
        mBuilder.append(VMESSAGE_END_OF_LINE);
        mBuilder.append(DATE);
        mBuilder.append(TimeStamp);
        mBuilder.append(VMESSAGE_END_OF_LINE);
        mBuilder.append(SUBJECT);
        mBuilder.append(ENCODING);
        mBuilder.append(QUOTED);
        mBuilder.append(VMESSAGE_END_OF_SEMICOLON);
        mBuilder.append(CHARSET);
        mBuilder.append(UTF);
        mBuilder.append(VMESSAGE_END_OF_COLON);
        mBuilder.append(body);
        mBuilder.append(VMESSAGE_END_OF_LINE);
        mBuilder.append(END_VBODY);
        mBuilder.append(VMESSAGE_END_OF_LINE);
        mBuilder.append(END_VMSG);
        mBuilder.append(VMESSAGE_END_OF_LINE);
        return mBuilder.toString();
    }

    private String formatTimeStampString(Context context, long when) {// boolean fullFormat
        CharSequence formattor = DateFormat.format("yyyy/MM/dd kk:mm:ss", when);
        return formattor.toString();
    }
}
