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

import android.content.Context;
import android.content.ContentResolver;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import android.provider.Telephony;

import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.net.http.AndroidHttpClient;

import android.text.TextUtils;

import android.os.PowerManager;

import android.content.IntentFilter;
import android.content.Intent;
import android.content.BroadcastReceiver;


import org.apache.http.HttpEntity;
import org.apache.http.HttpHost;
import org.apache.http.HttpRequest;
import org.apache.http.HttpResponse;
import org.apache.http.StatusLine;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.conn.params.ConnRouteParams;
import org.apache.http.params.HttpParams;
import org.apache.http.params.HttpProtocolParams;
import org.apache.http.params.HttpConnectionParams;
import org.apache.http.entity.ByteArrayEntity;

import android.database.Cursor;

import android.telephony.TelephonyManager;

import android.util.Log;

import java.io.OutputStream;
import java.io.IOException;
import java.io.DataInputStream;

import java.net.SocketException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.InetAddress;
import java.net.UnknownHostException;

import android.net.NetworkInfo;
import android.net.NetworkInfo.State;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

import java.util.Locale;
import java.util.ArrayList;

import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import com.android.internal.telephony.Phone;

import com.mediatek.bluetooth.map.util.NetworkUtil;
import com.mediatek.xlog.Xlog;
import org.apache.http.impl.client.DefaultHttpRequestRetryHandler;

public class MmsConnection {
	private static final String TAG = "MmsConnection";

	public static final int STATE_PENDING = 0;
	public static final int STATE_SUCCESS = 1;
	public static final int STATE_FAIL  = 2;

  // Definition for necessary HTTP headers.
    private static final String HDR_KEY_ACCEPT = "Accept";
    private static final String HDR_KEY_ACCEPT_LANGUAGE = "Accept-Language";

    private static final String HDR_VALUE_ACCEPT =
        "*/*, application/vnd.wap.mms-message, application/vnd.wap.sic";

	private static final String[] APN_PROJECTION = {
            Telephony.Carriers.TYPE,            // 0
            Telephony.Carriers.MMSC,            // 1
            Telephony.Carriers.MMSPROXY,        // 2
            Telephony.Carriers.MMSPORT          // 3
    };
    private static final int COLUMN_TYPE         = 0;
    private static final int COLUMN_MMSC         = 1;
    private static final int COLUMN_MMSPROXY     = 2;
    private static final int COLUMN_MMSPORT      = 3;

	private static final String DEFAULT_USER_AGENT = "Android-Mms/2.0";
	private static int mHttpSocketTimeout = 60*1000;            // default to 1 min

	private static final String HTTP_PARAMS_LINE1_KEY = "##LINE1##";
	private static final String UAPROF_TAG_NAME 		= "x-wap-profile";
	private static final String UAPROF 				  = "http://www.google.com/oha/rdf/ua-profile-kila.xml";
	private static final String CALLING_LINE			= "x-up-calling-line-id";
	
	public static final long NO_TOKEN = -1L;

	private static final String HDR_VALUE_ACCEPT_LANGUAGE;

	private static final int MMS_EVENT_NEW_PDU = 0;
	private static final int MMS_EVENT_NETWORK_READY = 1;
	private static final int MMS_EVENT_SEND_COMPLETE = 2;

    static {
        HDR_VALUE_ACCEPT_LANGUAGE = getHttpAcceptLanguage();
    }

	private static Context mContext;
	private static String mServiceCenter;
    private static String mProxyAddress;
    private static int mProxyPort = -1;
    private ConnectivityManager mConnMgr;

	private static int mSimId;

	private static String mUserAgent = DEFAULT_USER_AGENT;

	private ArrayList<Task> mPendingTask = new ArrayList<Task>();

	private static MmsConnection mMmsConnection;

	private ConnectionListener mLisenter;
	private boolean mReceiverFlag = false;
	private boolean mDataTransferring = false;
	private class Task {
		public long mToken;
		public byte[] mPdu;
	}

	public interface ConnectionListener{
		void onSendResult(int state);
	}

