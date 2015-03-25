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

package com.mediatek.app.mtv;



import android.database.Cursor;
import android.media.MediaRecorder;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.Surface;

import com.mediatek.atv.AtvChannelManager;
import com.mediatek.atv.AtvPlayer;
import com.mediatek.atv.AtvRecorder;
import com.mediatek.atv.AtvService;


import com.mediatek.mtvbase.ChannelManager;
import com.mediatek.mtvbase.Player;
import com.mediatek.mtvbase.Recorder;


import java.io.IOException;
import java.util.ArrayList;


/**
 * The engine is the interface between activity and other module.
 */
public class MtvEngine implements AtvService.EventCallback {
    
    /*MTV Modes*/
    public static final int MTV_ATV = ChannelManager.MTV_ATV;    
    /*MTV Errors*/
    public static final int MTV_MSG_INITIALIZE_DONE = 0xf0001001;    
    public static final int MTV_MSG_OPEN_DONE = 0xf0001002;    
    public static final int MTV_MSG_RELEASE_ENGINE = 0xf0001005; 
    public static final int MTV_MSG_NOTIFY_POWER_ON = 0xf0001006;    
    
    public static final int MTV_SCAN_PROGRESS = AtvService.ATV_SCAN_PROGRESS;
    public static final int MTV_SCAN_FINISH = AtvService.ATV_SCAN_FINISH;
    public static final int MTV_AUDIO_FORMAT_CHANGED = AtvService.ATV_AUDIO_FORMAT_CHANGED;
    public static final int MTV_CHIP_SHUTDOWN = AtvService.ATV_CHIP_SHUTDOWN;
    public static final int MTV_PLAYER_ERROR_SERVER_DIED  = Player.MTV_PLAYER_ERROR_SERVER_DIED;
    public static final int MTV_PLAYER_ERROR_UNKNOWN  = Player.MTV_PLAYER_ERROR_UNKNOWN;    
    public static final int MTV_RECORDER_ERROR_UNKNOWN = Recorder.MTV_RECORDER_ERROR_UNKNOWN;
    public static final int MTV_RECORDER_INFO_MAX_DURATION_REACHED = Recorder.MTV_RECORDER_INFO_MAX_DURATION_REACHED;
    public static final int MTV_RECORDER_INFO_MAX_FILESIZE_REACHED = Recorder.MTV_RECORDER_INFO_MAX_FILESIZE_REACHED;
    public static final int MTV_RECORDER_INFO_UNKNOWN = Recorder.MTV_RECORDER_INFO_UNKNOWN;    

    public static final int MTV_MSG_CAPTURE_DONE = 0x100;
    
    public static final String REQUEST_SHUTDOWN_CMD = "com.mediatek.app.mtv.ACTION_REQUEST_SHUTDOWN";
    public static final String NOTIFY_POWER_ON = "com.mediatek.app.mtv.POWER_ON";
        
    private static final int ATV_AUTOSCAN_MODE  = 0;    
    private static final int ATV_FULLSCAN_MODE  = 1;  
    private static final int ATV_QUICKSCAN_MODE  = 2;    
    
    
    private static final String TAG = "ATV/MtvEngine";


    private static final int MTVENGINE_STATE_IDLE = 0;
    private static final int MTVENGINE_STATE_INITIALIZING = 1;
    private static final int MTVENGINE_STATE_INITIALIZED = 2;
    private static final int MTVENGINE_STATE_OPENING = 3;
    private static final int MTVENGINE_STATE_INITIALIZING_OPENING = 4;
    private static final int MTVENGINE_STATE_INITIALIZING_OPENED = 5;
    private static final int MTVENGINE_STATE_OPENED = 6;
    private static final int MTVENGINE_STATE_SCANNING = 7;
    //H.263 don't support 320*240,so only support MP4 for ATV now.
    private static final int RECORDER_SPEC = MediaRecorder.VideoEncoder.MPEG_4_SP;
    private static final int DEFAULT_ALIVE_TIME = 20 * 1000;//in ms.
    
    private static MtvEngine sEngine = new MtvEngine();
    
    private int mState;
    private boolean mRequestTurnOff;
    
    private Thread mOpenPlayerThread;
    private Thread mTurnOnThread;
    private Thread mScanThread;    
    private ChannelProvider mChannelProvider;

    
    private int mCurrentChannel = -1;
    private ArrayList<ListItem> mCacheList;
    private ArrayList<ListItem> mList;
    private int mLastChannel;
    private int mChannelNum;

