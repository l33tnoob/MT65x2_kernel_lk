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

package com.mediatek.CellConnService;

import com.mediatek.CellConnService.IPhoneStatesCallback;
import com.mediatek.CellConnService.IPhoneStatesMgrService;

import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;
import java.util.ArrayList;

import android.os.RemoteException;
import android.os.ServiceManager;

/**
 * @author mtk80196
 *
 */
public class CellConnMgr {

	private static final String TAG = "CellConnMgr";
	
	public static final int RESULT_UNKNOWN = -1;
	public static final int RESULT_OK = 0;
	public static final int RESULT_WAIT = 1;
	public static final int RESULT_ABORT = 2;
	public static final int RESULT_EXCEPTION = 3;
	public static final int RESULT_STATE_NORMAL = 4;
	
	public static final int REQUEST_TYPE_UNKNOWN = 300;
	public static final int REQUEST_TYPE_SIMLOCK = 302;
	public static final int REQUEST_TYPE_FDN = 304;
	public static final int REQUEST_TYPE_ROAMING = 306;
	
	public static final int FLAG_SUPPRESS_CONFIRMDLG = 0x80000000;
	public static final int FLAG_REQUEST_NOPREFER = 0x40000000;
	
    /**************************************************/
       /**
        * To activate an application to unlock SIM lock.
        */
        public static final String ACTION_UNLOCK_SIM_LOCK = 
                "com.android.phone.ACTION_UNLOCK_SIM_LOCK";
    
       /**
        * To identify unlock type.
        * <P>Type: int</P>
        */
        public static final String EXTRA_UNLOCK_TYPE = 
                "com.android.phone.EXTRA_UNLOCK_TYPE";
    
       /**
        * To identify SIM ME lock type.
        * <P>Type: int</P>
        */
        public static final String EXTRA_SIMME_LOCK_TYPE = 
                "com.android.phone.EXTRA_SIMME_LOCK_TYPE";
    
       /**
        * The SIM slot.
        * <P>Type: int(Phone.GEMINI_SIM_1, Phone.GEMINI_SIM_2,...)</P>
        */
        public static final String EXTRA_SIM_SLOT = 
                "com.android.phone.EXTRA_SIM_SLOT";

	public static final int VERIFY_TYPE_PIN = 501;
	public static final int VERIFY_TYPE_PUK = 502;
	public static final int VERIFY_TYPE_SIMMELOCK = 503;
	public static final int VERIFY_TYPE_PIN2 = 504;
	public static final int VERIFY_TYPE_PUK2 = 505;
    
    /**************************************************/
    
	public static String resultToString(int ret) {
		if (RESULT_OK == ret) {
			return "RESULT_OK";
		} else if (RESULT_WAIT == ret) {
			return "RESULT_WAIT";
		} else if (RESULT_ABORT == ret) {
			return "RESULT_ABORT";
		} else if (RESULT_UNKNOWN == ret) {
			return "RESULT_UNKNOWN";
		} else if (RESULT_EXCEPTION == ret) {
			return "RESULT_EXCEPTION";
		} else if (RESULT_STATE_NORMAL == ret) {
			return "RESULT_STATE_NORMAL";
		}
		
		return "null";
	}

	public CellConnMgr(Runnable r) {
		this.mOnServiceComplete = r;
		mResult = RESULT_UNKNOWN;
	}
	
	public CellConnMgr() {
		this.mOnServiceComplete = null;
		mResult = RESULT_UNKNOWN;
	}

	private Runnable mOnServiceComplete;
	private int mResult;
	private Handler mHandler = new Handler(Looper.getMainLooper());
	private IPhoneStatesMgrService mService;
	private Context mCtx;
	private int mPreferSlot;
	private boolean mIsVerifying;

	public void register(Context ctx) {
		Log.d(TAG, "register");

		mCtx = ctx;

		Intent it = new Intent("android.intent.action.CELLCONNSERVICE");
		mCtx.startService(it);
		mCtx.bindService(it, mConnection, Context.BIND_AUTO_CREATE);
	}

