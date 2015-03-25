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

package com.mediatek.bluetooth.bip;

import android.os.Handler;
import android.os.Message;
import android.os.Process;

import android.os.IBinder;
import android.app.Service;
import android.content.Intent;
import android.net.Uri;


import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;
import java.io.File; //mtk71255
import com.mediatek.bluetooth.util.SystemUtils;  //20120216 add by mtk71255

public class BluetoothBipServer {

    private static final String TAG = "BluetoothBipServer";
    private static final int
        BIP_INITIATOR_ENABLE_SUCCESS= 0,
        BIP_INITIATOR_ENABLE_FAIL = 1, //Not used
        BIP_INITIATOR_DISABLE_SUCCESS = 2,
        BIP_INITIATOR_DISABLE_FAIL = 3,    //Not used
        BIP_INITIATOR_OBEX_AUTHREQ = 4,
        BIP_INITIATOR_CONNECT_SUCCESS = 5,
        BIP_INITIATOR_CONNECT_FAIL = 6,
        BIP_INITIATOR_GET_CAPABILITY_SUCCESS = 7,
        BIP_INITIATOR_GET_CAPABILITY_FAIL = 8,       // 1108, BIP service. Responder. Capa .... ..
        BIP_INITIATOR_IMAGE_PUSH_START = 9,          // 1109, BIP service. ... .. ..
        BIP_INITIATOR_PROGRESS = 10,                  // 1110
        BIP_INITIATOR_IMAGE_PUSH_SUCCESS = 11,        // 1111, BIP service. ... .. ..
        BIP_INITIATOR_IMAGE_PUSH_FAIL = 12,           // 1112, BIP service. ... .. ..
        BIP_INITIATOR_THUMBNAIL_REQ = 13,             // 1113
        BIP_INITIATOR_THUMBNAIL_PUSH_START = 14,      // 1114, BIP service. Thumbnail .. ..
        BIP_INITIATOR_THUMBNAIL_PUSH_SUCCESS = 15,    // 1115, BIP service. Thumbnail .. ..
        BIP_INITIATOR_THUMBNAIL_PUSH_FAIL = 16,       // 1116, BIP service. Thumbnail .. ..
        BIP_INITIATOR_DISCONNECT_SUCCESS = 17,        // 1117
        BIP_INITIATOR_DISCONNECT_FAIL = 18,           // 1118
        BIP_INITIATOR_CANCEL_SUCCESS = 19,            // 1119, BIP service .. .. ..
        BIP_INITIATOR_CANCEL_FAIL = 20,               // 1120, BIP service .. .. ..   //Not used

        BIP_RESPONDER_ENABLE_SUCCESS = 21,            // 1121
        BIP_RESPONDER_ENABLE_FAIL = 22,               // 1122, Not used
        BIP_RESPONDER_DISABLE_SUCCESS = 23,           // 1123
        BIP_RESPONDER_DISABLE_FAIL = 24,              // 1124, Not used
        BIP_RESPONDER_AUTH_REQ = 25,                  // 1125, BIP Responder Authorize Req
        BIP_RESPONDER_OBEX_AUTHREQ = 26,              // 1126, OBEX Authentication Req ... . ..
        BIP_RESPONDER_CONNECT_SUCCESS = 27,           // 1127, Not send to UI
        BIP_RESPONDER_CONNECT_FAIL = 28,              // 1128, Not send to UI
        BIP_RESPONDER_ACCESS_REQ = 29,                // 1129
        BIP_RESPONDER_GET_CAPABILITY_REQ = 30,        // 1130
        BIP_RESPONDER_CAPABILITY_RES_SUCCESS = 31,    // 1131
        BIP_RESPONDER_IMAGE_RECEIVE_START = 32,       // 1132
        BIP_RESPONDER_PROGRESS = 33,                  // 1133
        BIP_RESPONDER_IMAGE_RECEIVE_SUCCESS = 34,     // 1134
        BIP_RESPONDER_IMAGE_RECEIVE_FAIL = 35,        // 1135
        BIP_RESPONDER_THUMBNAIL_RECEIVE_START = 36,   // 1136
        BIP_RESPONDER_THUMBNAIL_RECEIVE_SUCCESS = 37, // 1137
        BIP_RESPONDER_THUMBNAIL_RECEIVE_FAIL = 38,    // 1138
        BIP_RESPONDER_DISCONNECT_SUCCESS = 39,        // 1139
        BIP_RESPONDER_DISCONNECT_FAIL = 40;           // 1140


 
    class BipListener extends Thread {
        public BipListener(){
            super( "BipListener" );
        }