	private final Handler mHandler = new Handler() {
		public void handleMessage(Message msg) {
			int netState;
			int event = msg.what;
			log("event:"+event);
			switch(event) {
			case MMS_EVENT_NEW_PDU:
				if (mPendingTask.size() == 0) {
					sendMessage(obtainMessage(MMS_EVENT_SEND_COMPLETE));
					break;
				}
				netState = startConnection();
				if (netState == PhoneConstants.APN_ALREADY_ACTIVE){
					if (!mDataTransferring) {
						process();
						mDataTransferring = true;
					}
				} else if (netState == PhoneConstants.APN_REQUEST_STARTED){
					if (!mReceiverFlag) {
						IntentFilter filter = new IntentFilter();
						filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
						mContext.registerReceiver(mReceiver, filter);
						mReceiverFlag = true;
					}
				} else {
					log("fail to begin mms connection:"+netState);
				}
				break;
			case MMS_EVENT_SEND_COMPLETE:
				mDataTransferring = false;
			case MMS_EVENT_NETWORK_READY:				
				if (mPendingTask.size() > 0) {
					sendMessage(obtainMessage(MMS_EVENT_NEW_PDU));
				} else {
					if (mReceiverFlag) {
					mContext.unregisterReceiver(mReceiver);
					mReceiverFlag = false;
					}
				}
				break;
			default:		
				log("unexpected event");
		}
	}
	};

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
        	log("receive intent:"+intent.getAction());
            if (ConnectivityManager.CONNECTIVITY_ACTION.equals(intent.getAction())) {
				int simId = intent.getIntExtra(ConnectivityManager.EXTRA_SIM_ID, -1);
		Bundle b = intent.getExtras();
                if (b != null) {
		    NetworkInfo a = (NetworkInfo)b.get(ConnectivityManager.EXTRA_NETWORK_INFO);
					if (a == null){
						return;
					}
		    State state = a.getState();
		    int type = a.getType();
		    int subtype = a.getSubtype();
                    log( "Connectivity type name:" + a.getTypeName()+
						",type is "+type+
						", subtype is " + subtype +
						"state is "+state +
						"sim id is "+ simId);                    
					if ((type == ConnectivityManager.TYPE_MOBILE || type == ConnectivityManager.TYPE_MOBILE_MMS) &&
						state == State.CONNECTED) {
						mHandler.sendMessage(mHandler.obtainMessage(MMS_EVENT_NETWORK_READY));
			          }
        }
				
            }
        }
    };

	private MmsConnection(Context context, int sim){
		mContext = context;
		mSimId = sim;
		init();
		mConnMgr = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
       
		log("mServiceCenter:"+mServiceCenter+",mProxyAddress:"+mProxyAddress
			+",mProxyPort:"+mProxyPort);
	}

	public static MmsConnection getDefault(Context context, int sim){
		if (mMmsConnection == null) {
			mMmsConnection = new MmsConnection(context, sim);
		}
		init();
		return mMmsConnection;
	}

	public void registerListener(ConnectionListener l){
		mLisenter = l;
	}

	public void unregisterListener(){
		mLisenter = null;
	}

	private static void init(){
		String selection = null;
		Cursor cursor = mContext.getContentResolver().query(
                            Uri.withAppendedPath(Telephony.Carriers.CONTENT_URI, "current"),
                            APN_PROJECTION, selection, null, null);
		if (cursor == null) {
            Log.e(TAG, "Apn is not found in Database!");
            return;
        }
		boolean sawValidApn = false;
        try {
            while (cursor.moveToNext() && TextUtils.isEmpty(mServiceCenter)) {
                // Read values from APN settings
                if (isValidApnType(cursor.getString(COLUMN_TYPE), PhoneConstants.APN_TYPE_MMS)) {
                    sawValidApn = true;
                    mServiceCenter = cursor.getString(COLUMN_MMSC) != null ? cursor.getString(COLUMN_MMSC).trim():null;
                    mProxyAddress = cursor.getString(COLUMN_MMSPROXY);
                    if (isProxySet()) {
                        String portString = cursor.getString(COLUMN_MMSPORT);
                        try {
                            mProxyPort = Integer.parseInt(portString);
                        } catch (NumberFormatException e) {
                            if (TextUtils.isEmpty(portString)) {
                                Log.w(TAG, "mms port not set!");
                            } else {
                                Log.e(TAG, "Bad port number format: " + portString, e);
                            }
                        }
                    }
                }
            }
        } finally {
            cursor.close();
        }

        if (sawValidApn && TextUtils.isEmpty(mServiceCenter)) {
            Log.e(TAG, "Invalid APN setting: MMSC is empty");
        }
		
	
	}


	public void send(long token, byte[] pdu){		
		if (pdu == null || pdu.length == 0) {
			log("pdu to send is null");
			return;
		}
		Task task = new Task();
		task.mToken = token;
		task.mPdu = pdu;
		mPendingTask.add(task);
		
		mHandler.sendMessage(mHandler.obtainMessage(MMS_EVENT_NEW_PDU));
	}
			
	private void process(){		
		Thread thread = new Thread(){
			public void run(){
				Task task = mPendingTask.remove(0);
				if (task != null){
					processConnection(task.mToken,task.mPdu);
				}
			}
		};
		thread.start();
		
	}

	private void processConnection(long token, byte[] pdu){
		if (ensureRouteToHost()){
			try {
				httpConnection(token, pdu);
			} catch (IOException e){
				log(e.toString());
			}
		} else {
			log("fail to rout to host");
		}
		mHandler.sendMessage(mHandler.obtainMessage(MMS_EVENT_SEND_COMPLETE));
	}

	private int startConnection(){
		int result;
		if(NetworkUtil.isGeminiSupport()){
			result = beginMmsConnectivityGemini(mSimId);
		} else {
			result = beginMmsConnectivity();
		}
		return result;
	}
	private void endConnection(){
		if(NetworkUtil.isGeminiSupport()){
			endMmsConnectivityGemini(mSimId);
		} else {
			endMmsConnectivity();
		}
	}
	public byte[] httpConnection(long token, byte[] pdu) throws IOException {
        String url = mServiceCenter;

		log("httpConnection()");

	//	beginMmsConnectivityGemini();

        AndroidHttpClient client = null;

        try {
            // Make sure to use a proxy which supports CONNECT.
            URI hostUrl = new URI(url);
            HttpHost target = new HttpHost(
                    hostUrl.getHost(), hostUrl.getPort(),
                    HttpHost.DEFAULT_SCHEME_NAME);

            client = createHttpClient();
            HttpRequest req = null;
            ProgressCallbackEntity callbackentity = new ProgressCallbackEntity(
                                                        mContext, token, pdu);
            // Set request content type.
            callbackentity.setContentType("application/vnd.wap.mms-message");

            HttpPost post = new HttpPost(url);
            post.setEntity(callbackentity);
            req = post;
             

            // Set route parameters for the request.
            HttpParams params = client.getParams();
            if (isProxySet()) {
                ConnRouteParams.setDefaultProxy(
                        params, new HttpHost(mProxyAddress, mProxyPort));
            }
            req.setParams(params);

            // Set necessary HTTP headers for MMS transmission.
            req.addHeader(HDR_KEY_ACCEPT, HDR_VALUE_ACCEPT);
			req.addHeader(UAPROF_TAG_NAME, UAPROF);

			String line1Number = NetworkUtil.getPhoneNumber(mSimId);
			if (!TextUtils.isEmpty(line1Number)) {
	 			line1Number =  line1Number.trim();
				req.addHeader(CALLING_LINE, line1Number);
			}
			
            req.addHeader(HDR_KEY_ACCEPT_LANGUAGE, HDR_VALUE_ACCEPT_LANGUAGE);

            HttpResponse response = null;
            try {
           		response = client.execute(target, req);
            } catch (IOException e){
                Log.e(TAG, "AndroidHttpClient.execute exception: " + e.getMessage());
            }
			
			if (null == response){
				Log.v(TAG, "httpConnection: client.execute() return null !!");
				return null;
			}             

            StatusLine status = response.getStatusLine();
            if (status.getStatusCode() != 200) { // HTTP 200 is success.
                throw new IOException("HTTP error: " + status.getReasonPhrase());
            }

            HttpEntity entity = response.getEntity();
            byte[] body = null;
            if (entity != null) {
                try {
                    if (entity.getContentLength() > 0) {
                        body = new byte[(int) entity.getContentLength()];
                        DataInputStream dis = new DataInputStream(entity.getContent());
                        try {
                            dis.readFully(body);
                        } finally {
                            try {
                                dis.close();
                            } catch (IOException e) {
                                Log.e(TAG, "Error closing input stream: " + e.getMessage());
                            }
                        }
                    }
                } finally {
                    if (entity != null) {
                        entity.consumeContent();
                    }
                }
            }
            return body;
        } catch (URISyntaxException e) {
       //     handleHttpConnectionException(e, url);
      		log(e.toString());
        } catch (IllegalStateException e) {
      //      handleHttpConnectionException(e, url);
      		log(e.toString());
        } catch (IllegalArgumentException e) {
      //      handleHttpConnectionException(e, url);
      		log(e.toString());
        } catch (SocketException e) {
       //     handleHttpConnectionException(e, url);
       		log(e.toString());
        } catch (Exception e) {
       //     handleHttpConnectionException(e, url);
       		log(e.toString());
        }
        finally {
            if (client != null) {
                client.close();
            }
        }
        return null;
}
 private AndroidHttpClient createHttpClient() {
        AndroidHttpClient client = AndroidHttpClient.newInstance(mUserAgent, mContext);
        HttpParams params = client.getParams();
        HttpProtocolParams.setContentCharset(params, "UTF-8");

        // set the socket timeout
        int soTimeout = mHttpSocketTimeout;
       
        HttpConnectionParams.setSoTimeout(params, soTimeout);
		        
        /// M: Enable HTTP Retry
        client.setHttpRequestRetryHandler(new DefaultHttpRequestRetryHandler(1, true));
        return client;
    }
 public class ProgressCallbackEntity extends ByteArrayEntity {
	 private static final int DEFAULT_PIECE_SIZE = 4096; 

	 private final Context mContext;
	 private final byte[] mContent;
	 private final long mToken;
 
	 public ProgressCallbackEntity(Context context, long token, byte[] b) {
		 super(b);
 
		 mContext = context;
		 mContent = b;
		 mToken = token;
	 }
 
	 @Override
	 public void writeTo(final OutputStream outstream) throws IOException {
		 if (outstream == null) {
			 throw new IllegalArgumentException("Output stream may not be null");
		 }
 
		 boolean completed = false;
		 try {
			
			 int pos = 0, totalLen = mContent.length;
			 while (pos < totalLen) {
				 int len = totalLen - pos;
				 if (len > DEFAULT_PIECE_SIZE) {
					 len = DEFAULT_PIECE_SIZE;
				 }
				 outstream.write(mContent, pos, len);
				 outstream.flush();
 
				 pos += len;
 
				
			 }
			
			 completed = true;
		 } finally {
			 
		 }
	 }
 
	
 }


 static private boolean isValidApnType(String types, String requestType) {
        // If APN type is unspecified, assume APN_TYPE_ALL.
        if (TextUtils.isEmpty(types)) {
            return true;
        }

        for (String t : types.split(",")) {
            if (t.equals(requestType) || t.equals(PhoneConstants.APN_TYPE_ALL)) {
                return true;
            }
        }
        return false;
    }
 
 public static boolean isProxySet() {
		 return (mProxyAddress != null) && (mProxyAddress.trim().length() != 0);
	 }

 private static String getHttpAcceptLanguage() {
        Locale locale = Locale.getDefault();
        StringBuilder builder = new StringBuilder();

        addLocaleToHttpAcceptLanguage(builder, locale);
        if (!locale.equals(Locale.US)) {
            if (builder.length() > 0) {
                builder.append(", ");
            }
            addLocaleToHttpAcceptLanguage(builder, Locale.US);
        }
        return builder.toString();
    }
  private static void addLocaleToHttpAcceptLanguage(
            StringBuilder builder, Locale locale) {
        String language = locale.getLanguage();

        if (language != null) {
            builder.append(language);

            String country = locale.getCountry();

            if (country != null) {
                builder.append("-");
                builder.append(country);
            }
        }
		
    }

    public boolean ensureRouteToHost()  {
       log("ensureRouteToHost");
	   boolean result = true;
	   
       if (mConnMgr == null){
	   		mConnMgr = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
       	}
        int inetAddr;
        if (isProxySet()) {            
            inetAddr = lookupHost(mProxyAddress);
            if (inetAddr == -1) {
                log("Cannot establish route for " + mServiceCenter+ ": Unknown host");
				result = false;
            } else {
                if (!mConnMgr.requestRouteToHost(ConnectivityManager.TYPE_MOBILE_MMS, inetAddr)) {
                    log("Cannot establish route to proxy " + inetAddr);
					result = false;
                }
            }
        } else {
            Uri uri = Uri.parse(mServiceCenter);
            inetAddr = lookupHost(uri.getHost());
            if (inetAddr == -1) {
                log("Cannot establish route for " + mServiceCenter + ": Unknown host");
				result = false;
            } else {
                if (!mConnMgr.requestRouteToHost(ConnectivityManager.TYPE_MOBILE_MMS, inetAddr)) {
                    log("Cannot establish route to " + inetAddr + " for " + mServiceCenter);
					result = false;
                }
            }
        }
		return result;
    }
	public int lookupHost(String hostname) {
        InetAddress inetAddress;
        try {
            inetAddress = InetAddress.getByName(hostname);
        } catch (UnknownHostException e) {
            return -1;
        }
        byte[] addrBytes;
        int addr;
        addrBytes = inetAddress.getAddress();
        addr = ((addrBytes[3] & 0xff) << 24)
                | ((addrBytes[2] & 0xff) << 16)
                | ((addrBytes[1] & 0xff) << 8)
                |  (addrBytes[0] & 0xff);
        return addr;
    }

     public int beginMmsConnectivity() {
        // Take a wake lock so we don't fall asleep before the message is downloaded.
        createWakeLock();
		 log("beginMmsConnectivity");

        int result = mConnMgr.startUsingNetworkFeature(
                ConnectivityManager.TYPE_MOBILE, Phone.FEATURE_ENABLE_MMS);
       

        switch (result) {
            case PhoneConstants.APN_ALREADY_ACTIVE:
            case PhoneConstants.APN_REQUEST_STARTED:
                acquireWakeLock();
                return result;
			default :
				log("Cannot establish MMS connectivity"+result);
				return result;
        }
    }

    public void endMmsConnectivity() {
		log("endMmsConnectivity");
        try {      

            if (mConnMgr != null) {
                mConnMgr.stopUsingNetworkFeature(
                        ConnectivityManager.TYPE_MOBILE,
                        Phone.FEATURE_ENABLE_MMS);
               
            }
        } finally {
            releaseWakeLock();
        }
    }

  
  // add for gemini
	 public int beginMmsConnectivityGemini(int simId) {
		 // Take a wake lock so we don't fall asleep before the message is downloaded.
		 createWakeLock();

		 
  
		 // convert sim id to slot id
	//	 int slotId = SIMInfo.getSlotById(getApplicationContext(), simId);
		int slotId = simId;
  
		int result = mConnMgr.startUsingNetworkFeatureGemini(
				 ConnectivityManager.TYPE_MOBILE, Phone.FEATURE_ENABLE_MMS, slotId);
 		log("beginMmsConnectivityGemini: result is "+result);
		 switch (result) {
			 case PhoneConstants.APN_ALREADY_ACTIVE:
			 case PhoneConstants.APN_REQUEST_STARTED:
				 acquireWakeLock();				
				 return result;
			 default:
				 log("Cannot establish MMS connectivity:"+result);
				 return result;
		 }
	 }
  
	 // add for gemini
	 public void endMmsConnectivityGemini(int simId) {
	 	log("endMmsConnectivityGemini()");
		 try {
			 // convert sim id to slot id
			 //	 int slotId = SIMInfo.getSlotById(getApplicationContext(), simId);
				int slotId = simId;		
			 if (mConnMgr != null) {
				 mConnMgr.stopUsingNetworkFeatureGemini(
						 ConnectivityManager.TYPE_MOBILE,
						 Phone.FEATURE_ENABLE_MMS, slotId);			
			 }
		 } finally {
			 releaseWakeLock();
		 }
	 }

	private PowerManager.WakeLock mWakeLock;

	private synchronized void createWakeLock() {
        // Create a new wake lock if we haven't made one yet.
        if (mWakeLock == null) {
            PowerManager pm = (PowerManager)mContext.getSystemService(Context.POWER_SERVICE);
            mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "MMS Connectivity");
            mWakeLock.setReferenceCounted(false);
        }
    }

    private void acquireWakeLock() {
        // It's okay to double-acquire this because we are not using it
        // in reference-counted mode.
        mWakeLock.acquire();
    }

    private void releaseWakeLock() {
        // Don't release the wake lock if it hasn't been created and acquired.
        if (mWakeLock != null && mWakeLock.isHeld()) {
            mWakeLock.release();
        }
    }

  

  private static void log(String info){
	if (null != info){
	Xlog.v(TAG, info);
  }
}
}
