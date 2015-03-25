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

package com.mediatek.bluetooth.map;
import android.telephony.SmsMessage;
import com.android.internal.telephony.gsm.SmsMessage.DeliverPdu;
import com.android.internal.telephony.GsmAlphabet.TextEncodingDetails;
import android.telephony.TelephonyManager;
import android.telephony.PhoneNumberUtils;
import android.util.Log;
import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.File;

import android.os.RemoteException;
import android.os.ServiceManager;
import android.content.Context;
import com.mediatek.common.telephony.ITelephonyEx;

import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.EncodeException;
import com.android.internal.telephony.cdma.sms.SmsEnvelope;
import com.android.internal.telephony.cdma.sms.BearerData;
import com.android.internal.telephony.cdma.sms.CdmaSmsAddress;
import com.android.internal.telephony.cdma.sms.UserData;

import com.mediatek.bluetooth.map.util.NetworkUtil;
import com.mediatek.xlog.Xlog;

public class SmsMessageEntity{	
	private final static String TAG = "SmsMessageEntity";

	private static SmsMessageEntity mEntity;

	private SmsMessageEntity(){
	}

	public static SmsMessageEntity getDefault (){
		if (mEntity == null) {
			mEntity = new SmsMessageEntity();
		}
		return mEntity;
	}
	
	
	public  byte[] getSubmitPdu(String orignator, String recipient, String message, int slotId){
		SmsMessage.SubmitPdu pdu;
		ByteArrayOutputStream bo;
		int length = 0;
		bo = new ByteArrayOutputStream(
                SmsMessage.MAX_USER_DATA_BYTES + 40);
		pdu = SmsMessage.getSubmitPdu(orignator,
									recipient, message, false);	
		setScaAddress(bo, slotId);
		bo.write(pdu.encodedMessage, 0, pdu.encodedMessage.length);	
		return bo.toByteArray();
	}
	public  byte[] getDeliverPdu(String orignator, String recipient, String message,int slotId){
		int activePhone;
		int length = 0;
		
		if (NetworkUtil.isGeminiSupport()){
			activePhone = NetworkUtil.getGeminiNetworkType(slotId);
		} else {
			activePhone = NetworkUtil.getNetworkType();
		}

        if (TelephonyManager.PHONE_TYPE_CDMA == activePhone) {
			return getCdmaDeliverPdu(orignator, message);
        } else if (TelephonyManager.PHONE_TYPE_GSM == activePhone){
			return getGsmDeliverPdu(orignator, message, slotId);
		} else {
			Log.d(TAG, "unkown net type");
			return null;
		}
	}
	private  byte[] getCdmaDeliverPdu(String orignator, String message){
		int length = 0;
		CdmaSmsAddress destAddr = CdmaSmsAddress.parse(orignator);
		if (destAddr == null) return null;		

		ByteArrayOutputStream baos = new ByteArrayOutputStream(100);
		DataOutputStream dos = new DataOutputStream(baos);
		
		BearerData bearerData = new BearerData();
		bearerData.messageType = BearerData.MESSAGE_TYPE_DELIVER;
		
		//bearerData.messageId = getNextMessageId();
		
		bearerData.deliveryAckReq = false;
		bearerData.userAckReq = false;
		bearerData.readAckReq = false;
		bearerData.reportReq = false;

		UserData uData = new UserData();
        uData.payloadStr = message;
        uData.userDataHeader = null;
		bearerData.userData = uData;

		byte[] encodedBearerData = BearerData.encode(bearerData);

		try{

		dos.writeInt(SmsEnvelope.MESSAGE_TYPE_POINT_TO_POINT);//messageType
   //     dos.writeInt(0); //teleService
   		int tele = (SmsEnvelope.TELESERVICE_WAP << 16) + (0x2 << 8);
   		dos.writeInt(tele);  

		//for point to point, service Category is optional
	//	int serviceCategory = (0x2 << 8);
    //    dos.writeInt(serviceCategory); //serviceCategory

		dos.write(destAddr.digitMode);
		dos.write(destAddr.numberMode);
		dos.write(destAddr.ton); // number_type
		dos.write(destAddr.numberPlan);
		dos.write(destAddr.numberOfDigits);
		dos.write(destAddr.origBytes, 0, destAddr.origBytes.length); // digits
		dos.writeInt(0); //bearerReply

		// CauseCode values:
		dos.write(0);	//replySeqNo;
		dos.write(0);	//errorClass;
		dos.write(0);	//causeCode ;

		//encoded BearerData:		
		if (encodedBearerData != null) {
			dos.write(encodedBearerData.length);
			dos.write(encodedBearerData, 0, encodedBearerData.length);
		}
		length = baos.toByteArray().length;
		dos.close();
		baos.close();
		} catch (IOException e) {
			e.printStackTrace();
			return null;
		}
 		return baos.toByteArray();

          		
	}
	private byte[] getGsmDeliverPdu(String orignator, String message, int slotId){
		ByteArrayOutputStream bo = new ByteArrayOutputStream(
                SmsMessage.MAX_USER_DATA_BYTES + 40);
		byte[] daBytes;
		int encoding = SmsMessage.ENCODING_UNKNOWN;
        int language = -1;
		int length = 0;

		log("message body is "+message);

		TextEncodingDetails details = com.android.internal.telephony.gsm.SmsMessage.calculateLength(message, false);
        encoding = details.codeUnitSize;
		language = details.shiftLangId;

		DeliverPdu pdu = com.android.internal.telephony.gsm.SmsMessage.getDeliverPduWithLang
			(null, orignator, message,null, 0, encoding, language);
		if(pdu == null) {
			log("fail to get deliver pdu");
			return null;
		}
		setScaAddress(bo, slotId);
		bo.write(pdu.encodedMessage, 0, pdu.encodedMessage.length);	
		
		return bo.toByteArray();
	}

	private void setScaAddress(ByteArrayOutputStream out, int slotId){ 
		byte[] scaAddr = null;
		String sca = null;
		ITelephonyEx iTelEx = null;
		
		iTelEx = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
		
		if(iTelEx != null)
		{
			try{
				sca = iTelEx.getScAddressGemini(slotId);
			} catch (RemoteException ex) {
				Log.e(TAG, "ITelephony api exception:" + ex);
			}
		}

		if (sca != null){
			scaAddr = PhoneNumberUtils.networkPortionToCalledPartyBCDWithLength(sca);
		} 

		if (scaAddr != null){
			out.write(scaAddr, 0, scaAddr.length);	
		}else {
			out.write(0x00); 
		}	
	}
	private void log(String info){
		if (null != info){
			Xlog.v(TAG, info);
		}
	}
}