        @Override
        public void run() {
            Process.setThreadPriority( Process.THREAD_PRIORITY_BACKGROUND );

            Xlog.i(TAG, "[BIP Server] listen thread run......");
            BluetoothBipServer.this.startListenNative();
            Xlog.i(TAG, "[BIP Server] listen thread stopped......");
        }

        public void startup(){

        //try{
        //    Thread.sleep(15000);
        //} catch (InterruptedException e) {
        //    e.printStackTrace();
        //}
            this.start();
        }

        public void shutdown(){
            BluetoothBipServer.this.stopListenNative();
        }
    }

    private int mNativeData = 0;
    private BipListener mListener;
    private Handler mCallback;
    private String mPath = null;  // 20120216 mtk71255
    private String mReceivedFileName = null;
    private native static void classInitNative();
    private native void initializeDataNative();
    private native void cleanupDataNative();

    private native boolean enableServiceNative();
    private native boolean startListenNative();
    private native boolean stopListenNative();
    private native void disableServiceNative();

    private native boolean bipiEnableNative();
    private native boolean bipiDisableNative();
    private native boolean bipiDisconnectNative(String BdAddr);
    private native boolean bipiGetCapabilityReqNative(String BdAddr);
    private native boolean bipiPushImageNative(String BdAddr, BipImage object);
    private native boolean bipiPushThumbnailNative(String BdAddr, BipImage object);
    //private native boolean bipiAuthRspNative(AuthInfo authReply);

    private native boolean biprEnableNative(String rootPath);
    private native boolean biprDisableNative();
    private native boolean biprAuthorizeRspNative(int authorize);
    private native boolean biprDisconnectNative();
    //private native boolean biprAccessRspNative(AuthRes replay);
    private native boolean biprAccessRspNative(int reply, int thumbnail, String objectPath);
    //private native boolean biprGetCapabilityRspNative(AuthRes replay, Capability capa);
    private native boolean biprGetCapabilityRspNative(int replay, Capability capa);
    //private native boolean biprAuthRspNative(AuthInfo authReply);

    private native boolean bipAuthRspNative(AuthInfo authReply, boolean isResponder);

// 20120627 added by mtk71255 for avoiding duplicated name file override
	private native void biprObjRename(String newName);  


//for perforamce tuning
    private static int mPreReceiveLength = 0;
    private static int mCurReceiveLength = 0;
    private static int mRObjLength = 0;
    private static int mPreSentLength = 0;
    private static int mCurSentLength = 0;
    private static int mSObjLength = 0;



    static
    {
/*
        try{
            Thread.sleep(15000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
*/

        System.loadLibrary("extbip_jni");
        classInitNative();
    }


    public BluetoothBipServer(final Handler callback)
    {
        mCallback = callback;
    }



    public boolean enable() {
        Xlog.i(TAG, "+enable");
  
        if ( mNativeData == 0 )
        {
            initializeDataNative();
            enableServiceNative(); 
            mListener = new BipListener();
            mListener.startup();
        }
        else
        { 
            Xlog.e(TAG, "mNativeData has been initialized");
        } 
        Xlog.i(TAG, "-enable");
        return true;
    }


    public void disable(){
        Xlog.i(TAG, "+disable");

        
        if (mNativeData != 0 )
        {
            try {
                mListener.shutdown();
                mListener.join();
                mListener = null;
            } catch (InterruptedException ex) {
                Xlog.w(TAG, "BipServer mListener close error");
            }
            disableServiceNative();    // release communication channel with BT-Task
            cleanupDataNative();
        }
        else
        {
            Xlog.e(TAG, "mNativeData has been cleaned");
        }
        Xlog.i(TAG, "-disable");
    }


/*----------------------------------------
Native call encapulation to JNI
----------------------------------------*/

