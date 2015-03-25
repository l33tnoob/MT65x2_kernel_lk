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

package com.mediatek.bluetooth.map.cache;
import com.mediatek.bluetooth.map.MAP;
import com.mediatek.bluetooth.map.util.HandleUtil;
//rule:
// 1 no check when set data
// 2.the object operating the class is responsible for check 
public class MessageRequest {
		private int mMasId;
		private String mAddr; // todo: maybe not useful
		private long mHandle;
		private boolean mAttachement;
		private int mCharset;
		private int mFractionReq; // optional

		public MessageRequest(int id) {
			mMasId = id;
		}

		//the message can only be accessed by native method in JNI
		//Todo: check the address
	/*	private void setInfo (String devAddr, String msgHandle, boolean attachement, String charset, String fraction_req){
			mAddr = devAddr;
			mHandle = msgHandle;
			this.attachement = attachement;
			mCharset = charset;
			mFractionReq = fraction_req;
		}

		private void setInfo (String devAddr, String msgHandle, boolean attachement, String charset){
			mAddr = devAddr;
			mHandle = msgHandle;
			mAttachement = attachement;
		}
*/
		public boolean isAttachDelivered() {
			return mAttachement;
		}

		public long getMessageId(){
			return HandleUtil.getId(mHandle) ;
		}

		public String getAddress(){
			return mAddr;
		}
		public long getHandle(){
			return mHandle;
		}
		public int getCharset(){
			return mCharset;
		}

		public void setHandle(long id){
			mHandle = id;
		}
		public void setAttachment(boolean attach){
			mAttachement = attach;
		}

		public int getFractionReq () {
			return mFractionReq;
		}
	}