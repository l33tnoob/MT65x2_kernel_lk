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

package com.mediatek.tb.telephony;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.ContextWrapper;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.os.UserHandle;
import android.os.Message;
import android.os.PowerManager;
import android.os.UserHandle;
import android.provider.Telephony;
import android.provider.Telephony.Sms.Intents;
import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.app.AppOpsManager;
import android.app.Activity;
import android.telephony.Rlog;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.net.ConnectivityManager;
import android.text.TextUtils;
import android.Manifest;


import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.io.ByteArrayInputStream;   
import java.io.ByteArrayOutputStream;   
import java.io.IOException;   
import java.io.ObjectInputStream;   
import java.io.ObjectOutputStream; 

import static android.net.ConnectivityManager.TYPE_MOBILE;
import com.android.internal.telephony.WspTypeDecoder;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.SmsApplication;


import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.tb.telephony.OnlyOwnerSimSupport;
import com.mediatek.common.telephony.IOnlyOwnerSimSupport;



public class OnlyOwnerSimSupportTB extends OnlyOwnerSimSupport {
    private static final String TAG = "OnlyOwnerSimSupportTB";
    private static final boolean DEBUG =true;
    
    private Context mContext = null;
    private PowerManager.WakeLock mWakeLock;
    private int mUserId = UserHandle.USER_OWNER;
    private final int WAKE_LOCK_TIMEOUT = 5000;
    private PendingIntent mPendingIntent;
    private ArrayList<PendingIntent> mPendingIntents;
    private boolean userReceiverFlag = false;
    private NormalUserReceivedImpl mNormalUserReceivedImpl = null;
    private NormalUserMsgBroadcastReceiver mNormalUserMsgBroadcastReceiver = null;
    private SwitchOwnerBroadcastReceiver mSwitchOwnerBroadcastReceiver = null;
    
    
    private final BroadcastReceiver mUserReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if(Intent.ACTION_USER_SWITCHED.equals(action)){
                mUserId = intent.getIntExtra(Intent.EXTRA_USER_HANDLE, UserHandle.USER_OWNER);
                if(DEBUG)
                    Rlog.d(TAG, "onReceive user switch userId = " + mUserId);

                Intent i = new Intent("mediatek.action.USER_SWITCH");
                i.putExtra(Intent.EXTRA_USER_HANDLE, mUserId);
                mContext.sendOrderedBroadcast(i, null);
                
            }
            /*if (mUserId == UserHandle.USER_OWNER) {
                Intent i = new Intent("mediatek.action.USER_SWITCH_TO_OWNER");
                mWakeLock.acquire(WAKE_LOCK_TIMEOUT);
                mContext.sendOrderedBroadcast(i, null);
            }*/
        }
    };

    
    
    private void createWakelock() {
        PowerManager pm = (PowerManager)mContext.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "SMSDispatcher");
        mWakeLock.setReferenceCounted(true);
    }

    public OnlyOwnerSimSupportTB() {
    }
    
    public OnlyOwnerSimSupportTB(Context context) {
        if(context == null) {
            Rlog.e(TAG, "FAIL! context is null");
            return;
        }
        mContext = context;
        createWakelock();
        
        IntentFilter userFilter = new IntentFilter();
        userFilter.addAction(Intent.ACTION_USER_SWITCHED);
        mContext.registerReceiver(mUserReceiver, userFilter);
        userReceiverFlag =true;
    }
    
    public OnlyOwnerSimSupportTB(Context context, boolean enableNormalUserReceived){
        if(context == null) {
            Rlog.e(TAG, "FAIL! context is null");
            return;
        }  
        mContext = context;
        createWakelock();
        
        IntentFilter userFilter = new IntentFilter();
        userFilter.addAction(Intent.ACTION_USER_SWITCHED);
        mContext.registerReceiver(mUserReceiver, userFilter);
        userReceiverFlag =true;

        if(enableNormalUserReceived){
            mNormalUserReceivedImpl = new NormalUserReceivedImpl(mContext);
            if(DEBUG)
                Rlog.d(TAG,"new NormalUserReceivedImpl....");
            
            mNormalUserMsgBroadcastReceiver = new NormalUserMsgBroadcastReceiver();
            IntentFilter filter = new IntentFilter();
            filter.addAction(IOnlyOwnerSimSupport.MTK_NORMALUSER_SMS_ACTION);
            filter.addAction(IOnlyOwnerSimSupport.MTK_NORMALUSER_MMS_ACTION);
            filter.addAction(IOnlyOwnerSimSupport.MTK_NORMALUSER_CB_ACTION);
            mContext.registerReceiver(mNormalUserMsgBroadcastReceiver, filter);
            if(DEBUG)
                Rlog.d(TAG,"new NormalUserMsgBroadcastReceiver....");

            mSwitchOwnerBroadcastReceiver = new SwitchOwnerBroadcastReceiver();
            IntentFilter fliter_MTKSwitchOwner = new IntentFilter();
            fliter_MTKSwitchOwner.addAction("mediatek.action.USER_SWITCH");
            mContext.registerReceiver(mSwitchOwnerBroadcastReceiver, fliter_MTKSwitchOwner);
            if(DEBUG)
                Rlog.d(TAG,"new SwitchOwnerBroadcastReceiver....");
       
        }
        
    }
    
    public void dispatchMsgOwner(Intent intent, int simId, String permission, int appOp) {
        //Do not use this method  in kk.
        if(DEBUG)
            Rlog.d(TAG, "call dispatchOwner:  nothing to do  ");
            
        /*
        String action = intent.getAction();
        if(DEBUG)
            Rlog.d(TAG, "call dispatchOwner: action = " + action + " permission = " + permission);
        
        if (action.equals(Intents.SMS_RECEIVED_ACTION)) { // SMS
            intent.setAction("mediatek.Telephony.NORMALUSER_SMS_RECEIVED");
        } else if (action.equals(Intents.WAP_PUSH_RECEIVED_ACTION)) { // MMS
            intent.setAction("mediatek.Telephony.NORMALUSER_MMS_RECEIVED");
        } else if (action.equals(Intents.SMS_CB_RECEIVED_ACTION)) { // CB
            intent.setAction("mediatek.Telephony.NORMALUSER_CB_RECEIVED");
        } 
        
        intent.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY, simId);
        
        // Hold a wake lock for WAKE_LOCK_TIMEOUT seconds, enough to give any
        // receivers time to take their own wake locks.
        mWakeLock.acquire(WAKE_LOCK_TIMEOUT);

        //appOp will be added into Intent 
        mContext.sendOrderedBroadcast(intent, permission);
        */
    }
    
    public boolean isCurrentUserOwner() {
        if(DEBUG)
            Rlog.d(TAG,"isCurrentUserOwner  userReceiverFlag = " + userReceiverFlag);
        if(!isOnlyOwnerSimSupport()) return true;
        if(userReceiverFlag){
            if(DEBUG)
                Rlog.v(TAG,"mUserId:" + mUserId);    
            return mUserId == UserHandle.USER_OWNER;

        }else {
            if(DEBUG)
                Rlog.d(TAG,"UserHandle.myUserId: " + UserHandle.myUserId());
            return UserHandle.myUserId() == UserHandle.USER_OWNER;
        }        
    }
    
    public boolean isOnlyOwnerSimSupport(){
        if(DEBUG)
            Rlog.d(TAG,"FeatureOption: MTK_ONLY_OWNER_SIM_SUPPORT is  " + FeatureOption.MTK_ONLY_OWNER_SIM_SUPPORT);
        return FeatureOption.MTK_ONLY_OWNER_SIM_SUPPORT;
    }
    public boolean isMsgDispatchOwner(Intent intent, String permission, int appOp){
        if(!isCurrentUserOwner()){
            String action = intent.getAction();
            if(DEBUG){
                Rlog.d(TAG, "isMsgDispatchOwner:action = " + action + " permission = " + permission);
            }
            if ((action.equals(Intents.SMS_RECEIVED_ACTION) && permission.equals("android.permission.RECEIVE_SMS")) ||  // SMS
                (action.equals(Intents.WAP_PUSH_RECEIVED_ACTION) && permission.equals("android.permission.RECEIVE_MMS")) ||  // MMS
                (action.equals(Intents.SMS_CB_RECEIVED_ACTION) && permission.equals("android.permission.RECEIVE_SMS"))) {  // CB
                 return true;
            }
        }
        return false;

    }

    public boolean isNetworkTypeMobile(int networkType){
        if(DEBUG)
            Rlog.d(TAG,"checkNetworkType:networkType is " +networkType);
        if(!isCurrentUserOwner()){
            //return networkType == ConnectivityManager.TYPE_MOBILE;
            return ConnectivityManager.isNetworkTypeMobile(networkType);
        }
        return false;
    }
   /* public void intercept(){
        return ;
    }*/
    
    public void intercept(Object obj, int resultCode){
        if(obj == null){
            Rlog.e(TAG,"call intercept, Object is:null ");
            return ;
        }
        if(obj.getClass().equals(PendingIntent.class)){
            if(DEBUG)
                Rlog.d(TAG,"call intercept, Object is:PendingIntent ");
            send((PendingIntent)obj,resultCode);
        }else if(obj.getClass().equals(ArrayList.class)){
            ArrayList al = (ArrayList)obj;
            if(al.size()>0){ 
                //ensure ArrayList<PengingIntent>
                for(int i = 0; i<al.size(); i++){
                    if(!(al.get(i).getClass().equals(PendingIntent.class))){
                        Rlog.e(TAG,"parameter of intercept must be PendingIntent or ArrayList<PendingIntern>");
                        return;
                    }
                }
                if(DEBUG)
                    Rlog.d(TAG,"call intercept, Object is:ArrayList<PendingIntent> ");
                mPendingIntents = al;
                send(mPendingIntents,resultCode);
            }
        }else{
            Rlog.e(TAG,"parameter of intercept must be PendingIntent or ArrayList<PendingIntern> ");
        }


    }   
        
    public void send(PendingIntent sentIntent, int resultCode){
        try {
            if (sentIntent != null) {
                sentIntent.send(resultCode);
            }
        } catch (CanceledException ex) {
            ex.printStackTrace();
        }
    }
    public void send(ArrayList<PendingIntent> sentIntents, int resultCode){
        if (sentIntents != null) {
            for (PendingIntent sentIntent : sentIntents) {
                send(sentIntent,resultCode);
            }
        }
    }


   private final class SwitchOwnerBroadcastReceiver extends BroadcastReceiver {
           @Override
           public void onReceive(Context context, Intent intent) {
               if(mNormalUserReceivedImpl == null){
                   Rlog.e(TAG,"mNormalUserReceivedImpl is null");
                   return;
               }
               String action = intent.getAction();
               if ("mediatek.action.USER_SWITCH".equals(action)) { 
                    int userId = intent.getIntExtra(Intent.EXTRA_USER_HANDLE, UserHandle.USER_OWNER);
                    if(DEBUG)
                        Rlog.d(TAG," mediatek.action.USER_SWITCH: userId is  " + userId);
                    if(userId == UserHandle.USER_OWNER){
                        mNormalUserReceivedImpl.handleSwitchToOwner();
                    }
               } 
               if(DEBUG)
                   Rlog.d(TAG,"SwitchOwnerBroadcastReceiver, abortBroadcast ");
               abortBroadcast();
               
           } 
   } 




   private final class NormalUserMsgBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            if(mNormalUserReceivedImpl == null){
                Rlog.e(TAG,"mNormalUserReceivedImpl is null");
                return;
            }
            String action = intent.getAction();
            if(DEBUG)
                Rlog.d(TAG,"NormalUserMsgBroadcastReceiver, action = " + action);
            if (IOnlyOwnerSimSupport.MTK_NORMALUSER_SMS_ACTION.equals(action)) { // SMS
                mNormalUserReceivedImpl.handleUserSmsReceived(intent);
            } else if (IOnlyOwnerSimSupport.MTK_NORMALUSER_MMS_ACTION.equals(action)) { // MMS
                mNormalUserReceivedImpl.handleUserMmsReceived(intent);                
            } else if (IOnlyOwnerSimSupport.MTK_NORMALUSER_CB_ACTION.equals(action)) { // CB
                mNormalUserReceivedImpl.handleUserCBReceived(intent);
            } 
            if(DEBUG)
                Rlog.d(TAG," NormalUserMsgBroadcastReceiver, abortBroadcast ");
            abortBroadcast();
        }
        
    } 

