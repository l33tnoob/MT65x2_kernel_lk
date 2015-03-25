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
import java.util.Vector;

import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.utils.PeriodicRefresher;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.core.ims.protocol.http.*;
/**
 * Resends the XCAP request according to a retry algorithm.
 */
public class XcapRequestsRetryManager extends PeriodicRefresher {
	/**
	 * The logger
	 */
	private Logger mLogger = Logger.getLogger(this.getClass().getName());

	/**
	 * The Container stores HttpRequest objects that to be resent.
	 */
	private List<HttpRequest> mResendRequestList;

	/**
	 * HttpRequest that to be resent.
	 */
	private HttpRequest mCurrentRequest;

	/**
	 * HttpResponse of the HttpRequest that to be resent.
	 */
	private HttpResponse mCurrentResponse;

	/**
	 * XdmManager instance
	 */
	private XdmManager mXdmManager;

	/**
	 * Retry intervals in seconds. 30s, 2min, 5min, 15min, 30min, 1hour,
	 * 2hours...24hours.
	 */
	private int[] mRetryIntervals = { 30, 2 * 60, 5 * 60, 15 * 60, 30 * 60,
			1 * 60 * 60, 2 * 60 * 60, 4 * 60 * 60, 8 * 60 * 60, 16 * 60 * 60,
			24 * 60 * 60 };

	/**
	 * Retrying times count.
	 */
	private int mRetryCount = -1;

	private static XcapRequestsRetryManager mInstance;

	/**
	 * Get the single instance.
	 * 
	 * @param xdmManager
	 *            XdmManager object that used to resend request
	 */
	public static XcapRequestsRetryManager getInstance(XdmManager xdmManager) {
		if (null == mInstance) {
			mInstance = new XcapRequestsRetryManager(xdmManager);
		}

		return mInstance;
	}

	/**
	 * Constructor
	 * 
	 * @param xdmManager
	 *            XdmManager object that used to resend request.
	 */
	private XcapRequestsRetryManager(XdmManager xdmManager) {
		mXdmManager = xdmManager;
		if (null == mResendRequestList) {
			mResendRequestList = new Vector<HttpRequest>();
		}
	}

	/**
	 * Add HttpRequest that will be send to XDMS into list.
	 * 
	 * @param request
	 */
	private void addRequest(HttpRequest request) {
		if (mLogger.isActivated()) {
			mLogger.debug("addRequest");
		}
		
		synchronized (mResendRequestList) {
			if (!mResendRequestList.contains(request)) {
				mResendRequestList.add(request);
			}
		}
	}

	/**
	 * Remove HttpRequest from list. When the request is sent successfully,
	 * remove it from list.
	 * 
	 * @param request
	 */
	private void removeRequest(HttpRequest request) {
		if (mLogger.isActivated()) {
			mLogger.debug("removeRequest");
		}
		
		synchronized (mResendRequestList) {
			if (mResendRequestList.contains(request)) {
				mResendRequestList.remove(request);
			}
		}
	}

	/**
	 * Set the new HttpResponse to appointed HttpRequest if it is resent. This
	 * method must be invoked before calling the method startRetry.
	 * 
	 * @param request
	 * @param response
	 */
	public void setCurrentRequestAndResponse(HttpRequest request,
			HttpResponse response) {
		if (mLogger.isActivated()) {
			mLogger.debug("setCurrentRequestAndResponse");
		}
		
		synchronized (mResendRequestList) {
			if (!response.isSuccessfullResponse()) {
				addRequest(request);
			} else {
				removeRequest(request);
			}

			mCurrentRequest = request;
			mCurrentResponse = response;

			if (response.isSuccessfullResponse()) {
				// Always Get the first HttpRequest in the list to send every
				// time until the list size is zero.
				if (mResendRequestList.size() > 0) {
					mCurrentRequest = mResendRequestList.get(0);
					try {
						mCurrentResponse = mXdmManager
								.sendHttpRequestToXDMS(mCurrentRequest);
					} catch (CoreException e) {
						e.printStackTrace();
					}
				}
			} else {
				// Start retrying
				startRetry();
			}
		}

	}

	/**
	 * Start Retrying
	 */
	public void startRetry() {
		++mRetryCount;
		
		if (mLogger.isActivated()) {
			mLogger.debug("startRetry: mRetryCount = " + mRetryCount);
		}

		if (mRetryCount < mRetryIntervals.length) {
			startTimer(mRetryIntervals[mRetryCount]);
		} else {
			startTimer(mRetryIntervals[mRetryIntervals.length - 1]);
		}
	}

	/**
	 * Stop Retrying
	 */
	public void stopRetry() {
		if (mLogger.isActivated()) {
			mLogger.debug("stopRetry");
		}
		
		mRetryCount = -1;
		mCurrentRequest = null;
		mCurrentResponse = null;
		stopTimer();
	}

	@Override
	public void periodicProcessing() {
		if (mLogger.isActivated()) {
			mLogger.debug("periodicProcessing");
		}
		
		if (null != mCurrentResponse
				&& !mCurrentResponse.isSuccessfullResponse()) {
			try {
				if (null != mCurrentRequest) {
					mCurrentResponse = mXdmManager
							.sendHttpRequestToXDMS(mCurrentRequest);
				} else {
					if (mLogger.isActivated()) {
						mLogger.info("periodicProcessing: Resend XCAP request but it's null.");
					}
				}
			} catch (CoreException e) {
				e.printStackTrace();
			}
		}
	}
}