	public void unregister() {
		Log.d(TAG, "unregister");
		if (null != mService) {
			mCtx.unbindService(mConnection);
		}
	}

	public int getResult() {
		return mResult;
	}
	
	public int getPreferSlot() {
		return mPreferSlot;
	}

	private IPhoneStatesCallback.Stub mCallback = new IPhoneStatesCallback.Stub() {

		public void onComplete(int nRet) {
			Log.d(TAG, "IPhoneStatesCallback onComplete");
			mResult = nRet;
			if (null != mOnServiceComplete) {
				Log.d(TAG, "IPhoneStatesCallback call service complete");
				mHandler.post(mOnServiceComplete);
			} else {
				Log.d(TAG, "IPhoneStatesCallback no callback to call");
			}
			mIsVerifying = false;
		}
		
		public void onCompleteWithPrefer(int nRet, int nPreferSlot) {
			Log.d(TAG, "IPhoneStatesCallback onComplete with PreferSlot");
			Log.d(TAG, "IPhoneStatesCallback nRet = " + nRet + " nPreferSlot = " + nPreferSlot);
			mResult = nRet;
			if (RESULT_STATE_NORMAL == mResult && mPreferSlot != nPreferSlot) {
			    Log.d(TAG, "fzw");
			    mPreferSlot = nPreferSlot;
			    handleCellConn(mPreferSlot, REQUEST_TYPE_SIMLOCK);
            } else {
                mPreferSlot = nPreferSlot;
                if (null != mOnServiceComplete) {
                    Log.d(TAG, "IPhoneStatesCallback call service complete");
                    mHandler.post(mOnServiceComplete);
                } else {
                    Log.d(TAG, "IPhoneStatesCallback no callback to call");
                }
                mIsVerifying = false;
            }
		}
	};

	private ServiceConnection mConnection = new ServiceConnection() {

		public void onServiceConnected(ComponentName name, IBinder service) {
			// TODO Auto-generated method stub
			Log.d(TAG, "onServiceConnected PhoneStateService connected");

			mService = IPhoneStatesMgrService.Stub.asInterface(service);
			if (null == mService) {
				Log
						.e(TAG,
								"onServiceConnected PhoneStateService get service is null");
				return;
			}

			Log.d(TAG, "onServiceConnected notify service ready");
			synchronized (workerThread) {
				workerThread.setServiceReady();
				workerThread.notify();
			}
		}

		public void onServiceDisconnected(ComponentName name) {
			// TODO Auto-generated method stub
			Log.d(TAG, "onServiceDisconnected PhoneStateService disconnected");
			workerThread.resetThreadStarted();
			mService = null;
		}

	};

	/*
	 * Handle the request for Cell Connection
	 * Return:
	 *   RESULT_WAIT - need to wait the service callback
	 *   RESULT_OK - the current cell connection state is normal and can continue to next action
	 *   RESULT_EXCEPTION - exception occurred at the handling action
	 */
	public int handleCellConn(int slot, int reqType) {
		Log.d(TAG, "handleCellConn slot = " + slot + " reqType = " + reqType);
		mPreferSlot = slot;

		if(mIsVerifying == true) {
			Log.d(TAG, "There exists a comment");
			if (null != mOnServiceComplete) {
				Log.d(TAG, "IPhoneStatesCallback call service complete");
				mHandler.post(mOnServiceComplete);
			}
			return RESULT_ABORT;
		}

		if (null == mService) {
			Log
					.d(TAG,
							"handleCellConn mService is not ready and start thread to handle this request");
			workerThread.addElem(slot, reqType);
			if (!workerThread.isThreadStarted()) {
				Log.d(TAG, "handleCellConn start workerThread");
				workerThread.setThreadStarted();
				//workerThread.start();
				new Thread(workerThread).start();
			}

			return RESULT_WAIT;
		}

		return verifyCellState(slot, reqType);
	}

