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

package com.mediatek.bluetooth.map.mime;

import com.google.android.mms.pdu.EncodedStringValue;
import com.google.android.mms.pdu.PduContentTypes;
import com.google.android.mms.pdu.*;
import com.google.android.mms.ContentType;
import com.google.android.mms.InvalidHeaderValueException;
import android.content.Context;
import android.content.ContentResolver;
import android.util.Log;
import com.google.android.mms.pdu.Base64;
import com.google.android.mms.pdu.QuotedPrintable;

import java.io.FileInputStream;
import java.io.InputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;

import com.mediatek.bluetooth.map.MAP;
import com.mediatek.bluetooth.map.Address;
import com.mediatek.bluetooth.map.mime.MimeContent;
import com.mediatek.xlog.Xlog;
public class MmsMime extends MimeBase {
	private static final String TAG = "MmsMime";

	private static final long    DEFAULT_EXPIRY_TIME     = 7 * 24 * 60 * 60;

	private String mMmsCenter;
	public MmsMime(ContentResolver resolver, GenericPdu pdu, String mmsCenter){
		super(resolver, MSG_TYPE_MMS);
		loadMime(pdu);
		mMmsCenter = mmsCenter;
	}
	public MmsMime(ContentResolver resolver, GenericPdu pdu){
		super(resolver, MSG_TYPE_MMS);
		loadMime(pdu);
	}
	public MmsMime(ContentResolver resolver){
		super(resolver, MSG_TYPE_MMS);
	}
	private void loadMime(GenericPdu pdu){
		if (pdu == null) {
			return;
		}
		int type = pdu.getMessageType();	
		log("type:"+type);
		 switch (type) {
            case PduHeaders.MESSAGE_TYPE_SEND_REQ:
                loadFromSendReq((SendReq)pdu);
                break;
            case PduHeaders.MESSAGE_TYPE_SEND_CONF:
                loadFromSendConf((SendConf)pdu);
                break;
            case PduHeaders.MESSAGE_TYPE_NOTIFICATION_IND:
                loadFromNotificationInd((NotificationInd)pdu);
                break;
            case PduHeaders.MESSAGE_TYPE_NOTIFYRESP_IND:
                loadFromNotifyRespInd((NotifyRespInd)pdu);
                break;
            case PduHeaders.MESSAGE_TYPE_RETRIEVE_CONF:
                loadFromRetrieveConf((RetrieveConf)pdu);                  
                break;
            case PduHeaders.MESSAGE_TYPE_DELIVERY_IND:
                loadFromDeliveryInd ((DeliveryInd)pdu);
                break;
            case PduHeaders.MESSAGE_TYPE_ACKNOWLEDGE_IND:
                loadFromAcknowledgeInd ((AcknowledgeInd)pdu);
                break;
            case PduHeaders.MESSAGE_TYPE_READ_ORIG_IND:
                loadFromReadOrigInd((ReadOrigInd)pdu);
                break;
            case PduHeaders.MESSAGE_TYPE_READ_REC_IND:
                loadFromReadRecInd((ReadRecInd)pdu);
                break;
            default:
				log("unsupported type:"+type);
		 	}

		
	}	

	private void loadFromGenericPdu(GenericPdu pdu){
		EncodedStringValue value = pdu.getFrom();
		if (value != null) {
			mHeaders.mFrom = getAddress(value);
		}
	}
	
