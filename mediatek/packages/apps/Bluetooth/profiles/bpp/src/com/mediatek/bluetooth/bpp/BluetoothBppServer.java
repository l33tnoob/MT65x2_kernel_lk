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

package com.mediatek.bluetooth.bpp;

import com.mediatek.bluetooth.R;

import android.content.Context;

import android.os.Handler;
import android.os.Message;
import android.os.Process;

import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;

public class BluetoothBppServer {

    private static final String TAG = "BluetoothBppServer";
 
    private final static String NO_DEFINITION = "nukonwn";

    //sync with bpp_mime_type_table (bt_bpp_porting.c, bpp_data.c), bt_bpp_mime_type(bluetooth_bpp_common.h)
    private final static String mimeTable[] = {"application/vnd.pwg-xhtml-print+xml:0.95",
                              "application/vnd.pwg-xhtml-print+xml:1.0",
                              "application/vnd.pwg-multiplexed",
                              "text/plain",
                              "text/x-vcard:2.1",
                              "text/x-vcard:3.0",
                              "text/x-vcalendar:1.0",
                              "text/calendar:2.0",
                              "text/x-vmessage:1.1",
                              "text/x-vnote:1.1",
                              "image/jpeg",
                              "image/gif",
                              "image/bmp",
                              "image/vnd.wap.wbmp",
                              "image/png",
                              "image/svg+xml"};

/*
    private final static String[] ORIENTATION = {
                                  "portrait",
                                  "landscape",
                                  "reverse-portrait",
                                  "reverse-landscape",
                                  NO_DEFINITION,
                                  NO_DEFINITION,
                                  NO_DEFINITION,
                                  NO_DEFINITION};

    private final static String[] SIDESETTING = {
                                  "one-sided",
                                  "two-sided-long-edge",
                                  "two-sided-short-edge",
                                  NO_DEFINITION,
                                  NO_DEFINITION,
                                  NO_DEFINITION,
                                  NO_DEFINITION,
                                  NO_DEFINITION};

    private final static String[] QUALITY = {
                                  "normal",
                                  "draft",
                                  "high",
                                  NO_DEFINITION,
                                  NO_DEFINITION,
                                  NO_DEFINITION,
                                  NO_DEFINITION,
                                  NO_DEFINITION};
*/  
    private final static String[] MEDIASIZE = {
                          "A10", "A9", "A8", "A7", "A6", "A5", "A5-extra", "A4", "A4-tab", "A4-extra", 
                          "A3", "A2", "A1", "A0", "2A0", "B10", "B9", "B8", "B7", "B6",
                          "B6C4", "B5", "B5-extra", "B4", "B3", "B2", "B1", "B0", "C10", "C9",
                          "C8", "C7", "C7C6", "C6", "C6C5", "C5", "C4", "C3", "C2", "C1",
                          "C0", "Photo 4x6", "Letter 8.5x11"}; 

    private static final int
        BPP_ENABLE_SUCCESS = 0,     // 600, BPP Service Enable Success
        BPP_ENABLE_FAIL = 1,            // 601, BPP Service Enable Fail
        BPP_DISABLE_SUCCESS = 2,        // 602, BPP Service Disable Success
        BPP_DISABLE_FAIL = 3,           // 603, BPP Service Disable Fail
        BPP_OBEX_AUTHREQ = 4,           // 604, OBEX Authentication Request
        BPP_CONNECT_SUCCESS = 5,        // 605, BPP Connection Success //(Not send to UI)
        BPP_CONNECT_FAIL = 6,           // 606, BPP Connection Fail  //(Not send to UI)
        BPP_GET_PRINT_ATTR_SUCCESS = 7, // 607
        BPP_GET_PRINT_ATTR_FAIL = 8,    // 608
        BPP_PROGRESS = 9,               // 609, BPP service
        BPP_PRINT_STATUS = 10,           // 610, BPP Print Status (Only Job-Based Transfer)
        BPP_PRINT_COMPLETE_SUCCESS = 11, // 611, BPP service
        BPP_PRINT_COMPLETE_FAIL = 12,    // 612, BPP service
        BPP_DISCONNECT_SUCCESS = 13,     // 613, BPP Connection Release Success
        BPP_DISCONNECT_FAIL = 14,        // 614, BPP Connection Release Fail
        BPP_CANCEL_SUCCESS = 15,         // 615, BPP service
        BPP_CANCEL_FAIL = 16;            // 616, BPP service