    private Player mPlayer;
    private Recorder mRecorder;
    private ChannelManager mChannelManager;
    private EventCallback mEventCallback;    
    private static final int MTVMODE = MTV_ATV;
    //in MtvEngine,mService is only responsible for controlling ATV chip only,
    //not used for player and recorder.So its lifetime is from turnOn to (internal)turnOff.
    private AtvService mService;
    private boolean mNeedSetTable = true;
    private boolean mFirstChannelFound = true;
    
    // Used to shade display area when change channel
    private static final int SHADE_TIME = 500;

    private final Handler mMainHandler = new Handler(Looper.getMainLooper()) {
                @Override
                public void handleMessage(Message msg) {
                    
                    boolean runAppCallback = true;
                    switch(msg.what) {
                    case MTV_SCAN_PROGRESS:
                        onScanProgressEvent(msg.arg1 >> 8,msg.arg1 & 0xff,(long)(Long)msg.obj,null);                        
                        break;
                        
                    case MTV_SCAN_FINISH:
                        if (mState == MTVENGINE_STATE_SCANNING) {    
                            mCacheList = null;
                            mLastChannel = 0;
                            mChannelNum = 0;                        
                            mState = MTVENGINE_STATE_INITIALIZED;                           
                        } else {
                            runAppCallback = false;
                        }
                        break;                    
                        
                    case MTV_CHIP_SHUTDOWN:
                        onDriverShutdown();                    
                        break;                                        

                    case MTV_PLAYER_ERROR_SERVER_DIED:    
                        //means that media server died,we need to do initialization again.
                        turnOff(true);   
                        break;
                    case MTV_PLAYER_ERROR_UNKNOWN:                    
                        watchOff();   
                        break; 
                        
                    case MTV_MSG_INITIALIZE_DONE:
                        initializeAndOpenDone(msg);
                        runAppCallback = false;
                        break;
                    case MTV_MSG_OPEN_DONE:
                        initializeAndOpenDone(msg);
                        runAppCallback = false;                    
                        break;
                    case MTV_MSG_RELEASE_ENGINE://we need to put it in another thread if it takes too lond time to execute.
                        internalTurnOff();
                        runAppCallback = false;                    
                        break;                      
                    default:
                        break;
                    }
                    
                    if (runAppCallback && mEventCallback != null) {
                        mEventCallback.onEvent(msg);
                    }                
                }
    };   
    
    /**
     *The list will save channel number and channel name.
     */
    public static class ListItem {
        //public int signal;
        public String mCh;
        public String mName;
        /**
         * save the channel num and name.
         */
        public ListItem(int c,String n) {
            mCh = String.valueOf(c);
            mName = n;
        }
    } 

    /**
     * When finish initialize or open, there is a message handleMessage will receive.
     * We will judge is it success or not.
     * @param msg
     */
    private void initializeAndOpenDone(Message msg) {
        switch(msg.what) {
        case MTV_MSG_INITIALIZE_DONE:
            mTurnOnThread = null;
            if (msg.arg1 == 1) {
                //Initialization succeed.
                initializeTrue();
            } else {
                initializeFalse();
            }
            if (!mRequestTurnOff) {
                if (mEventCallback != null) {
                    mEventCallback.onEvent(msg);
                    if (mState == MTVENGINE_STATE_OPENED) {
                        //this means opened is reached before initialized.
                        msg.what = MTV_MSG_OPEN_DONE;
                        mEventCallback.onEvent(msg);
                    }
                }
            }
            //runAppCallback = false;
            break;
        case MTV_MSG_OPEN_DONE:
            mOpenPlayerThread = null;
            if (msg.arg1 == 1) {
                //Initialization succeed.
                openTrue();
            } else {
                openFalse();
            }
            if (mEventCallback != null) {
                //postpone the notification of open event to MMI to the time of initialization done.
                if (mState == MTVENGINE_STATE_OPENED/*openTrue*/ ||
                    mState == MTVENGINE_STATE_INITIALIZED/*openFalse*/ ||
                    mState == MTVENGINE_STATE_INITIALIZING/*openFalse*/) {
                    mEventCallback.onEvent(msg);
                }
            }
            //runAppCallback = false;
            break;
          default:
            break;
        }
    }

    /**
     * Get MtvEngine instance.
     * @param mode  Indicator what get the MtvEngine.
     * @return  MtvEngine instance.
     */
    public static MtvEngine getEngine(int mode) {
        
        XLogUtils.d(TAG, "getEngine() sEngine = " + sEngine + " mode = " + mode);
        return sEngine;        
    }