	private void loadFromNotificationInd(NotificationInd pdu) {
		loadFromGenericPdu(pdu);
		EncodedStringValue value = pdu.getSubject();
		if (value != null) {
			mHeaders.mSubject = value.getString();
		}
	}
	private void loadFromNotifyRespInd(NotifyRespInd pdu) {
		loadFromGenericPdu(pdu);
	}
	private void loadFromDeliveryInd(DeliveryInd pdu) {
		loadFromGenericPdu(pdu);
		mHeaders.mTimeStamp = pdu.getDate();		
		mHeaders.mMsgId = getStringValue(pdu.getMessageId());
		mHeaders.mTo = getAddress(pdu.getTo());		
	}
	private void loadFromAcknowledgeInd(AcknowledgeInd pdu) {
		loadFromGenericPdu(pdu);
	}
	private void loadFromReadOrigInd(ReadOrigInd pdu) {
		loadFromGenericPdu(pdu);
		mHeaders.mTimeStamp = pdu.getDate();		
		mHeaders.mMsgId = getStringValue(pdu.getMessageId());
		mHeaders.mTo = getAddress(pdu.getTo());		
	}
	private void loadFromReadRecInd(ReadRecInd pdu) {
		loadFromGenericPdu(pdu);
		mHeaders.mTimeStamp = pdu.getDate();		
		mHeaders.mMsgId = getStringValue(pdu.getMessageId());
		mHeaders.mTo = getAddress(pdu.getTo());		
	}
	private void loadFromSendConf(SendConf pdu){
		loadFromGenericPdu(pdu);
		mHeaders.mMsgId = getStringValue(pdu.getMessageId());
	}
	private void loadFromMultimediaMessagePdu(MultimediaMessagePdu pdu) {
		int index;
		PduPart part = null;
		ArrayList<MimeAttachment> attaches = new ArrayList<MimeAttachment>();
		byte[] cache;
		EncodedStringValue value;
		String disposition = null;
		
		loadFromGenericPdu(pdu);
		value = pdu.getSubject();
		if (value != null) {
			mHeaders.mSubject = value.getString();
		}
		mHeaders.mTo = getAddress(pdu.getTo());		
		mHeaders.mTimeStamp = pdu.getDate(); 

		PduBody body = pdu.getBody();
		for(index = 0; (body != null) && (index < body.getPartsNum()); index++){
			part = body.getPart(index);
			MimeAttachment attach = new MimeAttachment();
			//content id
			cache = part.getContentId();
			if (cache != null) {
				attach.mContentId = new String(cache);
			}
			
			attach.mContentUri = part.getDataUri();
		//	attach.mContent = part.getData();
			attach.mContentBytes = part.getData();
		//	attach.mEncoding;

			//filename
			cache = part.getFilename();
			if (cache != null) {
				attach.mFileName = new String(cache);
			}
			cache = part.getName();
			if (cache != null) {
				attach.mName = new String(cache);
			}
			if (part.getContentDisposition() != null) {
				disposition = new String(part.getContentDisposition());
			}
			attach.mLocation = disposition;
			cache = part.getContentLocation();
			if (cache != null) {
				attach.mContentLocation = new String(cache);
			}
		//	attach.mMessageKey;

			//content type
			cache = part.getContentType();
			if (cache != null) {
				attach.mMimeType = new String(cache);
			}
			
			if (attach.mContentBytes != null) {
				attach.mSize = attach.mContentBytes.length;
			} else if (attach.mContentUri != null) {
				try{
					InputStream input = mContentResolver.openInputStream(attach.mContentUri);
            		if (input instanceof FileInputStream) {
						attach.mSize = ((FileInputStream)input).available();
					}
                                        input.close();
				} catch(FileNotFoundException e) {
					log(e.toString());
					continue;
					
				} catch(IOException e){
					log(e.toString());
					continue;
				}
				
			} 
			
			attaches.add(attach);
		}
		mAttachment = attaches.toArray(new MimeBase.MimeAttachment[attaches.size()]);
		
	}

	private void loadFromSendReq(SendReq pdu){
		loadFromMultimediaMessagePdu(pdu);
		mHeaders.mBcc = getAddress(pdu.getBcc());
		mHeaders.mCc = getAddress(pdu.getCc());
	}
	
	private void loadFromRetrieveConf(RetrieveConf pdu){
		loadFromMultimediaMessagePdu(pdu);
		mHeaders.mCc = getAddress(pdu.getCc());
		mHeaders.mMsgId = getStringValue(pdu.getMessageId());

		//todo content type
	}

	

	private String getAddress (EncodedStringValue[] values){
		StringBuilder addresses = new StringBuilder();
		String address;
		if (values == null) {
			return null;
		}
		for (EncodedStringValue value : values){
			if(addresses.length() > 0){
				addresses.append(";");
			}
			address = value.getString();			
			addresses.append(address);
		}
		log("get address([]):"+addresses.toString());
		return addresses.toString();
	}

