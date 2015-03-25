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

import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.Telephony.Mms;
import com.google.android.mms.InvalidHeaderValueException;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.NotificationInd;
import com.google.android.mms.pdu.PduComposer;
import com.google.android.mms.pdu.PduPersister;
import com.google.android.mms.pdu.RetrieveConf;
import com.google.android.mms.pdu.SendReq;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.datatransfer.utils.Constants;
import com.mediatek.datatransfer.utils.ModuleType;
import com.mediatek.datatransfer.utils.MyLogger;
import com.mediatek.datatransfer.utils.Constants.ModulePath;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import static com.google.android.mms.pdu.PduHeaders.MESSAGE_TYPE_NOTIFICATION_IND;
import static com.google.android.mms.pdu.PduHeaders.MESSAGE_TYPE_RETRIEVE_CONF;

public class MmsBackupComposer extends Composer {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/MmsBackupComposer";
    //private static final String MMS_SPECIAL_TYPE = "134";
    private static final String[] MMS_EXCLUDE_TYPE = {"134", "130"};
    private static final String COLUMN_NAME_ID = "_id";
    private static final String COLUMN_NAME_TYPE = "m_type";
    private static final String COLUMN_NAME_DATE = "date";
    private static final String COLUMN_NAME_MESSAGE_BOX = "msg_box";
    private static final String COLUMN_NAME_READ = "read";
    //private static final String COLUMN_NAME_ST = "st";
    private static final String COLUMN_NAME_SIMID = "sim_id";
    private static final String COLUMN_NAME_LOCKED = "locked";

    private static final Uri[] mMmsUriArray = {
        Mms.Sent.CONTENT_URI,
        //Mms.Outbox.CONTENT_URI,
        //Mms.Draft.CONTENT_URI,
        Mms.Inbox.CONTENT_URI
    };
    private Cursor[] mMmsCursorArray = { null, null };

    private int mMmsIndex;
    private MmsXmlComposer mXmlComposer;
    private Object mLock = new Object();
    private ArrayList<MmsBackupContent> mPduList = null;
    private ArrayList<MmsBackupContent> mTempPduList = null;
    private static final String mStoragePath = "mms";

    public MmsBackupComposer(Context context) {
        super(context);
    }


    /**
     * Describe <code>getModuleType</code> method here.
     *
     * @return an <code>int</code> value
     */
    public int getModuleType() {
        return ModuleType.TYPE_MMS;
    }


    /**
     * Describe <code>getCount</code> method here.
     *
     * @return an <code>int</code> value
     */
    public int getCount() {
        int count = 0;
        for (Cursor cur : mMmsCursorArray) {
            if (cur != null && !cur.isClosed() && cur.getCount() > 0) {
                count += cur.getCount();
            }
        }

        MyLogger.logD(CLASS_TAG, "getCount():" + count);
        return count;
    }


    /**
     * Describe <code>init</code> method here.
     *
     * @return a <code>boolean</code> value
     */
    public boolean init() {
        boolean result = false;
        mTempPduList = new ArrayList<MmsBackupContent>();
        for (int i = 0; i < mMmsUriArray.length; ++i) {
            if (mMmsUriArray[i] == Mms.Inbox.CONTENT_URI) {
                mMmsCursorArray[i] = mContext.getContentResolver().query(mMmsUriArray[i], null,
                        "m_type <> ? AND m_type <> ?", MMS_EXCLUDE_TYPE , null);
            } else {
                mMmsCursorArray[i] = mContext.getContentResolver().query(mMmsUriArray[i], null,
                        null, null, null);
            }
            if (mMmsCursorArray[i] != null) {
                mMmsCursorArray[i].moveToFirst();
                result = true;
            }
        }

        MyLogger.logD(CLASS_TAG, "init():" + result + " count:" + getCount());
        return result;
    }


    /**
     * Describe <code>isAfterLast</code> method here.
     *
     * @return a <code>boolean</code> value
     */
    public boolean isAfterLast() {
        boolean result = true;
        for (Cursor cur : mMmsCursorArray) {
            if (cur != null && !cur.isAfterLast()) {
                result = false;
                break;
            }
        }

        MyLogger.logD(CLASS_TAG, "isAfterLast():" + result);
        return result;
    }


    /**
     * Describe <code>composeOneEntity</code> method here.
     *
     * @return a <code>boolean</code> value
     */
    public boolean composeOneEntity() {
        return implementComposeOneEntity();
    }