/// M:For MultiUser 3gdatasms, when receive SMS/MMS/CB in normal user @{
    private final class NormalUserReceivedImpl extends ContextWrapper{

        //private static final String TAG = "NormalUserReceivedImpl";
        private Context mContext;
        //private int mUserId = UserHandle.USER_OWNER;
    
    
        private void dispatchSmsPdus(byte[] smsPdu, String format, int simId) {
            if(DEBUG)
                Rlog.d(TAG, "begin to dispatchSmsPdus");
            byte[][] pdus = new byte[1][];
            pdus[0] = smsPdu;
        
            //Intent intent = new Intent(Intents.SMS_RECEIVED_ACTION);
            Intent intent = new Intent(Intents.SMS_DELIVER_ACTION);

            intent.putExtra("pdus", pdus);
            intent.putExtra("format", format);
            intent.putExtra("simId", simId);
        
            //mContext.sendOrderedBroadcast(intent, "android.permission.RECEIVE_SMS");

            // Direct the intent to only the default SMS app. If we can't find a default SMS app
            // then sent it to all broadcast receivers.
            ComponentName componentName = SmsApplication.getDefaultSmsApplication(mContext, true);
            if (componentName != null) {
                // Deliver SMS message only to this receiver
                intent.setComponent(componentName);
                if(DEBUG)
                    Rlog.d(TAG,"Delivering SMS to: " + componentName.getPackageName() +
                            " " + componentName.getClassName());
            }
            BroadcastReceiver resultReceiver = new MsgBroadcastReceiver();
            dispatchIntent(intent, android.Manifest.permission.RECEIVE_SMS,
                AppOpsManager.OP_RECEIVE_SMS, resultReceiver);
            if(DEBUG)
                Rlog.d(TAG, "end to dispatchSmsPdus");
        }
        private void dispatchMmsPdus(int simId, int transactionId, int pduType, byte[] header, byte[] data,
                                    String address, String service_center, String type) {
            if(DEBUG)
                Rlog.d(TAG, "begin to dispatchMmsPdus");
        
           // Intent intent = new Intent(Intents.WAP_PUSH_RECEIVED_ACTION);
            Intent intent = new Intent(Intents.WAP_PUSH_DELIVER_ACTION);
           
            intent.setType("application/vnd.wap.mms-message");
            intent.putExtra("transactionId", transactionId);
            intent.putExtra("pduType", pduType);
            intent.putExtra("header", header);
            intent.putExtra("data", data);
            intent.putExtra("address", address);
            intent.putExtra("service_center", service_center);
            intent.putExtra("simId", simId);
        
            //mContext.sendOrderedBroadcast(intent, "android.permission.RECEIVE_MMS");

         
            // Direct the intent to only the default MMS app. If we can't find a default MMS app
            // then sent it to all broadcast receivers.
            ComponentName componentName = SmsApplication.getDefaultMmsApplication(mContext, true);
            if (componentName != null) {
                // Deliver MMS message only to this receiver
                intent.setComponent(componentName);
                if(DEBUG)
                    Rlog.d(TAG, "Delivering MMS to: " + componentName.getPackageName() +
                            " " + componentName.getClassName());
            }
 
            BroadcastReceiver resultReceiver = new MsgBroadcastReceiver();
            String permission;
            int appOp;
            String mimeType = type;

            if (mimeType.equals(WspTypeDecoder.CONTENT_TYPE_B_MMS)) {
                Rlog.d(TAG, "WapPush set permission for RECEIVE_MMS");
                permission = android.Manifest.permission.RECEIVE_MMS;
                appOp = AppOpsManager.OP_RECEIVE_MMS;
            } else {
                Rlog.d(TAG, "WapPush set permission for RECEIVE_WAP_PUSH");
                permission = android.Manifest.permission.RECEIVE_WAP_PUSH;
                appOp = AppOpsManager.OP_RECEIVE_WAP_PUSH;
            }

            dispatchIntent(intent, permission, appOp, resultReceiver);
            if(DEBUG)
                Rlog.d(TAG, "end to dispatchMmsPdus");
        }
    
        private void dispatchCBPdus(String action, byte[] cbPdu, int simId) {
            if(DEBUG)
                Rlog.d(TAG, "begin to dispatchCBPdus");
        
            byte[][] pdus = new byte[1][];
            pdus[0] = cbPdu;
            String permission;
            Intent intent = new Intent(action);
        
            if (action.equals(Intents.SMS_CB_RECEIVED_ACTION)) {
                permission = "android.permission.RECEIVE_SMS";
            } else {
                Rlog.e(TAG, "illegal action");
                return;
            }
        
            intent.putExtra("pdus", pdus);
            intent.putExtra("simId", simId);
        
            //mContext.sendOrderedBroadcast(intent, permission);


            String receiverPermission;
            int appOp;

            //Intent intent = new Intent(Telephony.Sms.Intents.SMS_CB_RECEIVED_ACTION);
            receiverPermission = Manifest.permission.RECEIVE_SMS;
            appOp = AppOpsManager.OP_RECEIVE_SMS;
            BroadcastReceiver resultReceiver = new MsgBroadcastReceiver();

            mContext.sendOrderedBroadcast(intent, receiverPermission, appOp, resultReceiver,
                null, Activity.RESULT_OK, null, null);
            if(DEBUG)
                Rlog.d(TAG, "end to dispatchCBPdus");
        }
    
        private void processReceivedSmsInUser() {
            Uri messageUri = null;
            ContentResolver resolver = mContext.getContentResolver();
            int id;
            byte[] smsPdu;
            String format;
            int simId;
            ArrayList idList = new ArrayList();
        
            Cursor cursor = SqliteWrapper.query(mContext, resolver, Uri.parse("content://usersms"),
                                new String[]{"_id", "pdus", "format", "simId"},
                                null, null, null);
        
            if (cursor == null) {
                Rlog.e(TAG, "processReceivedSmsInUser cursor is null");
                return;
            }
            if(cursor.getCount() == 0){ 
                Rlog.i(TAG, "processReceivedSmsInUser cursor.getCount is 0");
                cursor.close();
                return;
            }
        
        
            cursor.moveToFirst(); 
            for (int i = 0; i < cursor.getCount(); i++) {
                if (mUserId == UserHandle.USER_OWNER) {
                    id = cursor.getInt(cursor.getColumnIndex("_id"));
                    smsPdu = cursor.getBlob(cursor.getColumnIndex("pdus"));
                    format = cursor.getString(cursor.getColumnIndex("format"));
                    simId = cursor.getInt(cursor.getColumnIndex("simId"));
                
                    idList.add(id);
                    dispatchSmsPdus(smsPdu, format, simId);
                    cursor.moveToNext();
                    if(DEBUG)
                        Rlog.d(TAG, "processReceivedSmsInUser , move cursor, i = " + i);
                } else {
                    break;
                }
            }
            if(DEBUG)
                Rlog.d(TAG, "processReceivedSmsInUser out of loop");     
            cursor.close();
            if(DEBUG)
                Rlog.d(TAG, "begin to delete the message in temp db");
            for (int i = 0; i < idList.size(); i++) {
                if(DEBUG)
                    Rlog.d(TAG, "deleting the message in temp db, _id = " + idList.get(i));
                resolver.delete(Uri.parse("content://usersms"), "_id = ?", new String[]{String.valueOf(idList.get(i))});
            }
        }
    
        private void processReceivedMmsInUser() {
            Uri messageUri = null;
            ContentResolver resolver = mContext.getContentResolver();
            int id;
            int simId;
            int transactionId;
            int pduType;
            byte[] header;
            byte[] data;
            String address;
            String service_center;
            //add
            String mimeType;
            ArrayList idList = new ArrayList();
        
            Cursor cursor = SqliteWrapper.query(mContext, resolver, Uri.parse("content://usermms"),
                            null, null, null, null);
        
            if (cursor == null) {
                Rlog.e(TAG, "processReceivedMmsInUser cursor is null");
                return;
            }
            if(cursor.getCount() == 0){      
                Rlog.i(TAG, "processReceivedMmsInUser cursor.getCount is 0");
                cursor.close();
                return;                
            }
        
        
            cursor.moveToFirst(); 
            for (int i = 0; i < cursor.getCount(); i++) {
                if (mUserId == UserHandle.USER_OWNER) {
                    id = cursor.getInt(cursor.getColumnIndex("_id"));
                    simId = cursor.getInt(cursor.getColumnIndex("_id"));
                    transactionId = cursor.getInt(cursor.getColumnIndex("transactionId"));
                    pduType = cursor.getInt(cursor.getColumnIndex("pduType"));
                    header = cursor.getBlob(cursor.getColumnIndex("header"));
                    data = cursor.getBlob(cursor.getColumnIndex("data"));
                    address = cursor.getString(cursor.getColumnIndex("address"));
                    service_center = cursor.getString(cursor.getColumnIndex("service_center"));
                    //add 
                    mimeType = cursor.getString(cursor.getColumnIndex("mimeType"));
                    
                    idList.add(id);
                    dispatchMmsPdus(simId, transactionId, pduType, header, data, address, service_center,mimeType);
                    cursor.moveToNext();
                    if(DEBUG)
                        Rlog.d(TAG, "processReceivedMmsInUser, move cursor, i = " + i);
                } else {
                    break;
                }
            }  
            if(DEBUG)
                Rlog.d(TAG, "processReceivedMmsInUser out of loop");     
            cursor.close();

            if(DEBUG)
                Rlog.d(TAG, "begin to delete the message in temp db");
            for (int i = 0; i < idList.size(); i++) {
                if(DEBUG)
                    Rlog.d(TAG, "processReceivedMmsInUser deleting the message in temp db, _id = " + idList.get(i));
                resolver.delete(Uri.parse("content://usermms"), "_id = ?", new String[]{String.valueOf(idList.get(i))});
            }
        }
    
        private void processReceivedCBInUser() {
            Uri messageUri = null;
            ContentResolver resolver = mContext.getContentResolver();
            int id;
            byte[] smsPdu;
            String action;
            int simId;
            ArrayList idList = new ArrayList();
        
            Cursor cursor = SqliteWrapper.query(mContext, resolver, Uri.parse("content://usercb"),
                            null, null, null, null);
        
            if (cursor == null) {
                Rlog.e(TAG, "processReceivedCBInUser cursor is null");
                return;
            }
            if(cursor.getCount() == 0){
                Rlog.i(TAG, "processReceivedCBInUser cursor.getCount is 0");
                cursor.close();
                return;
            }
        
            cursor.moveToFirst(); 
            for (int i = 0; i < cursor.getCount(); i++) {
                if (mUserId == UserHandle.USER_OWNER) {
                    id = cursor.getInt(cursor.getColumnIndex("_id"));
                    smsPdu = cursor.getBlob(cursor.getColumnIndex("pdus"));
                    action = cursor.getString(cursor.getColumnIndex("action"));
                    simId = cursor.getInt(cursor.getColumnIndex("simId"));
                
                    idList.add(id);
                    dispatchCBPdus(action, smsPdu, simId);
                    cursor.moveToNext();
                    if(DEBUG)
                        Rlog.d(TAG, "processReceivedCBInUser, move cursor, i = " + i);
                } else {
                    break;
                }
            }  
            if(DEBUG)
                Rlog.d(TAG, "processReceivedCBInUser out of loop");     
            cursor.close();
            if(DEBUG)
                Rlog.d(TAG, "begin to delete the message in temp db");
            for (int i = 0; i < idList.size(); i++) {
                if(DEBUG)
                    Rlog.d(TAG, "processReceivedCBInUser deleting the message in temp db, _id = " + idList.get(i));
                resolver.delete(Uri.parse("content://usercb"), "_id = ?", new String[]{String.valueOf(idList.get(i))});
            }
        }
    
        public NormalUserReceivedImpl(Context context) {
            super(context);
            mContext = context;
        }
    
        public void handleUserSmsReceived(Intent intent) {
            if(DEBUG)
                Rlog.d(TAG, "get into handleUserSmsReceived");
                
            Object[] objPdus = (Object[]) intent.getSerializableExtra("pdus");
            byte[] pdus = (byte[])objPdus[0];
            //byte[] pdus = intent.getByteArrayExtra("pdus");
            String format = intent.getStringExtra("format");
            int simId = intent.getIntExtra("simId", -1);
        
            ContentValues values = new ContentValues();
            values.put("pdus", pdus);
            values.put("format", format);
            values.put("simId", simId);
        
            ContentResolver resolver = mContext.getContentResolver();
            SqliteWrapper.insert(mContext, resolver, Uri.parse("content://usersms"), values);
            if(DEBUG)
                Rlog.d(TAG, "end to handleUserSmsReceived");
        }
    
        public void handleUserMmsReceived(Intent intent) {
            if(DEBUG)
                Rlog.d(TAG, "get into handleUserMmsReceived");

            //add 
            String mimeType = intent.getType();
        
            int simId = intent.getIntExtra("simId", -1);
            int transactionId = intent.getIntExtra("transactionId", -1);
            int pduType = intent.getIntExtra("pduType", -1);
            byte[] header = intent.getByteArrayExtra("header");
            byte[] data = intent.getByteArrayExtra("data");
            String address = intent.getStringExtra("address");
            String service_center = intent.getStringExtra("service_center");
        
            ContentValues values = new ContentValues();
            values.put("simId", simId);
            values.put("transactionId", transactionId);
            values.put("pduType", pduType);
            values.put("header", header);
            values.put("data", data);
            values.put("address", address);
            values.put("service_center", service_center);
            values.put("mimeType",mimeType);
        
            ContentResolver resolver = mContext.getContentResolver();
            SqliteWrapper.insert(mContext, resolver, Uri.parse("content://usermms"), values);
            if(DEBUG)
                Rlog.d(TAG, "end of handleUserMmsReceived");
        }
    
        public void handleUserCBReceived(Intent intent) {
            if(DEBUG)
                Rlog.d(TAG, "get into handleUserCBReceived");
        
            int simId = intent.getIntExtra("simId", -1);
            Object[] objPdus = (Object[]) intent.getSerializableExtra("pdus");
            if (objPdus == null) {
                return;
            }
            byte[] pdus = (byte[])objPdus[0];
        
            ContentValues values = new ContentValues();
            values.put("action", intent.getAction());
            values.put("pdus", pdus);
            values.put("simId", simId);
        
            ContentResolver resolver = mContext.getContentResolver();
            SqliteWrapper.insert(mContext, resolver, Uri.parse("content://usercb"), values);
            if(DEBUG)
                Rlog.d(TAG, "end to handleUserCBReceived");
        }

        public void handleSwitchToOwner() {
            processReceivedSmsInUser();
            processReceivedMmsInUser();
            processReceivedCBInUser();
        }
    }

    private final class MsgBroadcastReceiver extends BroadcastReceiver {
        private long mBroadcastTimeNano;
        
        MsgBroadcastReceiver( ) {
            mBroadcastTimeNano = System.nanoTime();
        }
        
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intents.SMS_DELIVER_ACTION)) {
                // Now dispatch the notification only intent
                intent.setAction(Intents.SMS_RECEIVED_ACTION);
                intent.setComponent(null);
                dispatchIntent(intent, android.Manifest.permission.RECEIVE_SMS,
                        AppOpsManager.OP_RECEIVE_SMS, this);
            } else if (action.equals(Intents.WAP_PUSH_DELIVER_ACTION)) {
                // Now dispatch the notification only intent
                intent.setAction(Intents.WAP_PUSH_RECEIVED_ACTION);
                intent.setComponent(null);
                dispatchIntent(intent, android.Manifest.permission.RECEIVE_SMS,
                        AppOpsManager.OP_RECEIVE_SMS, this);
            
            } else {
                int durationMillis = (int) ((System.nanoTime() - mBroadcastTimeNano) / 1000000);
                if (durationMillis >= 5000) {
                    Rlog.e(TAG,"Slow ordered broadcast completion time: " + durationMillis + " ms");
                } else if (DEBUG) {
                    Rlog.d(TAG,"ordered broadcast completed in: " + durationMillis + " ms");
                }
            }
        }
    }

    void dispatchIntent(Intent intent, String permission, int appOp,
            BroadcastReceiver resultReceiver) {
        intent.addFlags(Intent.FLAG_RECEIVER_NO_ABORT);
        // MTK-START
        intent.putExtra("rTime", System.currentTimeMillis());
        //intent.putExtra(PhoneConstants.GEMINI_SIM_ID_KEY, mSimId);
        // MTK-END
        mContext.sendOrderedBroadcast(intent, permission, appOp, resultReceiver,
                null, Activity.RESULT_OK, null, null);
    }
 
    
}