    /**
     * MtvEngine constructor.
     */
    private MtvEngine() {
        //just to declare a private constructor.
    }
    
    /**
     * Get the last scanned channel.
     * @return Last scanned channel.
     */
    public int getLastChannel() {
        return mLastChannel;
    }

    /**
     * Get the channel number the country have.
     * @return Amount of channels.
     */
    public int getChannelNum() {
        return mChannelNum;
    }    
    
    /**
     * Scan progress call back function, when ATV chip finish scan a channel,
     * the function will be called.
     * @param chnum  Amount of channels the country has.
     * @param ch  The channel number device just scanned.
     * @param chInfo The channel's entry number.
     * @param obj Not use.
     */
      private void onScanProgressEvent(int chnum, int ch, long chInfo, Object obj) {      
        if (mState != MTVENGINE_STATE_SCANNING) { 
            //we may still receive scan event when user stop scan 
        //but driver has not got the notification yet,
        //because ap and driver run in different threads.
            return;
        }
        
        mLastChannel = ch;
        
        if (mFirstChannelFound) {
            mFirstChannelFound = false;
            mScanThread = null;
            
            mChannelNum = chnum;
            Cursor mCursor = mChannelProvider
                .getCursor(new String[]{ChannelProvider.CHANNEL_NUM,ChannelProvider.CHANNEL_NAME},true);

            XLogUtils.d(TAG, "onScanProgressEvent() getCount() = " + mCursor.getCount());
            if (mCursor.getCount() > 0) {
                
                mCacheList = new ArrayList<ListItem>();
                int no = mCursor.getColumnIndex(ChannelProvider.CHANNEL_NUM);
                int name = mCursor.getColumnIndex(ChannelProvider.CHANNEL_NAME);                
                mCursor.moveToFirst();                        
                do {
                    mCacheList.add(new ListItem(mCursor.getInt(no),mCursor.getString(name)));
                } while (mCursor.moveToNext());                
            }
            mCursor.close();
            //we don't need to init the chip with channel table if it just have done a run of scan.
            setNeedSetTable(false);            
        }
                
        if (chInfo != 0) {
            String name = "Ch " + ch;
            
            if (mCacheList == null) {                        
                    mChannelProvider.insertChannelEntry(ch, chInfo,name);
            }  else {
                int i = 0;
                int size = mCacheList.size();
                for (; i < size; i++) {
                    ListItem item = mCacheList.get(i);
                    if (ch == Integer.parseInt(item.mCh)) {                                
                        //old channel found
                        XLogUtils.d(TAG, "onScanProgressEvent() ch =" + ch + " is in old list " + i);
                        name = item.mName;
                        mChannelProvider.updateChannelEntry(ch, chInfo);
                        break;
                    } 
                }

                if (i == size /*cache list comes to the end and no matching value is found*/) {    
                    //new channel found
                    mChannelProvider.insertChannelEntry(ch, chInfo,name);
                    if (ch > Integer.parseInt(mCacheList.get(i - 1).mCh)) {
                        //we clear the cache list because any channel found afterwards must not in the cache list.
                        mCacheList = null;                    
                    }
                }                
            }
            mList.add((new ListItem(ch,name)));
        }
    }
    /**
     * Start to watch the TV program.
     */
    public void watchOn() {
        XLogUtils.d(TAG, "watchOn() mState = " + mState);

        switch (MTVMODE) {
            case MTV_ATV:
            switch(mState) {
                case MTVENGINE_STATE_IDLE: //it should not be in idle state normally,maybe init fail last time.
                    turnOn();
                    //mState == MTVENGINE_STATE_INITIALIZING after that
                    
                    //fall through
                case MTVENGINE_STATE_INITIALIZING: 
                case MTVENGINE_STATE_INITIALIZED:        
                    mOpenPlayerThread = new Thread(new Runnable() {
                        public void run() {
                            boolean result = false;
                            try {                         
                                result = open();                                                            
                            } catch (InterruptedException e) {
                                releasePlayer();
                                return;
                            } catch (Exception e) {
                                e.printStackTrace();
                                releasePlayer();
                            }
                            mMainHandler.sendMessage(mMainHandler.obtainMessage(MTV_MSG_OPEN_DONE,result ? 1 : 0,
                                0, null));    
                        }
                    });
                    
                    if (mState == MTVENGINE_STATE_INITIALIZING) {
                        mState = MTVENGINE_STATE_INITIALIZING_OPENING;
                    } else {                    
                        mState = MTVENGINE_STATE_OPENING;
                    }                    
                    mOpenPlayerThread.start();
                    break;                            
                case MTVENGINE_STATE_OPENED:    
                    
                    if (mEventCallback != null) {
                        mEventCallback.onEvent(mMainHandler.obtainMessage(MTV_MSG_OPEN_DONE, 1,0,null));
                    }
                    break;
                default:
                    break;                       
                } 
                mRequestTurnOff = false;
                    
                XLogUtils.v(TAG, "watchOn() remove MTV_MSG_RELEASE_ENGINE ");
                mMainHandler.removeMessages(MTV_MSG_RELEASE_ENGINE);
            break;
        default:
                /*should never go here*/
                myAssert(false);
                break;
        }
    }