    /**
     * Describe <code>implementComposeOneEntity</code> method here.
     *
     * @return a <code>boolean</code> value
     */
    public boolean implementComposeOneEntity() {
        boolean result = false;
        byte[] pduMid;

        for (int i = 0; i < mMmsCursorArray.length; ++i) {
            if (mMmsCursorArray[i] != null && !mMmsCursorArray[i].isAfterLast()) {
                int id = mMmsCursorArray[i].getInt(mMmsCursorArray[i]
                        .getColumnIndex(COLUMN_NAME_ID));
                Uri realUri = ContentUris.withAppendedId(mMmsUriArray[i], id);
                MyLogger.logD(CLASS_TAG, "id:" + id + ",realUri:" + realUri);

                PduPersister p = PduPersister.getPduPersister(mContext);
                try {
                    if (mMmsUriArray[i] == Mms.Inbox.CONTENT_URI) {
                        int type = mMmsCursorArray[i].getInt(mMmsCursorArray[i]
                                .getColumnIndex(COLUMN_NAME_TYPE));
                        MyLogger.logD(CLASS_TAG, "inbox, m_type:" + type);
                        if (type == MESSAGE_TYPE_NOTIFICATION_IND) {
                            NotificationInd nPdu = (NotificationInd) p.load(realUri);
                            pduMid = new PduComposer(mContext, nPdu).make(true);
                        } else if (type == MESSAGE_TYPE_RETRIEVE_CONF) {
                            RetrieveConf rPdu = (RetrieveConf) p.load(realUri, true);
                            pduMid = new PduComposer(mContext, rPdu).make(true);
                        } else {
                            pduMid = null;
                        }
                    } else {
                        SendReq sPdu = (SendReq) p.load(realUri);
                        pduMid = new PduComposer(mContext, sPdu).make();
                    }

                    if (pduMid != null) {
                        //printPdu(pduMid);
                        String fileName = Integer.toString(mMmsIndex++) + ModulePath.FILE_EXT_PDU;
                        String isRead = mMmsCursorArray[i].getString(mMmsCursorArray[i]
                                                                     .getColumnIndex(COLUMN_NAME_READ));
                        String msgBox = mMmsCursorArray[i].getString(mMmsCursorArray[i]
                                                                     .getColumnIndex(COLUMN_NAME_MESSAGE_BOX));
                        String date = mMmsCursorArray[i].getString(mMmsCursorArray[i]
                                                                   .getColumnIndex(COLUMN_NAME_DATE));
                        // String simId = mMmsCursorArray[i].getString(mMmsCursorArray[i]
                        //                                             .getColumnIndex(COLUMN_NAME_SIMID));
                        long simId = mMmsCursorArray[i].getLong(mMmsCursorArray[i]
                                                                  .getColumnIndex(COLUMN_NAME_SIMID));
                        String slotId = "0";
                        if(FeatureOption.MTK_GEMINI_SUPPORT == true && simId >= 0) {
                            /*
                             * M: remove getSlotById method.
                             */
//                            int slot = SimInfoManager.getSlotById(mContext, simId);
                            SimInfoRecord simInfo = SimInfoManager.getSimInfoById(mContext, simId);
                            int slot = simInfo.mSimSlotId;
                            slotId = String.valueOf(slot + 1);
                        }

                        String isLocked = mMmsCursorArray[i].getString(mMmsCursorArray[i]
                                                                    .getColumnIndex(COLUMN_NAME_LOCKED));
                        MmsXmlInfo record = new MmsXmlInfo();
                        record.setID(fileName);
                        record.setIsRead(isRead);

                        record.setMsgBox(msgBox);
                        record.setDate(date);
                        record.setSize(Integer.toString(pduMid.length));
                        record.setSimId(slotId);
                        record.setIsLocked(isLocked);
                        MmsBackupContent tmpContent = new MmsBackupContent();
                        tmpContent.pduMid = pduMid;
                        tmpContent.fileName = fileName;
                        tmpContent.record = record;
                        mTempPduList.add(tmpContent);
                    }

                    if (mMmsIndex % Constants.NUMBER_IMPORT_MMS_EACH == 0
                            || mMmsIndex >= getCount()) {
                        if (mPduList != null) {
                            synchronized (mLock) {
                                try {
                                    MyLogger.logD(CLASS_TAG, MyLogger.MMS_TAG
                                            + "wait for WriteFileThread:");
                                    mLock.wait();
                                } catch (InterruptedException e) {
                                }
                            }
                        }
                        mPduList = mTempPduList;
                        new WriteFileThread().start();
                        if (!isAfterLast()) {
                            mTempPduList = new ArrayList<MmsBackupContent>();
                        }
                    }

                    result = true;
                } catch (InvalidHeaderValueException e) {
                } catch (MmsException e) {
                } finally {
                    // mMmsCur[i].moveToNext();
                }

                mMmsCursorArray[i].moveToNext();
                break;
            }
        }

        MyLogger.logD(CLASS_TAG, "implementComposeOneEntity:" + result);
        return result;
    }