    private static String[] ORIENTATION;
    private static String[] SIDESETTING;
    private static String[] QUALITY;


    private class BppListener extends Thread {
        public BppListener(){
            super( "BipListener" );
        }

        @Override
        public void run() {
            Process.setThreadPriority( Process.THREAD_PRIORITY_BACKGROUND );

            Xlog.i(TAG, "[BPP Server] listen thread run......");
            BluetoothBppServer.this.startListenNative();
            Xlog.i(TAG, "[BPP Server] listen thread stopped......");
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
            BluetoothBppServer.this.stopListenNative();
        }
    }


    private int mNativeData = 0;
    private BppListener mListener;
    private Handler mCallback;

    private native static void classInitNative();
    private native void initializeDataNative();
    private native void cleanupDataNative();

    private native boolean enableServiceNative();
    private native boolean startListenNative();
    private native boolean stopListenNative();
    private native void disableServiceNative();

    private native boolean bppEnableNative();
    private native boolean bppDisableNative();
    private native void bppDisconnectNative();
    private native void bppAuthRspNative(AuthInfo authReply);
    private native void bppGetPrinterAttrNative(String address, int bitmask_attr);
    private native void bppPrintNative(String BdAddr, PrintObject object);
    //private native void bppCancelNative();

    public void onBppDisable(int cnfCode){};

    private static int mPreSentLength = 0;
    private static int mCurSentLength = 0;
    private static int mSObjLength = 0;


    static
    {
    //    try{
    //        Thread.sleep(15000);
    //    } catch (InterruptedException e) {
    //        e.printStackTrace();
    //    }
        System.loadLibrary("extbpp_jni");
        classInitNative();
    }


    public BluetoothBppServer(final Context context, final Handler callback)
    {
        mCallback = callback;

        ORIENTATION = context.getResources().getStringArray(R.array.bt_bpp_orientations);
        SIDESETTING = context.getResources().getStringArray(R.array.bt_bpp_sidesettings);
        QUALITY = context.getResources().getStringArray(R.array.bt_bpp_qualitys);
    }
/*----------------------------------------
Native call encapulation to JNI
----------------------------------------*/

    public boolean enable() {
        Xlog.i(TAG, "+enable");

//        if (!bppEnableNative()) {
//            Xlog.i(TAG, "-1 enable");
//            return false;
//        }

        if ( mNativeData == 0 )
        {
            initializeDataNative();
            enableServiceNative();
            mListener = new BppListener();
            mListener.startup();
        }
        else
        {
            Xlog.e(TAG, "mNativeData has been initialized");
        }

        if (!bppEnableNative()) {
            Xlog.i(TAG, "-1 enable");
            return false;
        }

        Xlog.i(TAG, "-enable");
        return true;
    }


    public void bppDisable(){
        Xlog.i(TAG, "+bppDisable");
        if (mNativeData != 0 )
        {
	        bppDisableNative();
	}
	else
	{
		Xlog.e(TAG, "mNativeData has been cleaned");
	}

        Xlog.i(TAG, "-bppDisable");
    }


    public void disableService(){
        Xlog.i(TAG, "+disableService");

        if (mNativeData != 0 )
        {
            try {
                mListener.shutdown();
                mListener.join();
                mListener = null;
            } catch (InterruptedException ex) {
                Xlog.w(TAG, "BppServer mListener close error");
            }
            disableServiceNative();    // release communication channel with BT-Task
            cleanupDataNative();
        }
        else
        {
            Xlog.e(TAG, "mNativeData has been cleaned");
        }

        Xlog.i(TAG, "-disableService");
    }

  
    public void bppGetPrinterAttr(String address, int bitmask_attr){
        Xlog.i(TAG, "+bppGetPrinterAttr");
	if (mNativeData != 0 )
	{
        	bppGetPrinterAttrNative(address, bitmask_attr);
	}
	else
	{
		Xlog.e(TAG, "mNativeData has been cleaned");
	}
	Xlog.i(TAG, "-bppGetPrinterAttr");
    }