	public int handleCellConn(int slot, int reqType, Runnable r) {
		Log.d(TAG, "handleCellConn slot = " + slot + " reqType = " + reqType + "and Runable");
		mPreferSlot = slot;

		if(mIsVerifying == true) {
			Log.d(TAG, "There exists a comment");
			if (null != r) {
				Log.d(TAG, "IPhoneStatesCallback call service complete");
				mHandler.post(r);
			}
			return RESULT_ABORT;
		}

		if (null == r) {
			Log.d(TAG, "handleCellConn runable is null");
		}
		
		this.mOnServiceComplete = r;
		
		return handleCellConn(slot, reqType);
	}

	private int verifyCellState(int slot, int reqType) {
		Log.d(TAG, "verifyCellState slot = " + slot + " reqType = " + reqType);

		int nRet = RESULT_UNKNOWN;
		if (null != mService) {
			try {
				if (null != mCallback) {
					mIsVerifying = true;
					nRet = mService.verifyPhoneState(slot, reqType, mCallback);
				} else {
					Log.e(TAG, "verifyCellState mCallback is null");
					nRet = RESULT_EXCEPTION;
				}
			} catch (RemoteException e) {
				Log.e(TAG, e.toString());
				Log.d(TAG, "verifyCellState excpetion");
				mResult = RESULT_EXCEPTION;
				mHandler.post(mOnServiceComplete);
				nRet = RESULT_EXCEPTION;
				e.printStackTrace();
			}
		} else {
			Log.e(TAG, "verifyCellState mService is null");
			mResult = RESULT_EXCEPTION;
			mHandler.post(mOnServiceComplete);
			nRet = RESULT_EXCEPTION;
		}

		return nRet;
	}

	//private class WorkerThread extends Thread {
	private class WorkerThread implements Runnable {
		
		public WorkerThread() {
			mThreadStarted = false;
			mServiceReady = false;
		}

		private class Element {
			private int mSlot;
			private int mReqType;

			public Element(int slot, int reqType) {
				this.mSlot = slot;
				this.mReqType = reqType;
			}

			public Element() {
				this.mSlot = 0;
				this.mReqType = 0;
			}
			
			public int getSlot() {
				return mSlot;
			}
			
			public int getReqType() {
				return mReqType;
			}
		}
		
		public void addElem(int slot, int reqType) {
			mRequstList.add(new Element(slot, reqType));
		}
		
		public void setThreadStarted() {
			this.mThreadStarted = true;
		}
		
        public void resetThreadStarted() {
            this.mThreadStarted = false;
        }
		
		public boolean isThreadStarted() {
			return this.mThreadStarted;
		}
		
		public void setServiceReady() {
			this.mServiceReady = true;
		}

		@Override
		public void run() {
			// TODO Auto-generated method stub
			Log.d(TAG, "WorkerThread run");
			
			// In the case of killed CellConnService, it would not be start phonestatesmgrservice.
			// We help to start this service.
			// It does not matter to start it more than once.
			
			register(mCtx);
			
			
			try {
				synchronized (this) {
					if (!mServiceReady) {
						Log.d(TAG, "WorkerThread wait notify");
						wait(10000); // Wait 10s to avoid ANR
					}
					Log.d(TAG, "WorkerThread run current requstList size is " + mRequstList.size());
					for (Element elem : mRequstList) {
						Log.d(TAG, "WorkerThread request slot = " + elem.getSlot() + " reqType = " + elem.getReqType());
						verifyCellState(elem.getSlot(), elem.getReqType());
					}
				}
			} catch (InterruptedException e) {
				Log.e(TAG, "WorkerThread wait exception");
				e.printStackTrace();
				mResult = RESULT_EXCEPTION;
				mHandler.post(mOnServiceComplete);
			}
		}

		private ArrayList<Element> mRequstList = new ArrayList<Element>();
		
		private boolean mThreadStarted;
		private boolean mServiceReady;
	}

	private WorkerThread workerThread = new WorkerThread();

}