	private String getAddress (EncodedStringValue values){
		if (values == null) {
			return null;
		}
		String address = values.getString();
		if (!Address.isValidAddress(address)) {
			if (mMmsCenter != null) {
				address += "@"+mMmsCenter;
			} else {
				log("MMSCenter is null");
			}
		}
		log("get address:"+address);
		return address;
	}
	
	private int getIntValue(byte[] value) {
		if(value == null){
			return 0;
		}
		return Integer.parseInt(new String(value));
	}

	private String getStringValue(byte[] value){
		if(value == null){
			return null;
		}
		return new String(value);
	}

	private EncodedStringValue[] reverseAddress(String value){
		ArrayList<EncodedStringValue> address = new ArrayList<EncodedStringValue>();
		if (value != null){
			String[] elements = value.split(";");
			for (String element : elements) {
				address.add(new EncodedStringValue(element));
			}
		}
		return address.toArray(new EncodedStringValue[address.size()]);
	}
	public MultimediaMessagePdu generatePdu(int type){
		MultimediaMessagePdu pdu = null;
		switch (type) {
            case PduHeaders.MESSAGE_TYPE_SEND_REQ:
               	pdu = generateSendReqPdu();
                break;            
            case PduHeaders.MESSAGE_TYPE_RETRIEVE_CONF:
				pdu = generateRetrieveConfPdu();
				break;
			default:
		}
		return pdu;
	}
	public RetrieveConf generateRetrieveConfPdu(){
		log("generateRetrieveConfPdu()");
		RetrieveConf pdu;
		try {
			pdu = new RetrieveConf();
		

			if (mHeaders.mMiltipartType != null) {
				pdu.setContentType(mHeaders.mMiltipartType.getBytes());
			} else {
				pdu.setContentType(MimeContent.TEXT_PLAIN.getBytes()); 
			}

			pdu.setDeliveryReport(PduHeaders.VALUE_NO);
			pdu.setReadReport(PduHeaders.VALUE_NO);
			if (mHeaders.mFrom != null) {
				pdu.setFrom(new EncodedStringValue(mHeaders.mFrom));
			} 
			if (mHeaders.mTo != null) {
				EncodedStringValue[] tos = reverseAddress(mHeaders.mTo);
				for (EncodedStringValue element : tos) {					
					pdu.addTo(element);
				}
			} 
			if(mHeaders.mSubject != null){
				pdu.setSubject(new EncodedStringValue(mHeaders.mSubject));
			}
			if (mHeaders.mTimeStamp != 0) {
				pdu.setDate(mHeaders.mTimeStamp / 1000L);
			} else {
				pdu.setDate(System.currentTimeMillis() / 1000L);
			}
			//set body
			PduBody body = generatePduBody();
			if (body != null) {
				pdu.setBody(body);
			}
		}catch (InvalidHeaderValueException e){
			log(e.toString());
			return null;
		}
		return pdu;
	}
	//in general, only sendreq is neccessary to generate for mms
	public SendReq generateSendReqPdu(){
		log("generateSendReqPdu()");
		SendReq req = new SendReq();

		try{
		if (mHeaders.mFrom != null) {
	//		req.setFrom(new EncodedStringValue(mHeaders.mFrom));
		} 
		
		req.setPriority(PduHeaders.PRIORITY_NORMAL);
		//convert date format
	//	req.setDate(mHeaders.mTimeStamp);
		req.setDate(System.currentTimeMillis() / 1000L);
		if (mHeaders.mSubject != null) {
			req.setSubject(new EncodedStringValue(mHeaders.mSubject));
		}
		req.setContentType(getContentType(mHeaders.mMiltipartType));
		req.setExpiry(DEFAULT_EXPIRY_TIME);
		req.setMessageSize(mHeaders.mSize);
		if (mHeaders.mTo != null) {
			req.setTo(reverseAddress(mHeaders.mTo));
		}
		req.setDeliveryReport(PduHeaders.VALUE_NO);
		req.setReadReport(PduHeaders.VALUE_NO);
		} catch (InvalidHeaderValueException e){
			log (e.toString());
		}

		//set body
		PduBody body = generatePduBody();
		if (body != null) {
			req.setBody(body);
		}
		return req;
	}
    private static final boolean DEFAULT_DELIVERY_REPORT_MODE  = false;
    private static final boolean DEFAULT_READ_REPORT_MODE      = false;
	public PduBody generatePduBody(){
		PduBody body = new PduBody();

		
		if (mAttachment == null || mAttachment.length == 0) {
			log("attachment is null");
			return null;
		}
		for (MimeAttachment attachement : mAttachment) {
			PduPart part = new PduPart();			
			
			if (attachement.mContentBytes != null && attachement.mContentBytes.length > 0) {
				part.setData(getDataBytes(attachement.mEncoding, attachement.mContentBytes));				
			} else if (attachement.mContentUri != null) {
				part.setDataUri(attachement.mContentUri);
			} else {
				log("data is null");
				continue;
			}
			if (attachement.mContentId != null) {
				log("mContentId is "+attachement.mContentId);
				part.setContentId(attachement.mContentId.getBytes());
			}
			if (attachement.mLocation != null && 
					!attachement.mLocation.equals(MimeContent.CONTNET_DISPOSITION_INLINE)) {
				part.setContentDisposition(attachement.mLocation.getBytes());
			}
			if (attachement.mContentLocation != null ) {
				part.setContentLocation(attachement.mContentLocation.getBytes());
			}
			
			part.setContentType(getContentType(attachement.mMimeType));

			if (attachement.mCharset != null) {
				try {
					int charsetInt = CharacterSets.getMibEnumValue(attachement.mCharset);
					if (charsetInt != -1) {
						part.setCharset(charsetInt);
					}
				} catch (UnsupportedEncodingException e) {
					log(e.toString());
				}
			}
			part.setContentTransferEncoding(PduPart.P_8BIT.getBytes());
			if (attachement.mFileName != null) {
				part.setFilename(attachement.mFileName.getBytes());
			}

			if ((null == attachement.mLocation)
                    && (null == attachement.mFileName)
                    && (null == attachement.mContentId)) {
                part.setContentLocation(Long.toOctalString(
                        System.currentTimeMillis()).getBytes());
            }
			if (attachement.mName != null){
				part.setName(attachement.mName.getBytes()) ;
			}
			body.addPart(part);
		}
		return body;
	}