    /**
     * Turn on ATV device. It will get the service first and start a thread to excuse turn on.
     */
    public void turnOn() {
        XLogUtils.d(TAG, "turnOn() mState = " + mState);
        if (MTVMODE == MTV_ATV) {
                switch(mState) {
                    case MTVENGINE_STATE_IDLE://it means that this is the first time creation.
                        if (mEventCallback != null) {
                            mEventCallback.onEvent(mMainHandler.obtainMessage(MTV_MSG_NOTIFY_POWER_ON, 0,0, null));
                        }
                        
                        mService = new AtvService(this); 
                        mTurnOnThread =    new Thread(new Runnable() {
                            public void run() {
                                boolean result = false;
                                try {                         
                                    result = initialize();                                                            
                                } catch (InterruptedException e) {
                                    return;
                                } catch (Exception e) {
                                    e.printStackTrace();
                                }
                                mMainHandler.sendMessage(mMainHandler.obtainMessage(MTV_MSG_INITIALIZE_DONE,result ? 1 : 0,
                                    0, null));    
                            }                                        
                        });
                        mTurnOnThread.start();            
                                        
                        //mChannelManager must be initialized at the construction of engine,
                        //and other part of code will assume mChannelManager is ready when engine is available.
                        mState = MTVENGINE_STATE_INITIALIZING;
                        break;
                    case MTVENGINE_STATE_INITIALIZED:
                    case MTVENGINE_STATE_OPENING:                        
                        if (mEventCallback != null) {
                            mEventCallback.onEvent(mMainHandler.obtainMessage(MTV_MSG_INITIALIZE_DONE, 1,0,null));
                        }
                        break;

                    case MTVENGINE_STATE_INITIALIZING:
                    case MTVENGINE_STATE_INITIALIZING_OPENING:
                    case MTVENGINE_STATE_INITIALIZING_OPENED:                        
                        break;
                    case MTVENGINE_STATE_SCANNING:
                       //activity may be recreated due to configuration change 
                    //during scanning and call turnOn again.
                        
                        break;
                    default:
                        //should not go here.
                        myAssert(false);
                        break;
                }             
            } else {
                //Not support yet.
                myAssert(false);                
            }        
        mRequestTurnOff = false;
        
        XLogUtils.v(TAG, "turnOn() remove MTV_MSG_RELEASE_ENGINE ");
        mMainHandler.removeMessages(MTV_MSG_RELEASE_ENGINE);
    }

    /**
     * Initialize the ATV device.
     * @return Init success or not.
     * @throws Exception
     */
    private boolean initialize() throws Exception {
        XLogUtils.d(TAG, "initialize()");

        int count = 10;
        while ((count-- > 0) && (!Thread.interrupted())) { 
            try {
                mService.init();
                //exit once succeeded.
                return true;                
            } catch (IOException e) {
                Thread.sleep(500);//wait some time in order to avoid the conflict with camera.
            }
        }
        return false;                        
    }

    /**
     * Set event call back, MtvEngine will use the callback to send MSG to activity.
     * @param cb Callback function in activity.
     */
    public final void setEventCallback(EventCallback cb) {
        XLogUtils.d(TAG, "setEventCallback() cb = " + cb);
        
        mEventCallback = cb;
    }
     
    
    /**
     * Other classes can use it send MSG to MtvEngine.
     * @param what  Value to assign to the returned Message.what field.
     * @param arg1   Value to assign to the returned Message.arg1 field.
     * @param arg2   Value to assign to the returned Message.arg2 field.
     * @param obj   Value to assign to the terurned Message.arg2 field.
     */
    public void callOnEvent(int what, int arg1, int arg2, Object obj) {

        XLogUtils.d(TAG, "callOnEvent()");
        mMainHandler.sendMessage(mMainHandler.obtainMessage(what, arg1,arg2, obj));
    }
    /**
     * Set flag the channel table is changed or not.
     * @param set Channel table is changed or not.
     */
    public void setNeedSetTable(boolean set) {
        
        XLogUtils.d(TAG, "setNeedSetTable() set = " + set);
        mNeedSetTable = set;
    }  