    /**
     * Describe <code>onStart</code> method here.
     *
     */
    public void onStart() {
        super.onStart();
        if(getCount() > 0) {
            if ((mXmlComposer = new MmsXmlComposer()) != null) {
                mXmlComposer.startCompose();
            }

            File path = new File(mParentFolderPath + File.separator + ModulePath.FOLDER_MMS);
            if (!path.exists()) {
                path.mkdirs();
            } else {
                File[] files = path.listFiles();
                for(File file : files) {
                    if(file.isFile()) {
                        file.delete();
                    }
                }
            }
        }
    }


    /**
     * Describe <code>onEnd</code> method here.
     *
     */
    public void onEnd() {
        if (mPduList != null) {
            synchronized (mLock) {
                try {
                    MyLogger.logD(CLASS_TAG, MyLogger.MMS_TAG + "wait for WriteFileThread:");
                    mLock.wait();
                } catch (InterruptedException e) {
                }
            }
        }

        super.onEnd();
        if (mXmlComposer != null) {
            mXmlComposer.endCompose();
            String msgXmlInfo = mXmlComposer.getXmlInfo();
            if (getComposed() > 0 && msgXmlInfo != null) {
                try {
                    writeToFile(mParentFolderPath + File.separator + ModulePath.FOLDER_MMS + File.separator + ModulePath.MMS_XML,
                                msgXmlInfo.getBytes());
                } catch (IOException e) {
                    if (super.mReporter != null) {
                        super.mReporter.onErr(e);
                    }
                }
            }
        }

        for (Cursor cur : mMmsCursorArray) {
            if (cur != null) {
                cur.close();
                cur = null;
            }
        }
    }


    private class WriteFileThread extends Thread {
        @Override
        public void run() {
            for (int j = 0; (mPduList != null) && (j < mPduList.size()); ++j) {
                byte[] pduByteArray = mPduList.get(j).pduMid;
                String fileName = mPduList.get(j).fileName;

                try {
                    if (pduByteArray != null) {
                        MyLogger.logD(CLASS_TAG, MyLogger.MMS_TAG + "WriteFileThread() pduMid.length:"
                                + pduByteArray.length);
                        writeToFile(mParentFolderPath + File.separator + ModulePath.FOLDER_MMS + File.separator + fileName, pduByteArray);
                        if (mXmlComposer != null) {
                            mXmlComposer.addOneMmsRecord(mPduList.get(j).record);
                        }

                        increaseComposed(true);
                        MyLogger.logD(CLASS_TAG, "WriteFileThread() addFile:" + fileName + " success");
                    }
                } catch (IOException e) {
                    if (mReporter != null) {
                        mReporter.onErr(e);
                    }
                    MyLogger.logE(CLASS_TAG, MyLogger.MMS_TAG + "WriteFileThread() addFile:" + fileName
                            + " fail");
                }
            }

            synchronized (mLock) {
                mPduList = null;
                mLock.notifyAll();
            }
        }
    }

    
    /**
     * Describe class <code>MmsBackupContent</code> here.
     *
     */
    private class MmsBackupContent {
        public byte[] pduMid;
        public String fileName;
        MmsXmlInfo record;
    }

    /**
     * Describe <code>writeToFile</code> method here.
     * 
     * @param fileName a <code>String</code> value
     * @param buf a <code>byte</code> value
     * @exception IOException if an error occurs
     */
    private void writeToFile(String fileName, byte[] buf) throws IOException {
        try {
            FileOutputStream outStream = new FileOutputStream(fileName);
            // byte[] buf = inBuf.getBytes();
            outStream.write(buf, 0, buf.length);
            outStream.flush();
            outStream.close();
        } catch (IOException e) {
            throw e;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // private void printPdu(byte[] pdu) {
    //     StringBuilder str = new StringBuilder();
    //     for(int i = 0; i < pdu.length; ++i) {
    //         str.append(pdu[i]);
    //         if(i != 0) {
    //             str.append(",");
    //         }

    //     }
    //     MyLogger.logD(CLASS_TAG, MyLogger.MMS_TAG + "pduMid:" + str.toString());
    // }

}
