package com.mediatek.hotknotbeam;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.net.Uri.Builder;
import android.os.Binder;
import android.os.IBinder;
import android.os.UserHandle;
import android.os.ServiceManager;
import android.os.Handler;
import android.os.Message;
import android.os.Parcelable;
import android.widget.Toast;
import android.util.Log;


import com.mediatek.hotknot.HotKnotAdapter;
import com.mediatek.hotknotbeam.DownloadNotifier;
import com.mediatek.hotknotbeam.DownloadInfo;
import com.mediatek.hotknotbeam.FileUploadTask;
import com.mediatek.hotknotbeam.FileUploadTask.FileUploadTaskListener;
import com.mediatek.hotknotbeam.HotKnotBeamConstants;
import com.mediatek.hotknotbeam.HotKnotFileServer;
import com.mediatek.hotknotbeam.HotKnotFileServer.HotKnotFileServerCb;

import com.mediatek.storage.StorageManagerEx;

import java.io.File;
import java.io.IOException;
import java.util.Collection;
import java.util.LinkedList;

public class HotKnotBeamService extends Service implements Handler.Callback {
    static protected final String TAG = "HotKnotBeamService";

    static final String TEST_INTENT = "com.mediatek.hotknot.upload";
    static final String SERVER_INTENT = "com.mediatek.hotknot.server";
    static final String HOTKNOT_BEAMING = "com.mediatek.howknot.beaming";
    static final String HOTKNOT_SEND = "com.mediatek.hotknotbeam.SEND";
    static final String HOTKNOT_RECV = "com.mediatek.hotknotbeam.RECV";
    static final String HOTKNOT_FINISH = "com.mediatek.hotknotbeam.FINISH";
    static final String HOTKNOT_DL_COMPLETE = "com.mediatek.hotknotbeam.DL.COMPLETE";

    static final String HOTKNOT_EXTRA_BEAM_ID = "com.mediatek.howknot.beam.id";
    static final String HOTKNOT_EXTRA_BEAM_IP = "ip";
    static final String HOTKNOT_EXTRA_BEAM_URIS = "uris";
    static final String HOTKNOT_EXTRA_BEAM_PATH = "path";


    static protected final int SERVICE_PORT = HotKnotBeamConstants.SERVICE_PORT;

    static private final int MSG_SENDER_REQ      = 0;
    static private final int MSG_RECEIVER_REQ    = 1;
    static private final int MSG_POLLING         = 2;
    static private final int MSG_CANCEL_REQ      = 3;
    static private final int MSG_CLIENT_DONE     = 4;
    static private final int MSG_CLIENT_END      = 5;
    static private final int MSG_SERVER_END      = 6;

    static private final int MSG_TEST_CLIENT_REQ      = 0x10;

    private Handler mHandler;
    private Context mContext;
    private HotKnotFileServer mHotKnotServer = null;
    private DownloadNotifier mDownloadNotifier = null;
    private Object mPollLock = new Object();
    private Object mServerLock = new Object();
    private String mServerIP = "127.0.0.1";
    static private int mIdleCounter = 0;

    // LinkList to queue the download request
    private LinkedList<Uri> mUploadList = new LinkedList<Uri>();

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate");

        IntentFilter filter = new IntentFilter();
        filter.addAction(HotKnotAdapter.ACTION_ADAPTER_STATE_CHANGED);
        filter.addAction(TEST_INTENT);
        filter.addAction(SERVER_INTENT);
        filter.addAction(HOTKNOT_BEAMING);
        filter.addAction(HOTKNOT_SEND);
        filter.addAction(HOTKNOT_RECV);
        registerReceiver(mReceiver, filter);

        mHandler = new Handler(this);
        mContext = this;

        mDownloadNotifier = new DownloadNotifier(this);

