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

package com.mediatek.op.sms;

import static com.mediatek.common.sms.IConcatenatedSmsFwkExt.UPLOAD_FLAG_TAG;
import static com.mediatek.common.sms.IConcatenatedSmsFwkExt.UPLOAD_FLAG_NEW;
import static com.mediatek.common.sms.IConcatenatedSmsFwkExt.UPLOAD_FLAG_UPDATE;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.database.SQLException;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.provider.Telephony;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.util.HexDump;
import com.mediatek.common.sms.IConcatenatedSmsFwkExt;
import com.mediatek.common.sms.TimerRecord;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;


public class ConcatenatedSmsFwkExt implements IConcatenatedSmsFwkExt {
    private static final String TAG = "ConcatenatedSmsFwkExt";
    
    private static final Uri mRawUri = Uri.withAppendedPath(Telephony.Sms.CONTENT_URI, "raw");
    private static final String[] CONCATE_PROJECTION = {
        "reference_number",
        "count",
        "sequence"
    };
    private static final String[] PDU_SEQUENCE_PORT_PROJECTION = {
        "pdu",
        "sequence",
        "destination_port",
    };
    private static final String[] PDU_SEQUENCE_PORT_UPLOAD_PROJECTION = {
        "pdu",
        "sequence",
        "destination_port",
        "upload_flag"
    };
    
    private static final String[] OUT_OF_DATE_PROJECTION = {
        "recv_time",
        "address",
        "reference_number",
        "count",
        "sim_id",
    };
    
    protected static int DELAYED_TIME = 60 * 1000;
    
    private ArrayList<TimerRecord> mTimerRecords = new ArrayList<TimerRecord>(5);
    protected Context mContext = null;
    private ContentResolver mResolver = null;
    protected int mSimId = -1;
    
    public ConcatenatedSmsFwkExt(Context context, int simId) {
        if(context == null) {
            Xlog.d(TAG, "FAIL! context is null");
            return;
        }
        this.mContext = context;
        this.mResolver = mContext.getContentResolver();
        this.mSimId = simId;

        IntentFilter filter = new IntentFilter(ACTION_CLEAR_OUT_SEGMENTS);
        mContext.registerReceiver(mReceiver, filter);
    }
    
    private void registerAlarmManager() {
        return;
    }
    
    private void deleteOutOfDateSegments(String address, int refNum, int count, int simId) {
        Xlog.d(TAG, "call deleteOutOfDateSegments");
        
        try {
            String where = "address=? AND reference_number=? AND count=? AND sim_id=?";
            String[] whereArgs = {
                address,
                Integer.toString(refNum),
                Integer.toString(count),
                Integer.toString(simId),
            };
            int numOfDeleted = mResolver.delete(mRawUri, where, whereArgs);
            Xlog.d(TAG, "remove " + numOfDeleted + " out of date segments, ref =  " + refNum);
        } catch(SQLException e) {
            Xlog.d(TAG, "FAIL! SQLException");
        }
    }
    