    public void bppPrint(String address, PrintObject print_object){
        Xlog.i(TAG, "+bppPrint");

        print_object.nMimeType = 0;
        Xlog.i(TAG, Integer.toString(mimeTable.length) );
        while ( print_object.nMimeType < mimeTable.length &&
                print_object.MimeType.compareTo(mimeTable[print_object.nMimeType++]) != 0 ) {

            Xlog.i(TAG,mimeTable[print_object.nMimeType-1]);
        }
        print_object.nMimeType--;

        print_object.nObjectSize = Integer.parseInt(print_object.ObjectSize);
        mPreSentLength = 0;
        mCurSentLength = 0;
        mSObjLength = print_object.nObjectSize;
        print_object.nNumberUp = Integer.parseInt(print_object.NumberUp);

        print_object.nSides = 0 ;
        while ( print_object.Sides.compareTo(SIDESETTING[print_object.nSides]) != 0)
            print_object.nSides++;
        print_object.nSides = 0x01 << print_object.nSides;

        print_object.nOrient = 0 ;
        while ( print_object.Orient.compareTo(ORIENTATION[print_object.nOrient]) != 0)
            print_object.nOrient++;
        print_object.nOrient = 0x01 << print_object.nOrient;

        print_object.nQuality = 0 ;
        while ( print_object.Quality.compareTo(QUALITY[print_object.nQuality]) != 0)
            print_object.nQuality++;
        print_object.nQuality = 0x01 << print_object.nQuality;

        print_object.nMediaSize = 0;
        while ( print_object.MediaSize.compareTo(MEDIASIZE[print_object.nMediaSize]) != 0 )
            print_object.nMediaSize++;


        Xlog.i(TAG, " filePath: " + print_object.DirName + "\tfileName: " + print_object.FileName +
               "\tfileSize: " + print_object.nObjectSize + "\tmimeType:" + print_object.nMimeType +
               "\tsideSetting: " + print_object.nSides + "\tsheetSetting: " + print_object.nNumberUp +
               "\torientation: " + print_object.nOrient + "\tquality: " + print_object.nQuality +
               "\tmediasize: " + print_object.nMediaSize);
		
	if (mNativeData != 0 )
	{
        	bppPrintNative(address, print_object);
	}
	else
	{
		Xlog.e(TAG, "mNativeData has been cleaned");
	}
	Xlog.i(TAG, "-bppPrint");

    }

