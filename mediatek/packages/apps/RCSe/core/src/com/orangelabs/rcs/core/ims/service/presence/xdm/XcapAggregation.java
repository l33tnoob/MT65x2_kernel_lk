/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.orangelabs.rcs.core.ims.service.presence.xdm;

import java.util.List;

import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.ims.protocol.http.HttpPutRequest;
import com.orangelabs.rcs.core.ims.protocol.http.HttpRequest;
import com.orangelabs.rcs.utils.PeriodicRefresher;
import com.orangelabs.rcs.utils.logger.Logger;


/**
 * Resends the XCAP request according to a retry algorithm.
 */
public class XcapAggregation extends PeriodicRefresher {
	/**
     * The logger
     */
    private Logger mLogger = Logger.getLogger(this.getClass().getName());
    
	/**
	 * XdmManager instance
	 */
	private XdmManager mXdmManager;

	/**
	 * XCAP aggregation of update commands timer intervals 120s in seconds.
	 */
	private static final int INTERVAL_UPDATE_COMMAND = 120;
	
	/**
	 * Container to store the XCAP aggregation of update commands
	 */
	private List<Object> mUpdateCommandList;
	
	/**
	 * The MIME type.
	 */
	private String mMIMEType = "application/diff+xml";
	
	/**
	 * A single XcapAggregation instance.
	 */
	private static XcapAggregation mInstance = null;
	
	/**
	 * Get the single instance.
	 * @param xdmManager XdmManager object that used to resend request
	 */
	public static XcapAggregation getInstance(XdmManager xdmManager) {
		if (null == mInstance) {
			mInstance = new XcapAggregation(xdmManager);
		}
		
		return mInstance;
	}

	/**
	 * Constructor
	 * @param xdmManager XdmManager object that used to resend request
	 */
	private XcapAggregation(XdmManager xdmManager){
		mXdmManager = xdmManager;
	}
	
	/**
	 * Get the MIME-Type that put in the HTTP request Content-Type header.
	 * @return MIME-Type
	 */
	public String getMIMEType() {
		return mMIMEType;
	}

	/**
	 * Set the MIME-Type that put in the HTTP request Content-Type header.
	 * @param mimeType
	 */
	public void setMIMEType(String mimeType) {
		this.mMIMEType = mimeType;
	}
	
	/**
	 * Start to transfer aggregation of XCAP update commands to XDMS every 120 seconds.
	 */
	public void startUpdate() {
		if (mLogger.isActivated()) {
			mLogger.debug("startUpdate");
		}
		
		startTimer(INTERVAL_UPDATE_COMMAND);
	}

	@Override
	public void periodicProcessing() {
		if (mLogger.isActivated()) {
			mLogger.debug("periodicProcessing");
		}
		
		String url = buildXcapRequestURL();
		String content = buildXcapRequestContent();
		//TODO
		HttpRequest request = new HttpPutRequest(url, content, mMIMEType);
		
		try {
			mXdmManager.sendHttpRequestToXDMS(request);
		} catch (CoreException e) {
			e.printStackTrace();
		}
		
		// Updates for every 120s.
		startTimer(INTERVAL_UPDATE_COMMAND);
		
	}
	
	/**
	 * Build HTTP request URL
	 * @return url
	 */
	public String buildXcapRequestURL() {
		//TODO
		String url = "";
		return url;
	}
	
	/**
	 * Build HTTP request Content
	 * @return content HTTP body content with XML format
	 */
	public String buildXcapRequestContent() {
		String content = null;
		if (null != mUpdateCommandList) {
			int size = mUpdateCommandList.size();
			for (int i = 0; i < size; i++) {
				//TODO:
				content += mUpdateCommandList.get(i).toString();
			}
			
			mUpdateCommandList.clear();
		}
		return content;
	}
}