    private void checkOutOfDateSegments() {
        Xlog.d(TAG, "call checkOutOfDateSegments");
        
        Cursor cursor = null;  
        try {
            String where = "sim_id=?";
            String[] whereArgs = {Integer.toString(mSimId)};
            cursor = mResolver.query(mRawUri,
                    OUT_OF_DATE_PROJECTION, where, whereArgs, null);
            Xlog.d(TAG, "checkOutOfDateSegments cursor open");
            if(cursor != null) {
                int columnRecvTime = cursor.getColumnIndex("recv_time");
                int columnAddress = cursor.getColumnIndex("address");
                int columnRefNum = cursor.getColumnIndex("reference_number");
                int columnCount = cursor.getColumnIndex("count");
                int columnSimId = cursor.getColumnIndex("sim_id");
                
                int cursorCount = cursor.getCount();
                Xlog.d(TAG, "checkOutOfDateSegments cursor size="+cursorCount+", simId="+Integer.toString(mSimId));
                for(int i = 0; i < cursorCount; ++i) {
                    cursor.moveToNext();
                    long recv_time = cursor.getLong(columnRecvTime);
                    long curr_time = System.currentTimeMillis();

                    TimerRecord tr = queryTimerRecord(cursor.getString(columnAddress), 
                                                      cursor.getInt(columnRefNum), 
                                                      cursor.getInt(columnCount));
                    if (tr != null) {
                        //recv_time = getLastReceiveTimeByTimeRecord(tr);
                        deleteTimerRecord(tr);
                    } 

                    Xlog.d(TAG, "currtime="+curr_time+", recv_time="+recv_time);
                    
                    if((curr_time - recv_time) >= OUT_OF_DATE_TIME) {
                    // delete segments which has the same address, reference_number, count & sim_id
                    deleteOutOfDateSegments(cursor.getString(columnAddress),
                            cursor.getInt(columnRefNum),
                            cursor.getInt(columnCount),
                            cursor.getInt(columnSimId));
                    }
                }			
            } else {
                Xlog.d(TAG, "FAIL! cursor is null");
            }
        } catch(SQLException e) {
            Xlog.d(TAG, "FAIL! SQLException");
        } finally {
            if (cursor != null) {
                cursor.close();
                Xlog.d(TAG, "checkOutOfDateSegments cursor close");
            }
        }
    }
    
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if(action.equals(ACTION_CLEAR_OUT_SEGMENTS)) {
                int id = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, -1);
                if(id == mSimId) {
                    // clear db
                    checkOutOfDateSegments();
                }
            }
        }
    };
    
    public synchronized boolean isFirstConcatenatedSegment(String address, int refNumber) {
        Xlog.d(TAG, "call isFirstConcatenatedSegment: " + address + "/" + refNumber);
        boolean result = false;
        
        Cursor cursor = null;  
        try {
            String where = "address=? AND reference_number=? AND sim_id=?";
            String[] whereArgs = new String[] {
                address,
                Integer.toString(refNumber),
                Integer.toString(mSimId)
            };
            cursor = mResolver.query(mRawUri,
                    CONCATE_PROJECTION, where, whereArgs, null);
            if(cursor != null) {
                int messageCount = cursor.getCount();
                if( messageCount == 0) {
                    result = true;
                }
            } else {
                Xlog.d(TAG, "FAIL! cursor is null");
            }
        } catch(SQLException e) {
            Xlog.d(TAG, "FAIL! SQLException");
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        
        Xlog.d(TAG, "isFirstConcatenatedSegment result ="+result);
        return result;
    }
    
    public synchronized boolean isLastConcatenatedSegment(String address, int refNumber, int msgCount) {
        Xlog.d(TAG, "call isLastConcatenatedSegment: " + address + "/" + refNumber);
        boolean result = false;
        
        Cursor cursor = null;  
        try {
            String where = "address=? AND reference_number=? AND sim_id=?";
            String[] whereArgs = new String[] {
                address,
                Integer.toString(refNumber),
                Integer.toString(mSimId)
            };
            cursor = mResolver.query(mRawUri,
                    CONCATE_PROJECTION, where, whereArgs, null);
            Xlog.d(TAG, "isLastConcatenatedSegment cursor open");
            if(cursor != null) {
                int messageCount = cursor.getCount();
                if (messageCount == msgCount - 1) {
                    result = true;
                }
            } else {
                Xlog.d(TAG, "FAIL! cursor is null");
            }
        } catch(SQLException e) {
            Xlog.d(TAG, "FAIL! SQLException");
        } finally {
            if (cursor != null) {
                cursor.close();
                Xlog.d(TAG, "isLastConcatenatedSegment cursor close");
            }
        }
        
        Xlog.d(TAG, "isLastConcatenatedSegment result ="+result);
        return result;
    }

    private long getLastReceiveTimeByTimeRecord(TimerRecord record) {
        Xlog.d(TAG, "call getLastReceiveTimeByTimeRecord: " + record.address + "/" + record.refNumber + "/" 
            + record.msgCount);

        long recv_time = 0;   
        Cursor cursor = null;    
        try {
            String where = "address=? AND reference_number=? AND sim_id=?";
            String[] whereArgs = new String[] {
                record.address,
                Integer.toString(record.refNumber),
                Integer.toString(mSimId)
            };
            cursor = mResolver.query(mRawUri,
                    CONCATE_PROJECTION, where, whereArgs, null);
            Xlog.d(TAG, "getLastReceiveTimeByTimeRecord cursor open");
            if(cursor != null) {
                int cursorCount = cursor.getCount();
				
                for(int i = 0; i < cursorCount; ++i) {
                    cursor.moveToNext();
                    long cursor_time=cursor.getLong(cursor.getColumnIndex("recv_time"));
                    Xlog.d(TAG, "cursor_time="+cursor_time);

                    if (cursor_time > recv_time) {
                        Xlog.d(TAG, "cursor_time replace " + recv_time);
                        recv_time = cursor_time;
                    }
                }
            }
        } catch(SQLException e) {
            Xlog.d(TAG, "FAIL! SQLException");
        } finally {
            if (cursor != null) {
                cursor.close();
                Xlog.d(TAG, "getLastReceiveTimeByTimeRecord cursor close");
            }
        }

        Xlog.d(TAG, "final : "+recv_time);
        return recv_time;
    }
        
    
    public void startTimer(Handler h, Object r) {
        Xlog.d(TAG, "call startTimer");
        boolean isParamsValid = checkParamsForMessageOperation(h, r);
        if(isParamsValid == false) {
            Xlog.d(TAG, "FAIL! invalid params");
            return;
        }
        
        addTimerRecord((TimerRecord) r);
        Message m = h.obtainMessage(EVENT_DISPATCH_CONCATE_SMS_SEGMENTS, r);
        h.sendMessageDelayed(m, DELAYED_TIME);
    }
    
    public void cancelTimer(Handler h, Object r) {
        Xlog.d(TAG, "call cancelTimer");
        boolean isParamsValid = checkParamsForMessageOperation(h, r);
        if(isParamsValid == false) {
            Xlog.d(TAG, "FAIL! invalid params");
            return;
        }
        
        h.removeMessages(EVENT_DISPATCH_CONCATE_SMS_SEGMENTS, r);
        deleteTimerRecord((TimerRecord) r);
    }
    
    public void refreshTimer(Handler h, Object r) {
        Xlog.d(TAG, "call refreshTimer");
        boolean isParamsValid = checkParamsForMessageOperation(h, r);
        if(isParamsValid == false) {
            Xlog.d(TAG, "FAIL! invalid params");
            return;
        }
        
        h.removeMessages(EVENT_DISPATCH_CONCATE_SMS_SEGMENTS, r);
        Message m = h.obtainMessage(EVENT_DISPATCH_CONCATE_SMS_SEGMENTS, r);
        h.sendMessageDelayed(m, DELAYED_TIME);
    }
    
    public synchronized TimerRecord queryTimerRecord(String address, int refNumber, int msgCount) {
        Xlog.d(TAG, "call queryTimerRecord");
        
        Xlog.d(TAG, "find record by [" + address + ", " + refNumber + ", " + msgCount +"]");
        for(TimerRecord record : mTimerRecords) {
            if(record.address.equals(address) && record.refNumber == refNumber && record.msgCount == msgCount) {
                Xlog.d(TAG, "find record");
                return record;
            }
        }
        
        Xlog.d(TAG, "don't find record");
        return null;
    }
    
    private void addTimerRecord(TimerRecord r) {
        Xlog.d(TAG, "call addTimerRecord");
        for(TimerRecord record : mTimerRecords) {
            if(record == r) {
                Xlog.d(TAG, "duplicated TimerRecord object be found");
                return;
            }
        }
        
        mTimerRecords.add(r);
    }
    
    private void deleteTimerRecord(TimerRecord r) {
        Xlog.d(TAG, "call deleteTimerRecord");
        
        if(mTimerRecords == null || mTimerRecords.size() == 0) {
            Xlog.d(TAG, "no record can be removed ");
            return;
        }
        
        int countBeforeRemove = mTimerRecords.size();
        mTimerRecords.remove(r);
        int countAfterRemove = mTimerRecords.size();
        
        int countRemoved = countBeforeRemove - countAfterRemove;
        if(countRemoved > 0) {
            Xlog.d(TAG, "remove record(s)" + countRemoved);
        } else {
            Xlog.d(TAG, "no record be removed");
        }
    }
    
    private boolean checkParamsForMessageOperation(Handler h, Object r) {
        Xlog.d(TAG, "call checkParamsForMessageOperation");
        if(h == null) {
            Xlog.d(TAG, "FAIL! handler is null");
            return false;
        }
        if(r == null) {
            Xlog.d(TAG, "FAIL! record is null");
            return false;
        }
        if(!(r instanceof TimerRecord)) {
            Xlog.d(TAG, "FAIL! param r is not TimerRecord object");
            return false;
        }
        
        return true;
    }
    
    private boolean checkTimerRecord(TimerRecord r) {
        Xlog.d(TAG, "call checkTimerRecord");
        if(mTimerRecords.size() == 0) {
            return false;
        }
        
        for(TimerRecord record : mTimerRecords) {
            if(r == record) {
                return true;
            }
        }
        
        return false;
    }
    
    public synchronized byte[][] queryExistedSegments(TimerRecord record) {
        Xlog.d(TAG, "call queryExistedSegments");
        
        byte[][] pdus = null;
        Cursor cursor=null;
        try {
            String where = "address=? AND reference_number=? AND sim_id=? AND count=?";
            String[] whereArgs = new String[] {
                record.address,
                Integer.toString(record.refNumber),
                Integer.toString(mSimId),
                Integer.toString(record.msgCount)
            };
            cursor = mResolver.query(mRawUri,
                    PDU_SEQUENCE_PORT_PROJECTION, where, whereArgs, null);
            Xlog.d(TAG, "queryExistedSegments cursor open");
            if(cursor != null) {
                byte[][] tempPdus = new byte[record.msgCount][];
                
                int columnSeqence = cursor.getColumnIndex("sequence");
                //Xlog.d(TAG, "columnSeqence = " + columnSeqence);
                int columnPdu = cursor.getColumnIndex("pdu");
                //Xlog.d(TAG, "columnPdu = " + columnPdu);
                int columnPort = cursor.getColumnIndex("destination_port");
                //Xlog.d(TAG, "columnPort = " + columnPort);
                
                int cursorCount = cursor.getCount();
                Xlog.d(TAG, "miss " + (record.msgCount - cursorCount) + " segment(s)");
                for(int i = 0; i < cursorCount; ++i) {
                    cursor.moveToNext();
                    int cursorSequence = cursor.getInt(columnSeqence);
                    Xlog.d(TAG, "queried segment " + cursorSequence + ", ref = " + record.refNumber);
                    tempPdus[cursorSequence - 1] = HexDump.hexStringToByteArray(
                            cursor.getString(columnPdu));
                    if(tempPdus[cursorSequence - 1] == null) {
                        Xlog.d(TAG, "miss segment " + cursorSequence + ", ref = " + record.refNumber);
                    }
                    
                    if(!cursor.isNull(columnPort)) {
                        Xlog.d(TAG, "segment contain port " + cursor.getInt(columnPort));
                        Xlog.d(TAG, "queryExistedSegments cursor close isnot null");
                        return null;
                    }
                }
                
                pdus = new byte[cursorCount][];
                int index = 0;
                for(int i = 0, len = tempPdus.length; i < len; ++i) {
                    if(tempPdus[i] != null) {
                        // Xlog.d(TAG, "add segment " + index + " into pdus");
                        pdus[index++] = tempPdus[i];
                    }
                }
            } else {
                Xlog.d(TAG, "FAIL! cursor is null");
            }
        } catch(SQLException e) {
            Xlog.d(TAG, "FAIL! SQLException");
            return null;
        } finally {
            if (cursor != null) {
                cursor.close();
                Xlog.d(TAG, "queryExistedSegments cursor close");
            }
        }
        
        return pdus;
    }

    public synchronized void deleteExistedSegments(TimerRecord record) {
        Xlog.d(TAG, "call deleteExistedSegments");
        
        try {
            String where = "address=? AND reference_number=? AND sim_id=?";
            String[] whereArgs = new String[] {
                record.address,
                Integer.toString(record.refNumber),
                Integer.toString(mSimId)
            };
            int numOfDeleted = mResolver.delete(mRawUri, where, whereArgs);
            Xlog.d(TAG, "deleteExistedSegments cursor open");
            Xlog.d(TAG, "remove " + numOfDeleted + " segments, ref =  " + record.refNumber);
        } catch(SQLException e) {
            Xlog.d(TAG, "FAIL! SQLException");
        }

        deleteTimerRecord(record);
    }
    
    public synchronized int getUploadFlag(TimerRecord record) {
        Xlog.d(TAG, "call getUploadFlag");
        
        Cursor cursor = null;
        try {
            String where = "address=? AND reference_number=? AND sim_id=?";
            String[] whereArgs = new String[] {
                record.address,
                Integer.toString(record.refNumber),
                Integer.toString(mSimId)
            };
            cursor = mResolver.query(mRawUri,
                    PDU_SEQUENCE_PORT_UPLOAD_PROJECTION, where, whereArgs, null);
            Xlog.d(TAG, "getUploadFlag cursor open");
            if(cursor != null) {
                while(cursor.moveToNext()) {
                int columnUpload = cursor.getColumnIndex(UPLOAD_FLAG_TAG);
                    int uploadFlag = cursor.getInt(columnUpload);
                    Xlog.d(TAG, "uploadFlag = "+uploadFlag);
                    if(uploadFlag == UPLOAD_FLAG_UPDATE) {
                        Xlog.d(TAG, "find update segment");
                        return UPLOAD_FLAG_UPDATE;
                    }
                }
                Xlog.d(TAG, "all segments are new");
                return UPLOAD_FLAG_NEW;
            } else {
                Xlog.d(TAG, "FAIL! cursor is null");
                return -1;
            }
        } catch(SQLException e) {
            Xlog.d(TAG, "FAIL! SQLException, fail to query upload_flag");
            return -1;
        } finally {
            if (cursor != null) {
                cursor.close();
                Xlog.d(TAG, "getUploadFlag cursor close");
            }
        }
    }
    
    public synchronized void setUploadFlag(TimerRecord record) {
        Xlog.d(TAG, "call setUploadFlag");
        
        try {
            String where = "address=? AND reference_number=? AND sim_id=? AND upload_flag<>?";
            String[] whereArgs = new String[] {
                record.address,
                Integer.toString(record.refNumber),
                Integer.toString(mSimId),
                Integer.toString(UPLOAD_FLAG_UPDATE)
            };
            ContentValues values = new ContentValues();
            values.put(UPLOAD_FLAG_TAG, UPLOAD_FLAG_UPDATE);
            int updatedCount = mResolver.update(mRawUri, values, where, whereArgs);
            Xlog.d(TAG, "update count: " + updatedCount);
        } catch(SQLException e) {
            Xlog.d(TAG, "FAIL! SQLException, fail to update upload flag");
        }
    }
}