    public void bppDisconnect(){
	Xlog.i(TAG, "+bppDisconnect");
	if (mNativeData != 0 )
	{
       		 bppDisconnectNative();
	}
	else
	{
		Xlog.e(TAG, "mNativeData has been cleaned");
	}
	Xlog.i(TAG, "-bppDisconnect");
    }
/*
    public void bppCancel(){
        bppCancelNative();
    } 
*/
    public void bppAuthRsp(AuthInfo authReply){
	Xlog.i(TAG, "+bppAuthRsp");
	if (mNativeData != 0 )
	{
        	bppAuthRspNative(authReply);
	}
	else
	{
		Xlog.e(TAG, "mNativeData has been cleaned");
	}
	Xlog.i(TAG, "-bppAuthRsp");
    }
/*----------------------------------------
CALLBCAK form JNI
----------------------------------------*/
    private void onCallback(int event, int arg1, int arg2, String[] parameters){
    //mCallback.obtainMessage(msg, arg1, 0).sendToTarget();
    //should there are two handlers? One for server and one for client

        switch(event)
        {
            case BPP_ENABLE_SUCCESS:
            {
                Xlog.i(TAG, "BPP_ENABLE_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_ENABLE, cnfCode, 0).sendToTarget();
            } 
            break;
            case BPP_ENABLE_FAIL:
            {
                Xlog.i(TAG, "BPP_ENABLE_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_ENABLE, cnfCode, 0).sendToTarget();
            }
            break;
            case BPP_DISABLE_SUCCESS:
            {
                Xlog.i(TAG, "BPP_DISABLE_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_DISABLE, cnfCode, 0).sendToTarget();
            }
            break;
            case BPP_DISABLE_FAIL:
            {
                Xlog.i(TAG, "BPP_DISABLE_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_DISABLE, cnfCode, 0).sendToTarget();
            }
            break;
            case BPP_OBEX_AUTHREQ:
            { 
                Xlog.i(TAG, "BPP_OBEX_AUTHREQ");
                if( parameters[0].equals("1") )
                {
                    Xlog.i(TAG, "UserID required");
                }
                else
                {

                }
                mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_AUTH_IND).sendToTarget();
                //pop obex authentication dialog
            }
            break;
            case BPP_CONNECT_SUCCESS:
            {
                Xlog.i(TAG, "BPP_CONNECT_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_CONNECT_CNF, cnfCode, 0).sendToTarget();
            }
            break;
            case BPP_CONNECT_FAIL:
            {
                Xlog.i(TAG, "BPP_CONNECT_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_CONNECT_CNF, cnfCode, 0).sendToTarget();
            }
            break;
            case BPP_DISCONNECT_SUCCESS:
            {
                Xlog.i(TAG, "BPP_DISCONNECT_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_DISCONNECT_CNF, cnfCode, 0).sendToTarget();
            }
            break;
            case BPP_DISCONNECT_FAIL:
            { 
                Xlog.i(TAG, "BPP_DISCONNECT_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_DISCONNECT_CNF, cnfCode, 0).sendToTarget();
            }
            break;
            case BPP_CANCEL_SUCCESS:
            {
                Xlog.i(TAG, "BPP_CANCEL_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_CANCEL_CNF, cnfCode, 0).sendToTarget();
            } 
            break;
            case BPP_CANCEL_FAIL:
            {
                Xlog.i(TAG, "BPP_CANCEL_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_CANCEL_CNF, cnfCode, 0).sendToTarget();
            }
            break;
            case BPP_GET_PRINT_ATTR_SUCCESS:
            {
                Xlog.i(TAG, "BPP_GET_PRINT_ATTR_SUCCESS");

                byte sides = Byte.parseByte(parameters[0]);
                byte orientations = Byte.parseByte(parameters[1]);
                byte qualities = Byte.parseByte(parameters[2]);
                int max_numberup = Integer.parseInt(parameters[3]);
                int max_copies = Integer.parseInt(parameters[4]);
                int media_size_number = Integer.parseInt(parameters[5]);
                int printer_type = Integer.parseInt(parameters[6]);


                Xlog.i(TAG, "sides: " + sides + "\torientations: " + orientations + "\tqualities: " + qualities +
                           "\tmax_numberup: " + max_numberup + "\tmax_copies: " + max_copies +
                           "\tmedia_size_number: " + media_size_number + "\tprinter_type: " + printer_type);

                String[] paperSize = new String[media_size_number];
                for ( int i = 0; i < media_size_number; i++)
                {
                    paperSize[i] = MEDIASIZE[Integer.parseInt(parameters[7 + i])];
                }

                String[] sidesSetting = translateBitMap(sides, SIDESETTING);
                String[] orientation = translateBitMap(orientations, ORIENTATION);
                String[] qualitySetting = translateBitMap(qualities, QUALITY);

                String[] paperPerSheet;

                if ( max_numberup == 1  ) {
                    paperPerSheet = new String[1];
                    paperPerSheet[0] = "1";
                } else if ( max_numberup < 4 ) {
                    paperPerSheet = new String[2];
                    paperPerSheet[0] = "1";
                    paperPerSheet[1] = "2";
                } else {
                    paperPerSheet = new String[3];
                    paperPerSheet[0] = "1";
                    paperPerSheet[1] = "2";
                    paperPerSheet[2] = "4";
                }
                //String [] sidesSetting = {"One", "Two", "Three"};
                //String [] orientation = {"reverse-portrait", "reverse-landscape"};
                //String [] qualitySetting = {"Bx10", "xB5", "xB84"};
                //String [] paperSize = {"B10", "B5", "B84"};
                //String [] paperPerSheet = {"1", "2", "4"};
                //PrinterAttr pAttr_obj = new PrinterAttr( sidesSetting, orientation, qualitySetting, paperSize, paperPerSheet, 10);a
                int cnfCode = printer_type; 
                PrinterAttr pAttr_obj = new PrinterAttr( sidesSetting, orientation, qualitySetting, paperSize, paperPerSheet, max_copies);

                mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_GET_PRINTER_ATTR_CNF, cnfCode, 0, pAttr_obj).sendToTarget();
            }
            break;
            case BPP_GET_PRINT_ATTR_FAIL:
            {
                Xlog.i(TAG, "BPP_GET_PRINT_ATTR_FAIL");
                int cnfCode = -1;


                mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_GET_PRINTER_ATTR_CNF, cnfCode, 0).sendToTarget();
            }
            break;
            case BPP_PROGRESS:
            {
                Xlog.i(TAG, "BPP_PROGRESS");
                int sentDataLen = Integer.parseInt(parameters[0]);
                int totalDataLen = Integer.parseInt(parameters[1]);

                Xlog.i(TAG, "current: "+ mCurSentLength );
                Xlog.i(TAG, "present: "+ mPreSentLength );
                mCurSentLength = sentDataLen;
                if ( totalDataLen == 100 ) {
                    mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_PROGRESS_IND, sentDataLen, totalDataLen).sendToTarget();
                }
                else if ( (mCurSentLength - mPreSentLength) > mSObjLength/40  ) {
                    mPreSentLength = mCurSentLength;
                    mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_PROGRESS_IND, sentDataLen, totalDataLen).sendToTarget();
                }

                //mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_PROGRESS_IND, sentDataLen, totalDataLen).sendToTarget();
            }
            break;
            case BPP_PRINT_STATUS:
            {
                Xlog.i(TAG, "BPP_PRINT_STATUS");
                int jobState = Integer.parseInt(parameters[0]);
                int printerState = Integer.parseInt(parameters[1]);
                int printerStateReason = Integer.parseInt(parameters[2]);

                mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_JOBSTATUS_IND, printerStateReason, 0).sendToTarget();
            }
            break;
            case BPP_PRINT_COMPLETE_SUCCESS:
            {
                Xlog.i(TAG, "BPP_PRINT_COMPLETE_SUCCESS");
                int cnfCode = 0;
                mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_PRINT_CNF, cnfCode, 0).sendToTarget();
            }
            break;
            case BPP_PRINT_COMPLETE_FAIL:
            {
                Xlog.i(TAG, "BPP_PRINT_COMPLETE_FAIL");
                int cnfCode = -1;
                mCallback.obtainMessage(BluetoothBppManager.MSG_ON_BPP_PRINT_CNF, cnfCode, 0).sendToTarget();
            }
            break;
            default:
                Xlog.e(TAG, "UN-KNOWN EVENT");

        }
    }
  
/*----------------------------------------
AUX function
----------------------------------------*/
    private String[] translateBitMap(byte bitmap, String[] defaultentries) {
         byte ele_number = 0;

         for (byte i=0; i <8; i++)
         {
             byte mask = (byte)(0x80 >>> i);
             if ( (bitmap & mask) != 0  )
                 ele_number ++;
         }

         String[] listEntries = new String[ele_number];
         for (byte i=0; i < ele_number; i++){
             listEntries[i] = defaultentries[i];
             Xlog.i(TAG, "listEntries: " + listEntries[i]);
         }
         return listEntries;
    }
}//BluetoothBppServer CLASS end




/*----------------------------------------
AUX CLASS
----------------------------------------*/

class PrinterAttr {
    public String[] Sides;
    public String[] Orientations;
    public String[] Qualities;  
    public String[] MediaSize;
    public String[] MaxNumberup;
    int MaxCopies;

    public PrinterAttr(String[] sides, String[] orientations, String[] qualities, 
                     String[] mediasize, String[] maxnumberup, int maxcopies){
        Sides = sides;
        Orientations = orientations;
        Qualities = qualities;
        MediaSize = mediasize;
        MaxNumberup = maxnumberup;
        MaxCopies = maxcopies;
    }
}

class PrintObject {
    public String DirName;
    public String FileName;
    public String MimeType;   //it's int originally
    public boolean bJobBased;
    public String ObjectSize;  //it's int originally
    //PrintAttribute
    public int Copies;
    public String NumberUp; //it's int originally
    public String Sides;    //it's int originally
    public String Orient;   //it's int originally
    public String Quality;  //it's int originally

    public String DocFmt;
    public String MediaSize;
    public int PrintMediaType;
    //JobSatus
    public int JobState;
    public int PrinterState;
    public int PrinterStateReason;

    public int nObjectSize;
    public int nMimeType;
    public int nNumberUp;
    public int nSides;
    public int nOrient;
    public int nQuality;
    public int nMediaSize;


    public PrintObject(String dirname, String filename, String mimetype, String objectsize, boolean bjobbased,
                     int copies, String numberup, String sides, String orient, String quality, String mediasize){
        DirName = dirname;
        FileName = filename;
        MimeType = mimetype;
        ObjectSize = objectsize;
        bJobBased = bjobbased;

        Copies = copies;
        NumberUp = numberup;
        Sides = sides;
        Orient = orient;
        Quality = quality;
        MediaSize = mediasize;     
    }
}


class AuthInfo {
    public boolean bAuth;
    public String UserId;
    public String Passwd;

    public AuthInfo(boolean auth, String uid, String pwd){
        bAuth = auth;
        UserId = uid;
        Passwd = pwd;
    }
}