    public boolean bipiEnable() {
        Xlog.i(TAG, "+bipiEnable");
        bipiEnableNative();
        Xlog.i(TAG, "-bipiEnable");
        return true;
    }

    public boolean bipiDisable() {
        Xlog.i(TAG, "+bipiDisable");
        bipiDisableNative();
        Xlog.i(TAG, "-bipiDisable");
        return true;
    }

    public boolean bipiGetCapabilityReq(String btaddr) {
        Xlog.i(TAG, "+bipiGetCapabilityReq");
        bipiGetCapabilityReqNative(btaddr);
        Xlog.i(TAG, "-bipiGetCapabilityReq");
        return true;
    }

    public boolean bipiDisconnect(String btaddr){
        Xlog.i(TAG, "+bipiDisconnect");
		if(btaddr != null)
        	bipiDisconnectNative(btaddr);
		else			
        	Xlog.w(TAG, "-Empty btaddr");
        Xlog.i(TAG, "-bipiDisconnect");
        return true;
    }

    public boolean bipiPushImage(String btaddr, BipImage object){
        Xlog.i(TAG, "+bipiPushImage");
        
        mSObjLength = object.ObjectSize;
        mPreSentLength = 0;
        mCurSentLength = 0;

        bipiPushImageNative(btaddr, object);
        Xlog.i(TAG, "-bipiPushImage");
        return true;
    }

    public boolean bipiPushThumbnail(String btaddr, BipImage object){
        Xlog.i(TAG, "+bipiPushThumbnail");
        bipiPushThumbnailNative(btaddr, object);
        Xlog.i(TAG, "-bipiPushThumbnail");
        return true;
    }



    public boolean bipAuthRsp(AuthInfo authInfo, boolean isResponder){
        Xlog.i(TAG, "+bipAuthRspNative");
        bipAuthRspNative(authInfo, isResponder);
        Xlog.i(TAG, "-bipAuthRspNative");
        return true;
    }



    public boolean biprEnable(String rootPath){
        Xlog.i(TAG, "+biprEnable");
        biprEnableNative(rootPath);
        Xlog.i(TAG, "-biprEnable");
        return true;
    }

    public boolean biprDisable(){
        Xlog.i(TAG, "+biprDisable");
        biprDisableNative();
        Xlog.i(TAG, "-biprDisable");
        return true;
    }

    public boolean biprDisconnect()
    {
        Xlog.i(TAG, "+biprDisconnect");
        biprDisconnectNative();
        Xlog.i(TAG, "-biprDisconnect");
        return true;
    }

    public boolean biprAuthorizeRsp(int authorize)
    {
        Xlog.i(TAG, "+biprAuthorizeRsp");
        biprAuthorizeRspNative(authorize);
        Xlog.i(TAG, "-biprAuthorizeRsp");
        return true;
    }

    public boolean biprGetCapabilityRsp(int reply, Capability capability)
    {
        Xlog.i(TAG, "+biprGetCapabilityRsp");
        biprGetCapabilityRspNative(reply, capability);
        Xlog.i(TAG, "-biprGetCapabilityRsp");
        return true;
    }