	private byte[] getDataBytes(String encoding, byte[] rawdata) {
		if (encoding == null) {
			return rawdata;
		}
		String newEncoding = encoding.toLowerCase();
		if (newEncoding.equalsIgnoreCase(PduPart.P_BASE64)) {
            return Base64.decodeBase64(rawdata);
        } else if (encoding.equalsIgnoreCase(PduPart.P_QUOTED_PRINTABLE)) {
            return QuotedPrintable.decodeQuotedPrintable(rawdata);
        } else {
            return rawdata;
        }
	}

	private byte[] getContentType(String originalType) {
		if (originalType == null || originalType.equals(MimeContent.TEXT_PLAIN)) {
			return MimeContent.TEXT_PLAIN.getBytes();
		} 
		String type = originalType.toLowerCase();
		if(type.equals(MimeContent.TEXT_PLAIN)) {
			type = MimeContent.TEXT_PLAIN;
		} else if (type.equals(MULTIPART_MIX)){
			type = ContentType.MULTIPART_MIXED;
		} else if (type.equals(MULTIPART_ALTERNATIVE)){
			type = ContentType.MULTIPART_ALTERNATIVE;	
		} else if (type.equals(MULTIPART_RELATED)){
			type = ContentType.MULTIPART_RELATED;	
		} 
		log("type is "+type);
		return type.getBytes();
	}

	public void setFromField(String from) {
		mHeaders.mFrom  = from;
	}
	public void addToField(String to) {
		if (to == null) {
			return;
		}
		if (mHeaders.mTo != null) {
			mHeaders.mTo = mHeaders.mTo + ";";
			mHeaders.mTo  = mHeaders.mTo + to;
		} else {
			mHeaders.mTo = to;
		}
	}
	public boolean isHeaderComplete(){
		return (mHeaders.mFrom != null) && (mHeaders.mTo != null);
	}
	private void log(String info){
		if (null != info){
			Xlog.v(TAG, info);
		}
	}
}