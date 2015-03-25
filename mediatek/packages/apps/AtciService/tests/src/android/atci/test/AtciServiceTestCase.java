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

package android.atci.test;

import android.test.AndroidTestCase;
import android.test.ServiceTestCase;
import android.content.Intent;
import android.util.Log;
import android.os.IBinder;
import android.os.Handler;
import android.os.SystemClock;
import com.mediatek.atci.service.AtciService;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.net.Uri;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class AtciServiceTestCase extends
             ServiceTestCase<AtciService> {
    
    public static final String LOG_TAG = "AtciServiceTestCase";   
    LocalSocket mSocket;
    static final String SOCKET_NAME_ATCI = "atci-service";
    static final int SOCKET_OPEN_RETRY_MILLIS = 4 * 1000;
    static final int ATCI_MAX_BUFFER_BYTES = 1024;

    static final int NUM_ZERO = 0;
    static final int NUM_ONE = 1;
    static final int NUM_TWO = 2;
    static final int NUM_THREE = 3;
    static final int NUM_FOUR = 4;
    static final int NUM_FIVE = 5;
    static final int NUM_SIX = 6;
    static final int NUM_SEVEN = 7;
    static final int NUM_TEN = 10;
    static final int NUM_ELEVEN = 11;
    static final int NUM_TWELVE = 12;
    static final int NUM_SIXTEEN = 16;
    static final int NUM_FORTY_TWO = 42;
    static final int THREAD_SLEEP_ONE = 2000;
    static final int THREAD_SLEEP_TWO = 4000;
    static final int TESTCASE_SLEEP_TIME = 5000;
    	
    public class AtciServiceClone extends AtciService{
        public AtciServiceClone() {
            //super(this);		 
        }
        public void handleInputClone(String str){
            handleInput(str);
        }
    }
    public void connectToAtciService(){
		int retryCount = 0;
		String socketAtci = SOCKET_NAME_ATCI;
		for (;;) {
		LocalSocket s = null;
		LocalSocketAddress l;
		Log.d(LOG_TAG, "connectToAtciService");
		try {
			s = new LocalSocket();
			l = new LocalSocketAddress(socketAtci, LocalSocketAddress.Namespace.RESERVED);
			s.connect(l);
		} catch (IOException ex) {
			try {
				Log.e(LOG_TAG, "NullPointerException ex1");
				
				if (s != null) {
					s.close();
                    s = null;
				}
			} catch (IOException ex2) {
				//ignore failure to close after failure to connect
				Log.e(LOG_TAG, "NullPointerException ex2");
			}
		
			if (retryCount == NUM_SIXTEEN) {
				Log.e(LOG_TAG,
					   "Couldn't find '" + socketAtci
					   + "' socket after " + retryCount
					   + " times, continuing to retry silently");
			} else if (retryCount > NUM_ZERO && retryCount < NUM_SIXTEEN) {
				Log.i(LOG_TAG,
					   "Couldn't find '" + socketAtci
					   + "' socket; retrying after timeout");
			}
		
            try {
                Thread.sleep(SOCKET_OPEN_RETRY_MILLIS);
            } catch (InterruptedException er) {
                Log.e(LOG_TAG, "InterruptedException er");
            }		
        }
		if(null == s){
			Log.e(LOG_TAG, "NULL socket~~");
		}else{		
            mSocket = s;
            break;
		}
		}
        Log.i(LOG_TAG, "Connected to '" + socketAtci + "' socket");
    }
    public synchronized boolean sendURC(String urc) {
        boolean ret = true;
        int reTryConn = 0;
		
        if (urc.length() > 0) {
            Log.d(LOG_TAG, "URC Processing:" + urc + ">");
            while(true){
            OutputStream os = null;
			if(null != mSocket) {
            try {
                os = mSocket.getOutputStream();
                os.write(urc.getBytes());
                //os.close();
                break;
            } catch (IOException e) {
                e.printStackTrace();	
                try {
                    mSocket.close();
                    if(os != null)
                        os.close();
                } catch (IOException ex2) {
                    Log.d(LOG_TAG, "mSocket close fail.");
                }
                connectToAtciService();
                reTryConn++;
                if(4 == reTryConn)
                {
                    Log.d(LOG_TAG, "mSocket retry for 3 times.");
                    ret = false;
                    break;
                }
                ret = false;
            } 
            } else {
                Log.d(LOG_TAG, "mSocket is null");
                ret = false;
				break;
            }
            }
            return ret;
        }
        return true;
    }
    
    public AtciServiceTestCase() {
        super(AtciService.class);
        
    }
    
    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }
    
    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }
    
    public void testcase01_Startable() {
        try {
            Log.d(LOG_TAG, "[ATCISERVICE]testcase01_Startable: start service.");
            Intent startIntent = new Intent();
            startIntent.setClass(getContext(), AtciService.class);
            startService(startIntent);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public void testcase02_Bindable() {
        try {
            Log.d(LOG_TAG, "[ATCISERVICE]testcase02_Bindable: bindService.");
            Intent startIntent = new Intent();
            startIntent.setClass(getContext(), AtciService.class);
            IBinder service = bindService(startIntent);            
            connectToAtciService();			
            Log.d(LOG_TAG, "[ATCISERVICE]AT%FLIGHT.");
			
            sendURC("AT%FLIGHT");
            SystemClock.sleep(TESTCASE_SLEEP_TIME);				
            sendURC("AT%FLIGHT=0");
            SystemClock.sleep(TESTCASE_SLEEP_TIME);				
            sendURC("AT%FLIGHT=?");
            SystemClock.sleep(TESTCASE_SLEEP_TIME);
            sendURC("AT%FLIGHT=1");
            SystemClock.sleep(TESTCASE_SLEEP_TIME);			
            sendURC("AT%FLIGHT?");
            SystemClock.sleep(TESTCASE_SLEEP_TIME);
            sendURC("AT%FLIGHT=3");
            SystemClock.sleep(TESTCASE_SLEEP_TIME);				
            Log.d(LOG_TAG, "[ATCISERVICE]AT%ECALL.");
            sendURC("AT%ECALL");
            SystemClock.sleep(3000);			
            sendURC("AT%ECALL=?");
            SystemClock.sleep(3000);			
            sendURC("AT%ECALL?");
            SystemClock.sleep(3000);			
            sendURC("AT%ECALL=1");
            SystemClock.sleep(3000);			
            sendURC("AT%ECALL=0");
            SystemClock.sleep(3000);			
            sendURC("AT%ECALL=2");
            SystemClock.sleep(3000);			
            sendURC("AT%ECALL=3");
            SystemClock.sleep(3000);			
			
            Log.d(LOG_TAG, "[ATCISERVICE]AT%INITDB.");
            sendURC("AT%INITDB");
            SystemClock.sleep(3000);
            sendURC("AT%INITDB?");
            SystemClock.sleep(3000);
            Log.d(LOG_TAG, "[ATCISERVICE]AT%DBCHK.");
            sendURC("AT%DBCHK");
            SystemClock.sleep(3000);
            sendURC("AT%DBCHK?");
            SystemClock.sleep(3000);
            Log.d(LOG_TAG, "[ATCISERVICE]AT%OSVER.");
            sendURC("AT%OSVER");
            SystemClock.sleep(3000);
            sendURC("AT%OSVER?");
            SystemClock.sleep(3000);
            Log.d(LOG_TAG, "[ATCISERVICE]AT%BTAD.");
            sendURC("AT%BTAD");
            SystemClock.sleep(3000);
            sendURC("AT%BTAD?");
            SystemClock.sleep(3000);
            sendURC("AT%BTAD=?");
            SystemClock.sleep(3000);
            sendURC("AT%BTAD=1234");
            SystemClock.sleep(3000);	
/* NE in writeBTaddress			
            sendURC("AT%BTAD=1,2,3,4,5,6,7,8,9,0,1,2");
            SystemClock.sleep(3000);
*/            
            Log.d(LOG_TAG, "[ATCISERVICE]AT%BTTM.");
            sendURC("AT%BTTM");
            SystemClock.sleep(3000);
            sendURC("AT%BTTM?");
            SystemClock.sleep(3000);
            sendURC("AT%BTTM=?");
            SystemClock.sleep(3000);
/* NE			
            sendURC("AT%BTTM=2");
            SystemClock.sleep(3000);
            sendURC("AT%BTTM=3");
            SystemClock.sleep(3000);
*/            
            sendURC("AT%BTTM=7");//Error case
            SystemClock.sleep(3000);
            sendURC("AT%BTTM=AA");//Error case
            SystemClock.sleep(3000);

            Log.d(LOG_TAG, "[ATCISERVICE]AT%FMR.");
            sendURC("AT%FMR");
            SystemClock.sleep(3000);
            sendURC("AT%FMR?");
            SystemClock.sleep(3000);
            sendURC("AT%FMR=?");
            SystemClock.sleep(3000);
            Log.d(LOG_TAG, "[ATCISERVICE]AT%LANG.");
            sendURC("AT%LANG");
            SystemClock.sleep(3000);
            sendURC("AT%LANG?");
            SystemClock.sleep(3000);

            sendURC("AT%MPT");
            SystemClock.sleep(3000);
            sendURC("AT%MPT?");
            SystemClock.sleep(3000);
            sendURC("AT%MPT=?");
            SystemClock.sleep(3000);
            sendURC("AT%MPT=0");
            SystemClock.sleep(3000);
            sendURC("AT%MPT=6");
            SystemClock.sleep(3000);
            sendURC("AT%MPT=1,2");//Error case
            SystemClock.sleep(3000);
/* JE		
            sendURC("AT%MPT=1");
            SystemClock.sleep(3000);
            sendURC("AT%MPT=2");
            SystemClock.sleep(3000);
            sendURC("AT%MPT=3");
            SystemClock.sleep(3000);
            sendURC("AT%MPT=4");
            SystemClock.sleep(3000);
            sendURC("AT%MPT=5");
            SystemClock.sleep(3000);
            sendURC("AT%MPT=6");
            SystemClock.sleep(3000);
*/
            Log.d(LOG_TAG, "[ATCISERVICE]AT%NOSLEEP.");
            sendURC("AT%NOSLEEP");
            SystemClock.sleep(3000);
            sendURC("AT%NOSLEEP?");
            SystemClock.sleep(3000);
            sendURC("AT%NOSLEEP=?");
            SystemClock.sleep(3000);
            sendURC("AT%NOSLEEP=1");
            SystemClock.sleep(3000);
            sendURC("AT%NOSLEEP=0");
            SystemClock.sleep(3000);
            sendURC("AT%NOSLEEP=2");
            SystemClock.sleep(3000);
            sendURC("AT%NOSLEEP=1,1");
            SystemClock.sleep(3000);

            sendURC("AT+SN");
            SystemClock.sleep(3000);	
            sendURC("AT+MODEL");
            SystemClock.sleep(3000);			
            sendURC("AT+WITOF=2");
            SystemClock.sleep(3000);		
            sendURC("AT+WITOF=1");
            SystemClock.sleep(3000);
            sendURC("AT+WITOF=3");//Error case
            SystemClock.sleep(3000);
            sendURC("AT+WITOF=1,1");
            SystemClock.sleep(3000);			
            sendURC("AT%CAM");
            SystemClock.sleep(3000);			
            sendURC("AT%CAM?");
            SystemClock.sleep(3000);			
            sendURC("AT%CAM=?");
            SystemClock.sleep(3000);			
            sendURC("AT%CAM=7");
            SystemClock.sleep(3000);			
            sendURC("AT%CAM=13");
            SystemClock.sleep(3000);			
            sendURC("AT%CAM=5");
            SystemClock.sleep(3000);//Turn off cam						
            sendURC("AT%CAM=AA");//Error case
            SystemClock.sleep(3000);			
            sendURC("AT%AVR");
            SystemClock.sleep(3000);			
            sendURC("AT%AVR?");
            SystemClock.sleep(3000);			
            sendURC("AT%AVR=?");
            SystemClock.sleep(3000);			
            sendURC("AT%AVR=8");
            SystemClock.sleep(3000);			
            sendURC("AT%AVR=7");
            SystemClock.sleep(3000);
            sendURC("AT%AVR=5");//Turn off avr
            SystemClock.sleep(3000);
            sendURC("AT%AVR=AA");//Error case
            SystemClock.sleep(3000);
            sendURC("AT+POWERKEY");
            SystemClock.sleep(3000);
            sendURC("AT+FACTORYRESET");
            SystemClock.sleep(3000);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public void testcase03_StopService() {
        try {
            Log.d(LOG_TAG, "[ATCISERVICE]testcase03_StopService.");
            Intent startIntent = new Intent();
            startService(startIntent);
            AtciService service = getService();
            service.stopService(startIntent);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