    /**
     * When choose a channel to watch in ChanneList activity,
     * it will be call to set this channel to atv device.
     * @param ch The channel want to watch.
     * @param holder Channel list use in driver.
     * @throws ChannelManager.ChannelTableEmpty
     */
    public void setFirstChannel(int ch,ChannelManager.ChannelHolder holder) 
                                      throws ChannelManager.ChannelTableEmpty {
        XLogUtils.d(TAG, "setFirstChannel,ch = " + ch);
        if (mChannelManager == null) {
            XLogUtils.d(TAG, "Enter setFirstChannel mChannelManager = " + mChannelManager);
            try {
                int sleepCount = 0;
                while (mChannelManager == null) {
                    if (sleepCount == 5) {
                        return;
                    }
                    Thread.sleep(SHADE_TIME);
                    sleepCount++;
                    XLogUtils.d(TAG, "Wait for initialize done, sleepCount = " + sleepCount
                            + ", mChannelManager = "
                            + mChannelManager);
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        XLogUtils.d(TAG, "TestMatv:: mChannelManager = " + mChannelManager);
        if (mNeedSetTable) {                
            mChannelManager.initChannelTable(holder);
            mNeedSetTable = false;
        }           
        mChannelManager.changeChannel(ch);
    }
    
    /**
     * Change TV program's channel.
     * @param ch  The channel want to watch.
     */
    public void changeChannel(int ch) {
        XLogUtils.d(TAG, "changeChannel,ch = " + ch);
        if (mState == MTVENGINE_STATE_OPENED) {
            mChannelManager.changeChannel(ch);
            try {
                Thread.sleep(SHADE_TIME);
            } catch (InterruptedException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }

        }
    }
    
    /**
     *Use to send msg from MtvEngine to activity(ChannelList or MtvPlayer).
     */
    public interface EventCallback {
        /*this is called in main thread*/
        void onEvent(Message msg);
    }

    /**
     * Start record TV program.
     * @param path   File's patch.
     * @param maxFileSize  Max file size.
     * @param s  Preview surface.
     * @throws Exception
     */
    public void startRecord(String path,long maxFileSize,Surface s) throws Exception {
        XLogUtils.d(TAG, "startRecord() path = " + path + ";maxFileSize = " + maxFileSize);
        if (MTVMODE == MTV_ATV) {
            if (mRecorder == null) {
                mRecorder = new AtvRecorder(this,mService);
            }
            mRecorder.start(MediaRecorder.OutputFormat.DEFAULT,path,maxFileSize,s
                ,RECORDER_SPEC,320,240);    
        }
    }

    /**
     * Stop record ATV program.
     */
    public void stopRecord() {
    XLogUtils.d(TAG, "stopRecord()");
        mRecorder.stop();
    }

    /**
     * Release Recorder.
     */
    public void releaseRecorder() {
        XLogUtils.d(TAG, "releaseRecorder()");
        if (mRecorder != null) {
            mRecorder.release();
            mRecorder = null;
        }
    }    

    /**
     * Release player.
     * we need to do synchronzation in MtvEngine since AtvPlayer 
     * do not have synchronization on its own.
     */
    public synchronized void releasePlayer() {        
        if (mPlayer != null) {        
            mPlayer.release();
            mPlayer = null;    
        }
    }    

    /**
     * Open atv device start preview.
     * @return Success or not.
     * @throws Exception
     */
    private synchronized boolean open() throws  Exception {
        XLogUtils.d(TAG, "open()");
        if (mPlayer == null) {
            mPlayer = new AtvPlayer(this,mService);    
        }
        int count = 10;
        while ((count-- > 0) && (!Thread.interrupted())) { 
            try {
                mPlayer.open(320,240);//only support 320*240 by chip,however we will resize it before displaying.  
                //exit once succeeded.
                return true;                
            } catch (AtvPlayer.RetriableException e) {
                Thread.sleep(500);//wait some time in order to avoid the conflict with camera.
            }
        }
        return false;
    }

    /**
     * Set the preview surface.
     * @param s  Preview surface.
     * @throws IOException
     */
    public synchronized void setSurface(Surface s) throws IOException {
        XLogUtils.d(TAG, "setSurface()");
        if (mPlayer != null) {
            mPlayer.start(s);
        }
    }

    /**
     * Set TV program mute or not.
     * @param mute  Mute or not.
     */
    public synchronized void setMute(boolean mute) {
        XLogUtils.d(TAG, "setMute() mute = " + mute);
        if (mPlayer != null) {
            mPlayer.setMute(mute);
        }
    }

    /**
     * Turn off TV.
     */
    public void watchOff() {
        XLogUtils.d(TAG, "watchOff() mState = " + mState);
        switch(mState) {
            case MTVENGINE_STATE_OPENED:
                releasePlayer();
                mState = MTVENGINE_STATE_INITIALIZED;
                break;
            case MTVENGINE_STATE_OPENING:
                stopOpengingPlayer();
                mState = MTVENGINE_STATE_INITIALIZED;
                break;                    
            case MTVENGINE_STATE_INITIALIZING_OPENING:
                stopOpengingPlayer();
                mState = MTVENGINE_STATE_INITIALIZING;
                break;    
            case MTVENGINE_STATE_INITIALIZING_OPENED:                
                mState = MTVENGINE_STATE_INITIALIZING;
                break;                
            default:
                //To enter camera we need to allow WatchOff to be called in any state.so just ignore here.
                break;
        }                        
    }

    /**
     * Release recorder and player.
     */
    private void releaseVideo() {
        XLogUtils.d(TAG, "releaseVideo()");
        //release recorder first and release player later,
        //not quite sure whether the reverse order will cause lock/unlock failure.
        releaseRecorder();        
        releasePlayer();        
    }      

    /**
     * Do snap when preview.
     */
    public void capture() {

        XLogUtils.d(TAG, "capture()");
        mService.capture(); 
    }       

    /**
     * It will be called when open success.
     */
    private void openTrue() {    
        
        XLogUtils.d(TAG, "openTrue() mState = " + mState);
        switch(mState) {
            case MTVENGINE_STATE_OPENING:                
                mState = MTVENGINE_STATE_OPENED;
                break;
            case MTVENGINE_STATE_INITIALIZING_OPENING:
                mState = MTVENGINE_STATE_INITIALIZING_OPENED;
                break;                    
            default:
                //ingore asnchronous events because it may already in queue when state is changed.
                return;
        }                        
    }

    /**
     * It will be called when open fail.
     */
    private void openFalse() {    
        
        XLogUtils.d(TAG, "openFalse() mState = " + mState);
        switch(mState) {
            case MTVENGINE_STATE_OPENING:
                mState = MTVENGINE_STATE_INITIALIZED;
                break;
            case MTVENGINE_STATE_INITIALIZING_OPENING:
                mState = MTVENGINE_STATE_INITIALIZING;
                break;                
            default:
                //ingore asnchronous events because it may already in queue when state is changed.
                return;
        }                
    }    

    /**
     * It will be called when initialize success.
     */
    private void initializeTrue() {    
        
        XLogUtils.d(TAG, "initializeTrue() mState = " + mState);
        switch(mState) {
            case MTVENGINE_STATE_INITIALIZING:                
                mState = MTVENGINE_STATE_INITIALIZED;
                break;
            case MTVENGINE_STATE_INITIALIZING_OPENING:
                mState = MTVENGINE_STATE_OPENING;
                break;                
            case MTVENGINE_STATE_INITIALIZING_OPENED:
                mState = MTVENGINE_STATE_OPENED;
                break;    
            default:
                //ingore asnchronous events because they may already be put in queue when state is changed.
                return;
        }        
        mChannelManager = new AtvChannelManager(mService);
        mNeedSetTable = true;
        if (mRequestTurnOff) {
            turnOff(false);
            return;
        } 
    }

    /**
     * It will be called when initialize fail.
     */
    private void initializeFalse() {    
        
        XLogUtils.d(TAG, "initializeFalse() mState = " + mState);
        switch(mState) {
            case MTVENGINE_STATE_INITIALIZING_OPENING:    
            case MTVENGINE_STATE_OPENING:                                
                stopOpengingPlayer();                
                break;                
            default:
                break;
        }                
        internalTurnOff();
    }    

    /**
     * Now is ready for record or not.
     * @return  Ready or not.
     */
    public boolean isRecordReady() {
        return mState == MTVENGINE_STATE_OPENED ? true : false;
    }    

    /**
     * Now is scan or not.
     * @return  Is scanning or not.
     */
    public boolean isScanning() {
        return mState == MTVENGINE_STATE_SCANNING ? true : false;
    }

    /**
     * Get signal strength.
     * @return Signal strength value.
     */
    public int getSignalStrengh() {
        XLogUtils.d(TAG, "getSignalStrengh()");
        
        return mChannelManager.getSignalStrengh();
    }    

    /**
     * Set the channel list of the selected country.
     * @param list  Channel list of the selected country.
     */
    public void setChannelList(ArrayList<ListItem> list) {
        mList = list;
    }

    /**
     * Set the channel provider instance.
     * @param cp Provider instance.
     */
    public void setChannelProvider(ChannelProvider cp) {
        mChannelProvider = cp;
    }

    /**
     * Get the channel list of selected country.
     * @return Channel list of selected country.
     */
    public Object getChannelList() {
        return mList;
    }    

    /**
     * Set channel's number playing now.
     * @param ch  Current's number.
     */
    public void setCurrentChannel(int ch) {
        mCurrentChannel = ch;
    }

    /**
     * Get channel playing now.
     * @return  Current channel's number.
     */
    public int getCurrentChannel() {
        return mCurrentChannel;
    }    

    /**
     * Config video setting.
     * @param item  Config item.
     * @param val  Item's value.
     */
    public void configVideo(byte item,int val) {
        XLogUtils.d(TAG, "configVideo() item = " + item + ";val = " + val);
        if (mState == MTVENGINE_STATE_INITIALIZED || mState == MTVENGINE_STATE_OPENING || mState == MTVENGINE_STATE_OPENED) {
            mService.adjustSetting(item, val);
        }
    }

    /**
     * Start scan channel.
     * @param area  Selected country.
     * @return  Success or not.
     */

    public boolean channelScan(final int area) {
        XLogUtils.d(TAG, "channelScan() area = " + area);
        if (mState == MTVENGINE_STATE_INITIALIZED) {
                mFirstChannelFound = true;                
                mCacheList = null;
                mLastChannel = 0;
                mChannelNum = 0;    
                mScanThread = new Thread(new Runnable() {
                    public void run() {                            
                        mChannelManager.channelScan(ATV_AUTOSCAN_MODE,area);
                    }
                });
                mScanThread.start();                    
                mState = MTVENGINE_STATE_SCANNING;
                return true;
        } else {
                XLogUtils.d(TAG, "channelScan() error mState = " + mState);
                return false;
        }
    } 

    /**
     * Stop channel scan process.
     */
    public void stopChannelScan() {
        XLogUtils.d(TAG, "stopChannelScan()");
        if (MTVMODE == MTV_ATV) {
            if (mState != MTVENGINE_STATE_SCANNING) {
                XLogUtils.w(TAG, "stopChannelScan() error mState = " + mState);
                return;
            }
            
            if (mScanThread != null) {
            //means that it is force closed last time but has not closed yet,
            //we need to wait to avoid corrupt the state of driver.
                try {
                    mScanThread.join();
                } catch (InterruptedException ex) {
                        // ignore
                  XLogUtils.d(TAG, "wait scan thread error.");
                }
            }
            mScanThread = null;
            mChannelManager.stopChannelScan();
            mCacheList = null;
            mState = MTVENGINE_STATE_INITIALIZED;
        }
    }     

    /**
     * Set selected audio format.
     * @param val Selected audio format.
     */
    public void setAudioFormat(int val) {
        XLogUtils.d(TAG, "setAudioFormat() val = " + val);
        if (mState == MTVENGINE_STATE_OPENED) {
            mService.setAudioFormat(val);
        }
    }    
    
    /**
     *  Get audio format the country supported.
     * @return  Support audio format.
     */
    public int getAudioFormat() {
        XLogUtils.d(TAG, "getAudioFormat()");
        
        return mService.getAudioFormat();
    }     
       
    /**
     * Not used.
     * @param set  Not used.
     */
    public void setAudioCallback(boolean set) {
        //marked for not use
        //XLogUtils.d(TAG, "setAudioCallback() set = "+set);
        //mService.setAudioCallback(set);
    }    

    /**
     * Release the opened player.
     */
    private void stopOpengingPlayer() {
        mOpenPlayerThread.interrupt();
        try {
            mOpenPlayerThread.join();
        } catch (InterruptedException ex) {
                // ignore
            XLogUtils.d(TAG, "wait OpenPlayThread error.");
        } finally {
            mOpenPlayerThread = null;
            mMainHandler.removeMessages(MTV_MSG_OPEN_DONE);            
        }
        releasePlayer();                
    }

    /**
     * When something wrong, force stop initialize.
     */
    private void forceStopInitializing() {
        mTurnOnThread.interrupt();
        try {
            mTurnOnThread.join();
        } catch (InterruptedException ex) {
            // ignore
            XLogUtils.d(TAG, "wait TurnOnThread error.");
        } finally {
            mTurnOnThread = null;
            internalTurnOff();
        }
    }

    /**
     * To turn off ATV chip.
     * driver may occasionally be shutdown due to
     * 1.camera application is started up.
     * 2.media server died
     */
    private void onDriverShutdown() {
        XLogUtils.d(TAG, "onDriverShutdown() mState = " + mState);
        
        switch(mState) {    
            case MTVENGINE_STATE_IDLE:
                return;    

            case MTVENGINE_STATE_OPENED:
            case MTVENGINE_STATE_OPENING:                    
            case MTVENGINE_STATE_INITIALIZING_OPENING:
            case MTVENGINE_STATE_INITIALIZING_OPENED:
                watchOff();
                //fall through
            case MTVENGINE_STATE_SCANNING:    
                //fall through
            case MTVENGINE_STATE_INITIALIZED:
                //video may be stopped but not released yet and we need to 
            //release it immediately to let user to enter camera related functions normally.
                break;                
            case MTVENGINE_STATE_INITIALIZING:    
                //HP said driver will wait initialization done before shutting down in driver.
                //so we don't need to wait initialization thread died because it must already died.
                break;                

            default:
                //should not go here.
                myAssert(false);
                break;            
                
        }    
        
        internalTurnOff();
    }    

    /**
     * When media server died or these is a quest to turn off atv.
     * @param force  Need turn off right now or not.
     */
    public void turnOff(boolean force) {
        XLogUtils.d(TAG, "turnOff() mState = " + mState);
        
        switch(mState) {            
            case MTVENGINE_STATE_INITIALIZING_OPENING:            
            case MTVENGINE_STATE_INITIALIZING_OPENED:

                watchOff();
                /*mState == MTVENGINE_STATE_INITIALIZING after that*/        
                
                //fall through
            case MTVENGINE_STATE_INITIALIZING:        
                if (force) {
                    forceStopInitializing();    
                } else {                
                    //we need to wait util INITIALIZED state is reached.
                    mRequestTurnOff = true;
                }                
                break;
                
            case MTVENGINE_STATE_IDLE:
            case MTVENGINE_STATE_SCANNING:
                //at present we assume turnOff(true) is caused by pressing power key,
                //and we don't want to abort scanning even in such case.
                //Maybe assert here is better?because MMI should take care of shutdown request during scanning.
                break;
                
            case MTVENGINE_STATE_OPENING:    
            case MTVENGINE_STATE_OPENED:
                watchOff();
                /*mState == MTVENGINE_STATE_INITIALIZED after that*/                
                
                //fall through
            case MTVENGINE_STATE_INITIALIZED:
                if (force) {
                    internalTurnOff();
                } else {
                    //video may be stopped but not released yet and we need to 
                    //release it immediately to let user to enter camera related functions normally.
                    releasePlayer();                    
                    XLogUtils.d(TAG, "Start timer to delay shutdown()");
                    mMainHandler.sendEmptyMessageDelayed(MTV_MSG_RELEASE_ENGINE,
                            DEFAULT_ALIVE_TIME);
                }
                break;
                
            default:
                //should not go here.
                myAssert(false);
                
        }        
    }

    /**
     * when MtvEngine run incorrectly, it will be called.
     * @param noassert is it run correctly or not.
     */
    private void myAssert(boolean noassert) {
        if (!noassert) {
            throw new RuntimeException(TAG + " assertion failed!");
        }
    }


    /**
     * The function will be called when app want to shutdown atv or something is wrong.
     */
    private void internalTurnOff() {
        XLogUtils.d(TAG, "internalTurnOff() mState = " + mState);
        
        shutdownEngine();
        resetEngine();
    }       

    /**
     * Turn off atv engine.
     */
    private void shutdownEngine() {
        //video is closed but may not be released.
        releaseVideo();
        mService.shutdown(true);
    }

    /**
     * Reset atv engine.
     */
    private void resetEngine() {
        mMainHandler.removeMessages(MTV_MSG_RELEASE_ENGINE);
        mMainHandler.removeMessages(MTV_MSG_INITIALIZE_DONE);
        mMainHandler.removeMessages(MTV_MSG_OPEN_DONE);            
        mNeedSetTable = true;
        mService = null;
        mChannelManager = null;

        mRequestTurnOff = false;
        mState = MTVENGINE_STATE_IDLE;
    }        
}