    public boolean biprAccessRsp(int reply, int thumbnail, String objectPath)
    {
        Xlog.i(TAG, "+biprAccessRsp");
	//20120627 mtk71255 added for arima rename bip
	String tempPath = mPath + "/" +mReceivedFileName;
	Xlog.i(TAG, "tempPath" + tempPath);
	File file = SystemUtils.createNewFileForSaving( tempPath );
	if( file != null){
		//this.oppsTask.setData(file.getAbsolutePath());
		mReceivedFileName = file.getName();
		biprObjRename(mReceivedFileName);
	}
	Xlog.i(TAG, "mReceivedFileName:" + mReceivedFileName);

        biprAccessRspNative(reply, thumbnail, objectPath);
        Xlog.i(TAG, "-biprAccessRsp");
        return true;
    }




/*----------------------------------------
CALLBCAK form JNI
----------------------------------------*/
    private void onCallback(int event, int arg1, int arg2, String[] parameters){
    //should there are two handlers? One for server and one for client
        switch(event)
        {
            case BIP_INITIATOR_ENABLE_SUCCESS:
            {
                Xlog.i(TAG, "BIP_INITIATOR_ENABLE_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_ENABLE, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_INITIATOR_ENABLE_FAIL:
            {
                Xlog.i(TAG, "BIP_INITIATOR_ENABLE_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_ENABLE, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_INITIATOR_DISABLE_SUCCESS:
            {
                Xlog.i(TAG, "BIP_INITIATOR_DISABLE_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_DISABLE, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_INITIATOR_DISABLE_FAIL:
            {
                Xlog.i(TAG, "BIP_INITIATOR_DISABLE_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_DISABLE, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_INITIATOR_OBEX_AUTHREQ: //pop obex authentication dialog
            {
                Xlog.i(TAG, "BIP_INITIATOR_OBEX_AUTHREQ");
                if( parameters[0].equals("1") )
                {
                    Xlog.i(TAG, "UserID required");
                }
                mCallback.obtainMessage( BipService.MSG_ON_BIPI_OBEX_AUTHREQ, 0, Integer.parseInt(parameters[0])).sendToTarget();
            }
            break;
            case BIP_INITIATOR_CONNECT_SUCCESS: //isn't used
            {
                Xlog.i(TAG, "BIP_INITIATOR_CONNECT_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_CONNECT, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_INITIATOR_CONNECT_FAIL:    //isn't used
            {
                Xlog.i(TAG, "BIP_INITIATOR_CONNECT_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_CONNECT, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_INITIATOR_GET_CAPABILITY_SUCCESS:
            {
                Xlog.i(TAG, "BIP_INITIATOR_GET_CAPABILITY_SUCCESS");

                Capability capaObj = new Capability(Integer.parseInt(parameters[8]));

                capaObj.PreferFormat.Version = parameters[0];
                capaObj.PreferFormat.Encoding = parameters[1];
                capaObj.PreferFormat.Width = Integer.parseInt(parameters[2]);
                capaObj.PreferFormat.Height = Integer.parseInt(parameters[3]);
                capaObj.PreferFormat.Width2 = Integer.parseInt(parameters[4]);
                capaObj.PreferFormat.Height2 = Integer.parseInt(parameters[5]);
                capaObj.PreferFormat.Size = Integer.parseInt(parameters[6]);
                capaObj.PreferFormat.Transform = Integer.parseInt(parameters[7]);
/*
                switch (Integer.parseInt(parameters[7])) {
                    case 0:
                        capaObj.PreferFormat.Transform =  Transformation.BIP_TRANS_NONE;
                    break;
                    case 1:
                        capaObj.PreferFormat.Transform =  Transformation.BIP_TRANS_STRETCH;
                    break;
                    case 2:
                        capaObj.PreferFormat.Transform =  Transformation.BIP_TRANS_FILL;
                    break;
                    case 3:
                        capaObj.PreferFormat.Transform =  Transformation.BIP_TRANS_CROP;
                    break;
                }
*/
                capaObj.NumImageFormats = Integer.parseInt(parameters[8]);
/*
                Xlog.i(TAG, "Version: " + capaObj.PreferFormat.Version);
                Xlog.i(TAG, "Encoding: " + capaObj.PreferFormat.Encoding);
                Xlog.i(TAG, "Width: " + capaObj.PreferFormat.Width);
                Xlog.i(TAG, "Height: " + capaObj.PreferFormat.Height);
                Xlog.i(TAG, "Width2: " + capaObj.PreferFormat.Width2);
                Xlog.i(TAG, "Height2: " + capaObj.PreferFormat.Height2);
                Xlog.i(TAG, "Size: " + capaObj.PreferFormat.Size);
                Xlog.i(TAG, "Transform: " + capaObj.PreferFormat.Transform);
                Xlog.i(TAG, "NumFormats: " + capaObj.NumImageFormats );
*/
                for(int i = 0; i < capaObj.NumImageFormats; i++)
                {
                    capaObj.ImageFormats[i].Encoding = parameters[9 + i*6] ; 
                    capaObj.ImageFormats[i].Width = Integer.parseInt(parameters[9 + i*6 + 1]); 
                    capaObj.ImageFormats[i].Height = Integer.parseInt(parameters[9 + i*6 + 2]); 
                    capaObj.ImageFormats[i].Width2 = Integer.parseInt(parameters[9 + i*6 + 3]); 
                    capaObj.ImageFormats[i].Height2 = Integer.parseInt(parameters[9 + i*6 + 4]); 
                    capaObj.ImageFormats[i].Size = Integer.parseInt(parameters[9 + i*6 + 5]); 
/*
                    Xlog.i(TAG, "SupEncoding: " + capaObj.ImageFormats[i].Encoding);
                    Xlog.i(TAG, "SupWidth: " + capaObj.ImageFormats[i].Width);
                    Xlog.i(TAG, "SupHeight: " + capaObj.ImageFormats[i].Height);
                    Xlog.i(TAG, "SupWidth2: " + capaObj.ImageFormats[i].Width2);
                    Xlog.i(TAG, "SupHeight2: " + capaObj.ImageFormats[i].Height2);
                    Xlog.i(TAG, "SupSize: " + capaObj.ImageFormats[i].Size);
*/
                }

                int cnfCode = 0;
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_GET_CAPABILITY, cnfCode, 0, capaObj).sendToTarget();

            }
            break;
            case BIP_INITIATOR_GET_CAPABILITY_FAIL:
            {
                Xlog.i(TAG, "BIP_INITIATOR_GET_CAPABILITY_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_GET_CAPABILITY, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_INITIATOR_IMAGE_PUSH_START:
            {
                Xlog.i(TAG, "BIP_INITIATOR_IMAGE_PUSH_START");
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_IMAGE_PUSH_START, 0, 0).sendToTarget();
            }
            break;
            case BIP_INITIATOR_PROGRESS:
            {
                Xlog.i(TAG, "BIP_INITIATOR_PROGRESS");
                Xlog.i(TAG, "Sending Length: " + parameters[0]);
                mCurSentLength = Integer.parseInt(parameters[0]);
                if ( (mSObjLength -  mCurSentLength) <= mSObjLength/100 ) {
                    mCallback.obtainMessage(BipService.MSG_ON_BIPI_PROGRESS, 0, mCurSentLength).sendToTarget();
                }
                else if ( (mCurSentLength - mPreSentLength) > mSObjLength/40  ) {
                    mPreSentLength = mCurSentLength;
                    mCallback.obtainMessage(BipService.MSG_ON_BIPI_PROGRESS, 0, mCurSentLength).sendToTarget();
                }
                //mCallback.obtainMessage(BipService.MSG_ON_BIPI_PROGRESS, 0, Integer.parseInt(parameters[0])).sendToTarget();
            }
            break;
            case BIP_INITIATOR_IMAGE_PUSH_SUCCESS:
            {
                Xlog.i(TAG, "BIP_INITIATOR_IMAGE_PUSH_SUCCESS");

                Xlog.i(TAG, "Image Handle: " + parameters[0]);

                int cnfCode = 0;
                int handle = -1;
                if (parameters[0].matches("[0-9]"))
                    handle = Integer.parseInt(parameters[0]);
                                       
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_PUSH, cnfCode, handle).sendToTarget();
            }
            break;
            case BIP_INITIATOR_IMAGE_PUSH_FAIL:
            {
                Xlog.i(TAG, "BIP_INITIATOR_IMAGE_PUSH_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_PUSH, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_INITIATOR_THUMBNAIL_REQ:
            {
                Xlog.i(TAG, "BIP_INITIATOR_THUMBNAIL_REQ");
                Xlog.i(TAG, "Image Handle: " + parameters[0]);
                //parse the handle to string
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_THUMBNAIL_REQ, 0, Integer.parseInt(parameters[0])).sendToTarget();
            }
            break;
            case BIP_INITIATOR_THUMBNAIL_PUSH_START:
            {
                Xlog.i(TAG, "BIP_INITIATOR_THUMBNAIL_PUSH_START");
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_THUMBNAIL_PUSH_START, 0, 0).sendToTarget();
            }
            break;
            case BIP_INITIATOR_THUMBNAIL_PUSH_SUCCESS:
            {
                Xlog.i(TAG, "BIP_INITIATOR_THUMBNAIL_PUSH_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_THUMBNAIL_PUSH, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_INITIATOR_THUMBNAIL_PUSH_FAIL:
            {
                Xlog.i(TAG, "BIP_INITIATOR_THUMBNAIL_PUSH_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_THUMBNAIL_PUSH, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_INITIATOR_DISCONNECT_SUCCESS:
            {
                Xlog.i(TAG, "BIP_INITIATOR_DISCONNECT_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_DISCONNECT, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_INITIATOR_DISCONNECT_FAIL:    //isn't used
            {
                Xlog.i(TAG, "BIP_INITIATOR_DISCONNECT_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_DISCONNECT, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_INITIATOR_CANCEL_SUCCESS:
            {
                Xlog.i(TAG, "BIP_INITIATOR_CANCEL_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_CANCEL, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_INITIATOR_CANCEL_FAIL:
            {
                Xlog.i(TAG, "BIP_INITIATOR_CANCEL_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BipService.MSG_ON_BIPI_CANCEL, cnfCode, 0).sendToTarget();
            }
            break;








            case BIP_RESPONDER_ENABLE_SUCCESS:
            {
                Xlog.i(TAG, "BIP_RESPONDER_ENABLE_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_ENABLE, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_RESPONDER_ENABLE_FAIL:
            {
                Xlog.i(TAG, "BIP_RESPONDER_ENABLE_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_ENABLE, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_RESPONDER_DISABLE_SUCCESS:
            {
                Xlog.i(TAG, "BIP_RESPONDER_DISABLE_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_DISABLE, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_RESPONDER_DISABLE_FAIL:
            {
                Xlog.i(TAG, "BIP_RESPONDER_DISABLE_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_DISABLE, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_RESPONDER_AUTH_REQ:
            {
                Xlog.i(TAG, "BIP_RESPONDER_AUTH_REQ");
                Xlog.i(TAG, "RemoteDevName: " + parameters[0]);
                Xlog.i(TAG, "RemoteBtAddr: " + parameters[1]);
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_AUTH_REQ, 0, 0, parameters).sendToTarget();
            }
            break;
            case BIP_RESPONDER_OBEX_AUTHREQ:
            {
                Xlog.i(TAG, "BIP_RESPONDER_OBEX_AUTHREQ");
                if( parameters[0].equals("1") )
                {
                    Xlog.i(TAG, "UserID required");
                }
                mCallback.obtainMessage( BipService.MSG_ON_BIPR_OBEX_AUTHREQ, 0, Integer.parseInt(parameters[0])).sendToTarget();
            }
            break;
            case BIP_RESPONDER_CONNECT_SUCCESS: //isn't used
            {
                Xlog.i(TAG, "BIP_RESPONDER_CONNECT_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_CONNECT, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_RESPONDER_CONNECT_FAIL: //isn't used
            {
                Xlog.i(TAG, "BIP_RESPONDER_CONNECT_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_CONNECT, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_RESPONDER_GET_CAPABILITY_REQ:
            {  
                Xlog.i(TAG, "BIP_RESPONDER_GET_CAPABILITY_REQ");
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_GET_CAPABILITY_REQ, 0, 0).sendToTarget();
            }
            break;
            case BIP_RESPONDER_CAPABILITY_RES_SUCCESS:
            {  
                Xlog.i(TAG, "BIP_RESPONDER_CAPABILITY_RES_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_CAPABILITY_RES, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_RESPONDER_ACCESS_REQ:
            {				
		mReceivedFileName = parameters[0];
                Xlog.i(TAG, "BIP_RESPONDER_ACCESS_REQ");
                Xlog.i(TAG, "fileName: " + parameters[0]);
                Xlog.i(TAG, "fileSize: " + parameters[1]);
                mPreReceiveLength = 0;
                mCurReceiveLength = 0;
                mRObjLength = Integer.parseInt(parameters[1]);               
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_ACCESS_REQ, 0, mRObjLength, parameters[0]).sendToTarget();
            }
            break;
            case BIP_RESPONDER_IMAGE_RECEIVE_START:
            {
                Xlog.i(TAG, "BIP_RESPONDER_IMAGE_RECEIVE_START");
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_IMAGE_RECEIVE_START, 0, 0).sendToTarget();
            }
            break;
            case BIP_RESPONDER_PROGRESS:
            {
                Xlog.i(TAG, "BIP_RESPONDER_PROGRESS");
                Xlog.i(TAG, "Object Length: " + parameters[1]);
                int objLength = Integer.parseInt(parameters[1]);

                if ( mRObjLength == 0 && objLength != 0 ) {
                    mRObjLength = objLength;
                }

                Xlog.i(TAG, "Receiving Length: " + parameters[0]);
                mCurReceiveLength = Integer.parseInt(parameters[0]);
                if ( (mRObjLength - mCurReceiveLength) <= mRObjLength/100 ) {
                    mCallback.obtainMessage(BipService.MSG_ON_BIPR_PROGRESS, mRObjLength, mCurReceiveLength).sendToTarget();
                }
                else if ( (mCurReceiveLength - mPreReceiveLength) > mRObjLength/40  ) {
                    mPreReceiveLength = mCurReceiveLength;
                    mCallback.obtainMessage(BipService.MSG_ON_BIPR_PROGRESS, mRObjLength, mCurReceiveLength).sendToTarget();
                }
            }
            break;
            case BIP_RESPONDER_IMAGE_RECEIVE_SUCCESS:
            {
                Xlog.i(TAG, "BIP_RESPONDER_IMAGE_RECEIVE_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_RECEIVE, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_RESPONDER_IMAGE_RECEIVE_FAIL:
            {
                Xlog.i(TAG, "BIP_RESPONDER_IMAGE_RECEIVE_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_RECEIVE, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_RESPONDER_THUMBNAIL_RECEIVE_START:
            {
                Xlog.i(TAG, "BIP_RESPONDER_THUMBNAIL_RECEIVE_START");
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_THUMBNAIL_RECEIVE_START, 0, 0).sendToTarget();
            }
            break;
            case BIP_RESPONDER_THUMBNAIL_RECEIVE_SUCCESS:
            {
                Xlog.i(TAG, "BIP_RESPONDER_THUMBNAIL_RECEIVE_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_THUMBNAIL_RECEIVE, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_RESPONDER_THUMBNAIL_RECEIVE_FAIL:
            {
                Xlog.i(TAG, "BIP_RESPONDER_THUMBNAIL_RECEIVE_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_THUMBNAIL_RECEIVE, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_RESPONDER_DISCONNECT_SUCCESS:
            {
                Xlog.i(TAG, "BIP_RESPONDER_DISCONNECT_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_DISCONNECT, cnfCode, 0).sendToTarget();
            }
            break;
            case BIP_RESPONDER_DISCONNECT_FAIL:
            {
                Xlog.i(TAG, "BIP_RESPONDER_DISCONNECT_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BipService.MSG_ON_BIPR_DISCONNECT, cnfCode, 0).sendToTarget();
            }
            break;
            default:
                Xlog.e(TAG, "UN-KNOWN EVENT");
        }//switch end
    } //onCallback end
	// 20120627 added by mtk71255 for avoiding duplicated name file override
    public void setRecvPath(String path){

		mPath = path;
	}
}//BluetoothBipServer CLASS end


/*----------------------------------------
AUX CLASS in BipImage.java 
----------------------------------------*/