        Toast.makeText(this, "service is started", Toast.LENGTH_SHORT).show();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mReceiver);
        Toast.makeText(this, "service is stopped", Toast.LENGTH_SHORT).show();
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Message msg = Message.obtain();

            String action = intent.getAction();
            Log.d(TAG, "action:" + action);

            if (HotKnotAdapter.ACTION_ADAPTER_STATE_CHANGED.equals(action)) {

            } else if (TEST_INTENT.equals(action)) {
                msg.what = MSG_TEST_CLIENT_REQ;
                msg.obj = intent.getStringExtra(HOTKNOT_EXTRA_BEAM_PATH);
                if(msg.obj != null) {
                    Log.d(TAG, "Test Path:" + msg.obj);
                }
                mHandler.sendMessage(msg);
            } else if(SERVER_INTENT.equals(action)) {
                mHandler.sendEmptyMessage(MSG_RECEIVER_REQ);
            } else if(HOTKNOT_BEAMING.equals(action)) {
                msg.what = MSG_CANCEL_REQ;
                msg.arg1  = intent.getIntExtra(HOTKNOT_EXTRA_BEAM_ID, 0);
                mHandler.sendMessage(msg);
            } else if(HOTKNOT_SEND.equals(action)) {
                String ipAddress = intent.getStringExtra(HOTKNOT_EXTRA_BEAM_IP);
                if(ipAddress != null && !ipAddress.isEmpty()) {
                    mServerIP = ipAddress;
                    Log.d(TAG, "Server address:" + mServerIP);
                } else {
                    Log.d(TAG, "Server address:" + mServerIP);
                }

                final Parcelable[] rawUris = intent.getParcelableArrayExtra(HOTKNOT_EXTRA_BEAM_URIS);
                final int uriCount = rawUris != null ? rawUris.length : 0;
                if(uriCount == 0) {
                    Log.e(TAG, "No Uris");
                    return;
                }
                Uri[] uris = new Uri[uriCount];
                for(int i = 0; i < uriCount; i++) {
                    uris[i] = (Uri) rawUris[i];
                }
                msg.what = MSG_SENDER_REQ;
                msg.obj = (Object) uris;
                mHandler.sendMessage(msg);
            } else if(HOTKNOT_RECV.equals(action)) {
                mHandler.sendEmptyMessage(MSG_RECEIVER_REQ);
            }
        }
    };

    @Override
    public boolean handleMessage(Message msg) {

        if(msg.what != MSG_POLLING) {
            Log.d(TAG, "msg.what:" + msg.what);
        }

        switch(msg.what) {
        case MSG_SENDER_REQ:
            synchronized (mUploadList) {
                Uri[] uris = (Uri[]) msg.obj;
                if(uris != null) {
                    String queryString = uris[0].getQuery();                    
                    if(queryString != null && queryString.indexOf(HotKnotBeamConstants.QUERY_ZIP + "=" + HotKnotBeamConstants.QUERY_VALUE_YES) != -1){                        
                        try{
                            File tmpFile = new File(StorageManagerEx.getDefaultPath() + File.separator + HotKnotBeamConstants.MAX_HOTKNOT_BEAM_TEMP_ZIP);                            
                            File baseFile = MimeUtilsEx.getFilePathFromUri(uris[0], mContext);
                            String zipeName = ZipFileUtils.zipUris(uris, tmpFile, baseFile.getParentFile(), mContext);
                            Uri.Builder zipUri = Uri.fromFile(tmpFile).buildUpon().appendQueryParameter(HotKnotBeamConstants.QUERY_ZIP, HotKnotBeamConstants.QUERY_VALUE_YES);
                            runUploadTask(zipUri.build(), zipeName);
                        }catch(IOException e){
                            e.printStackTrace();
                        }catch(Exception ee){
                            ee.printStackTrace();
                        }
                    }else{
                        for(int i = 0; i < uris.length; i++){
                            mUploadList.add(uris[i]);
                            runUploadTask(uris[i], null);
                        }
                    }
                } else {
                    mUploadList.add(Uri.parse("file://mnt/sdcard/testimg.jpg"));
                    runUploadTask(Uri.parse("file://mnt/sdcard/testimg.jpg"), null);
                }
            }
            break;
        case MSG_RECEIVER_REQ:
            mIdleCounter = 0;

            if(mHotKnotServer == null) {
                mHotKnotServer = new HotKnotFileServer(SERVICE_PORT, this);
                try {
                    mHotKnotServer.setHotKnotFileServerCb(new HotKnotFileServerCb() {
                        public void onHotKnotFileServerFinish(int status) {
                            updateNotification();
                            mHandler.sendEmptyMessage(MSG_SERVER_END);
                        }
                    });
                    mHotKnotServer.execute();
                } catch(Exception e) {
                    e.printStackTrace();
                    mHotKnotServer = null;
                }
            }

            synchronized (mPollLock) {
                if(!mHandler.hasMessages(MSG_POLLING)) {
                    mHandler.sendEmptyMessageDelayed(MSG_POLLING, HotKnotBeamConstants.FILE_PROGRESS_POLL);
                }
            }
            break;
        case MSG_POLLING:
            Log.d(TAG, "Polling msg");

            if(!updateNotification()) {
                mIdleCounter++;

                if(mIdleCounter > HotKnotBeamConstants.MAX_IDLE_COUNTER) {
                    Log.d(TAG, "There is no incoming client; stop server");
                    updateNotification();
                    if(!mHandler.hasMessages(MSG_SERVER_END)) {

                        if(mHotKnotServer != null) {
                            mHotKnotServer.stop();
                        }

                        mHandler.sendEmptyMessage(MSG_SERVER_END);
                    }
                    return true;
                }
            } else {
                mIdleCounter = 0;
            }

            synchronized (mPollLock) {
                if(!mHandler.hasMessages(MSG_POLLING)) {
                    mHandler.sendEmptyMessageDelayed(MSG_POLLING, HotKnotBeamConstants.FILE_PROGRESS_POLL);
                }
            }
            break;
        case MSG_CANCEL_REQ:
            int id = (int) msg.arg1;
            Log.d(TAG, "Cancel dowdload job:" + id);
            if(id != 0 && mHotKnotServer != null) {
                mHotKnotServer.cancel(id);
            }
            break;
        case MSG_CLIENT_DONE:
            synchronized (mUploadList) {
                Uri uri = (Uri) msg.obj;
                mUploadList.remove(uri);

                if(mUploadList.size() == 0) {
                    Log.d(TAG, "Send finish intent in client size");
                    uri = null;
                    runUploadTask(uri, null);
                }
            }
            break;
        case MSG_CLIENT_END:
            Log.d(TAG, "Finish client procedure");
            if(mHotKnotServer == null) {
                sendGenericIntent(HOTKNOT_FINISH);
            }
            break;
        case MSG_SERVER_END:
            if(mHandler.hasMessages(MSG_POLLING)) {
                mHandler.removeMessages(MSG_POLLING);
            }
            mHotKnotServer = null;

            synchronized (mUploadList) {
                boolean isClientStopped = (mUploadList.size() == 0) ? true : false;
                if(isClientStopped) {
                    sendGenericIntent(HOTKNOT_FINISH);
                }
            }
            break;
        case MSG_TEST_CLIENT_REQ:
            Intent intent = new Intent(HOTKNOT_SEND);
            intent.addFlags(Intent.FLAG_RECEIVER_REPLACE_PENDING |
                            Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
            Uri[] uris = new Uri[1];
            String path = (String) msg.obj;
            if(path ==  null) {
               /*                
               uris = new Uri[10];
               uris[0] = Uri.parse("file://mnt/sdcard/dog/IMG_20100102_012616_9CS.jpg?zip=yes");
               uris[1] = Uri.parse("file://mnt/sdcard/dog/IMG_20100102_012616_8CS.jpg");
               uris[2] = Uri.parse("file://mnt/sdcard/dog/IMG_20100102_012616_7CS.jpg");
               uris[3] = Uri.parse("file://mnt/sdcard/dog/IMG_20100102_012616_6CS.jpg");
               uris[4] = Uri.parse("file://mnt/sdcard/dog/IMG_20100102_012616_10CS.jpg");
               uris[5] = Uri.parse("file://mnt/sdcard/dog/IMG_20100102_012615_5CS.jpg");
               uris[6] = Uri.parse("file://mnt/sdcard/dog/IMG_20100102_012615_4CS.jpg");
               uris[7] = Uri.parse("file://mnt/sdcard/dog/IMG_20100102_012615_3CS.jpg");
               uris[8] = Uri.parse("file://mnt/sdcard/dog/IMG_20100102_012615_2CS.jpg");
               uris[9] = Uri.parse("file://mnt/sdcard/dog/IMG_20100102_012615_1CS.jpg");
               */
               uris = new Uri[14];
               uris[0] = Uri.parse("file://mnt/sdcard/demo_material/IMG_20131108_112533MT.jpg?zip=yes");               
               uris[1] = Uri.parse("file://mnt/sdcard/demo_material/.ConShots/IMG_20131108_112533MT/IMG_20131108_112533MT01.jpg");
               uris[2] = Uri.parse("file://mnt/sdcard/demo_material/.ConShots/IMG_20131108_112533MT/IMG_20131108_112533MT02.jpg");
               uris[3] = Uri.parse("file://mnt/sdcard/demo_material/.ConShots/IMG_20131108_112533MT/IMG_20131108_112533MT03.jpg");
               uris[4] = Uri.parse("file://mnt/sdcard/demo_material/.ConShots/IMG_20131108_112533MT/IMG_20131108_112533MT04.jpg");
               uris[5] = Uri.parse("file://mnt/sdcard/demo_material/.ConShots/IMG_20131108_112533MT/IMG_20131108_112533MT05.jpg");
               uris[6] = Uri.parse("file://mnt/sdcard/demo_material/.ConShots/IMG_20131108_112533MT/IMG_20131108_112533MT06.jpg");
               uris[7]  = Uri.parse("file://mnt/sdcard/demo_material/.ConShots/IMG_20131108_112533MTTK/IMG_20131108_112533MTTK01.jpg");
               uris[8]  = Uri.parse("file://mnt/sdcard/demo_material/.ConShots/IMG_20131108_112533MTTK/IMG_20131108_112533MTTK02.jpg");
               uris[9] = Uri.parse("file://mnt/sdcard/demo_material/.ConShots/IMG_20131108_112533MTTK/IMG_20131108_112533MTTK03.jpg");
               uris[10] = Uri.parse("file://mnt/sdcard/demo_material/.ConShots/IMG_20131108_112533MTTK/IMG_20131108_112533MTTK04.jpg");
               uris[11] = Uri.parse("file://mnt/sdcard/demo_material/.ConShots/IMG_20131108_112533MTTK/IMG_20131108_112533MTTK05.jpg");
               uris[12] = Uri.parse("file://mnt/sdcard/demo_material/.ConShots/IMG_20131108_112533MTTK/IMG_20131108_112533MTTK06.jpg");
               uris[13] = Uri.parse("file://mnt/sdcard/demo_material/.ConShots/InterMedia/IMG_20131108_112533MTIT");
               Log.i(TAG, "configure zip uris");
            } else if(path.indexOf("?") == -1) {
                uris[0] = Uri.parse("file:/" + path);
                Uri.Builder builder = uris[0].buildUpon();
                uris[0] = builder.appendQueryParameter(HotKnotBeamConstants.QUERY_NUM, "1").appendQueryParameter(HotKnotBeamConstants.QUERY_FORMAT, "no").appendQueryParameter(HotKnotBeamConstants.QUERY_FOLDER, "test/loc").appendQueryParameter(HotKnotBeamConstants.QUERY_SHOW, "no").build();
            } else {
                uris[0] = Uri.parse("file:/" + path);
            }

            intent.putExtra(HOTKNOT_EXTRA_BEAM_URIS, uris);
            final long ident = Binder.clearCallingIdentity();
            try {
                mContext.sendStickyBroadcastAsUser(intent, UserHandle.ALL);
            } finally {
                Binder.restoreCallingIdentity(ident);
            }
            break;
        default:

            break;
        }
        return true;
    }

    private void sendGenericIntent(String action) {
        Intent intent = new Intent(action);
        intent.addFlags(Intent.FLAG_RECEIVER_REPLACE_PENDING |
                        Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);

        final long ident = Binder.clearCallingIdentity();

        try {
            mContext.sendStickyBroadcastAsUser(intent, UserHandle.ALL);
        } finally {
            Binder.restoreCallingIdentity(ident);
        }
    }

    private boolean updateNotification() {

        if(mHotKnotServer == null) {
            Log.e(TAG, "mHotKnotServer is null");
            return false;
        }

        Collection<DownloadInfo> c = mHotKnotServer.getDownloadInfos();

        if(c != null) {
            mDownloadNotifier.updateWith(c);
            return true;
        }

        return false;
    }

    private void runUploadTask(Uri uri, String filename) {
        FileUploadTask uploadTask = new FileUploadTask(mServerIP, SERVICE_PORT, this);

        if(filename != null){
            uploadTask.setUploadFileName(filename);
        }

        uploadTask.setOnPostExecute(new FileUploadTaskListener() {
            public void onPostExecute(Void result, Uri uri) {
                Message msg = Message.obtain();
                msg.what = (uri != null) ? MSG_CLIENT_DONE : MSG_CLIENT_END;
                msg.obj = (Object) uri;
                mHandler.sendMessage(msg);
            }
        });

        uploadTask.execute(uri);
    }
}