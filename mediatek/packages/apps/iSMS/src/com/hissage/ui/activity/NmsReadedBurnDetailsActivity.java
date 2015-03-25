package com.hissage.ui.activity;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Hashtable;
import java.util.List;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.DisplayMetrics;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.hissage.R;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.api.NmsiSMSApi;
import com.hissage.config.NmsBitmapUtils;
import com.hissage.config.NmsCommonUtils;
import com.hissage.contact.NmsContact;
import com.hissage.message.ip.NmsHesineApiConsts.NmsImFlag;
import com.hissage.message.ip.NmsHesineApiConsts.NmsImReadMode;
import com.hissage.message.ip.NmsIpAttachMessage;
import com.hissage.message.ip.NmsIpImageMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpMessageConsts.NmsIpMessageStatus;
import com.hissage.message.ip.NmsIpMessageConsts.NmsIpMessageType;
import com.hissage.message.ip.NmsIpSessionMessage;
import com.hissage.service.NmsService;
import com.hissage.struct.SNmsImMsgCountInfo;
import com.hissage.ui.activity.NmsAllMediaDetailsActivity.MediaType;
import com.hissage.util.log.NmsLog;

public class NmsReadedBurnDetailsActivity extends Activity {

    private final static int HANDLER_DATA_ERROR = 1000;
    private final static int HANDLER_DATA_EMPTY = 1001;
    private final static int HANDLER_DATA_READY = 1002;

    private final static String TAG = "AllReadedBurnActivity";
    private int type = 0;
    private int position = 0;
    private long msgId = -1;
    private short ipDbId = -1;
    private NmsContact mContact;
    private NmsIpMessage mMessage;
    private Context mContext;

    private int currentIndex;
    private MediaType mediaType;
    private ImageView imageview;
    private TextView textview;
    private LinearLayout tipContainer;
    private Bitmap bitmap;
    public List<NmsIpMessage> ipMediaList = new ArrayList<NmsIpMessage>();
    private NmsIpImageMessage mSession;
    private boolean ready = false;
    private int recLen = 0;

    public static Hashtable<Object, MyThread> hashtable;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            NmsLog.trace(TAG, "handler received msg type: " + msg.what);
            switch (msg.what) {
            case HANDLER_DATA_ERROR:
                break;
            case HANDLER_DATA_READY:
                ready = true;
                initMediaPage();
                break;
            default:
                break;
            }
        }

    };

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        outState.putLong("msgId", msgId);
        outState.putInt("ipDbId", ipDbId);
        outState.putInt("position", position);
        outState.putInt("type", type);
        outState.putInt("time", recLen);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        Window win = getWindow();
        win.addFlags(WindowManager.LayoutParams.FLAG_SECURE);
        setContentView(R.layout.readed_burn_detail);

        Configuration config = getResources().getConfiguration();   
        if (config.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        } else if (config.orientation == Configuration.ORIENTATION_PORTRAIT) {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        }

        imageview = (ImageView) findViewById(R.id.iv_readedburn_image);
        tipContainer = (LinearLayout) findViewById(R.id.ll_readedburn_prompt);
        textview = (TextView) findViewById(R.id.tv_readedburn_time);

        mContext = this;
        if (null == savedInstanceState) {
            msgId = this.getIntent().getLongExtra("msgId", (long) -1);
            ipDbId = (short) this.getIntent().getIntExtra("ipDbId", -1);
            type = this.getIntent().getIntExtra("type", 0);
            position = this.getIntent().getIntExtra("position", 0);
            recLen = this.getIntent().getIntExtra("time", 0);
        } else {
            msgId = savedInstanceState.getLong("msgId", (long) -1);
            ipDbId = (short) savedInstanceState.getInt("ipDbId", -1);
            type = savedInstanceState.getInt("type", 0);
            position = savedInstanceState.getInt("position", 0);
            recLen = savedInstanceState.getInt("time", 0);
        }

        if (msgId > 0) {
            mMessage = NmsIpMessageApiNative.nmsGetIpMsgInfo(msgId);
            mContact = NmsIpMessageApiNative.nmsGetContactInfoViaMsgId(msgId);
        } else {
            mMessage = NmsiSMSApi.nmsGetIpMsgInfoViaDbId(ipDbId);
            mContact = NmsiSMSApi.nmsGetContactInfoViaDbId(ipDbId);
        }

        if (mContact == null || mMessage == null) {
            NmsLog.error(TAG, "The contact is null and msgId:" + msgId + " ipDBid:" + ipDbId);
            this.finish();
            return;
        }

        loadAllMedia();

    }

    private void initMediaPage() {
        tipContainer.setVisibility(View.GONE);
        for (int i = 0; i < ipMediaList.size(); i++) {
            if (currentIndex == i) {
                mSession = (NmsIpImageMessage) ipMediaList.get(i);
                textview.setText(Integer.toString(recLen)
                        + getResources().getString(R.string.STR_NMS_SECOND));
                if (hashtable == null) {
                    hashtable = new Hashtable<Object, MyThread>();
                }
                if (msgId > 0) {
                    if (hashtable.get(msgId) != null) {
                        MyThread mt = hashtable.get(msgId);
                        mt.stopthread();
                        hashtable.remove(msgId);
                    }
                    MyThread mt = new MyThread();
                    hashtable.put(msgId, mt);
                    new Thread(mt).start();
                } else {
                    if (hashtable.get(ipDbId) != null) {
                        MyThread mt = hashtable.get(ipDbId);
                        mt.stopthread();
                        hashtable.remove(ipDbId);
                    }
                    MyThread mt = new MyThread();
                    hashtable.put(ipDbId, mt);
                    new Thread(mt).start();
                }
                setMediaImage();
            }

        }
    }

    private void loadAllMedia() {
        NmsLog.trace(TAG, "thread to load the list info");

        new Thread() {
            @Override
            public void run() {

                SNmsImMsgCountInfo mMsgCountInfo = NmsiSMSApi.nmsSetImMode((int) mContact.getId(),
                        NmsImFlag.NMS_IM_FLAG_ALL, NmsImReadMode.NMS_IM_READ_MODE_ALL);

                if (mMsgCountInfo == null) {
                    NmsLog.error(TAG, "mMsgCountInfo is null. handler send msg, msg type: "
                            + HANDLER_DATA_ERROR);
                    mHandler.sendEmptyMessage(HANDLER_DATA_ERROR);
                    return;
                }

                if (mMsgCountInfo.allMsgCount <= 0) {
                    NmsLog.trace(TAG, "handler send msg, msg type: " + HANDLER_DATA_EMPTY);
                    mHandler.sendEmptyMessage(HANDLER_DATA_EMPTY);
                    return;
                }

                for (int i = 0; i < mMsgCountInfo.allMsgCount; ++i) {
                    NmsIpSessionMessage session = NmsiSMSApi.nmsGetMessage(i);
                    if (session == null) {
                        NmsLog.error(TAG, "session is null! Throw away.");
                        continue;
                    }

                    NmsIpMessage ipMsg = session.ipMsg;

                    if (ipMsg.type == NmsIpMessageType.PICTURE) {
                        if (!((NmsIpAttachMessage) ipMsg).isInboxMsgDownloalable()) {
                            ipMediaList.add(ipMsg);
                        }
                    }
                }
                sortList();

                initCurrIndex();

                NmsLog.trace(TAG, "handler send msg, msg type: " + HANDLER_DATA_READY);
                mHandler.sendEmptyMessage(HANDLER_DATA_READY);
            }
        }.start();
    }

    private void initCurrIndex() {
        if (ipMediaList == null) {
            NmsLog.error(TAG, "ipLocMsgList is null");
            return;
        }

        for (int i = 0; i < ipMediaList.size(); ++i) {
            NmsIpMessage ipMsg = ipMediaList.get(i);
            if (ipMsg == null) {
                NmsLog.error(TAG, "initCurrIndex. ipMsg is null");
                continue;
            }
            if (ipMsg.ipDbId == mMessage.ipDbId) {
                NmsLog.trace(TAG, "currIndex is " + ipMsg.ipDbId);
                currentIndex = i;
                break;
            }
        }
    }

    private void sortList() {
        if (ipMediaList == null) {
            NmsLog.error(TAG, "ipMediaList is null");
            return;
        }

        if (ipMediaList.size() == 0) {
            NmsLog.warn(TAG, "ipMediaList.size() is zero, do not sort");
            return;
        }

        Comparator<NmsIpMessage> timeComparator = new Comparator<NmsIpMessage>() {
            @Override
            public int compare(NmsIpMessage lhs, NmsIpMessage rhs) {
                if (lhs == null || rhs == null) {
                    NmsLog.error(TAG, "lhs or rhs is/are null");
                    return 0;
                }

                if (mediaType == MediaType.LOCATION) {
                    return rhs.time - lhs.time;
                } else {
                    return lhs.time - rhs.time;
                }
            }
        };

        Collections.sort(ipMediaList, timeComparator);
    }

    public void setMediaImage() {
        if (bitmap == null) {
            DisplayMetrics dm = new DisplayMetrics();
            WindowManager wmg = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
            wmg.getDefaultDisplay().getMetrics(dm);

            bitmap = NmsBitmapUtils.getBitmapByPath(mSession.path,
                    NmsBitmapUtils.getOptions(mSession.path), dm.widthPixels, dm.heightPixels);
            if (bitmap == null) {
                try {
                    bitmap = NmsBitmapUtils.getBitmapByPath(mSession.thumbPath,
                            NmsBitmapUtils.getOptions(mSession.thumbPath), dm.widthPixels,
                            dm.heightPixels);
                } catch (Exception e) {
                    NmsLog.warn(TAG, "BitmapFactory.decodeFile failed, ipMsg.ipDbId: "
                            + mSession.ipDbId);
                }
            }

            if (null == bitmap) {
                imageview.setImageResource(R.drawable.isms_media_failed_big);
            } else {
                imageview.setImageBitmap(bitmap);
            }
        }
    }

    final Handler handler = new Handler() { // handle
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case 1:
                if (recLen == 0) {
                    if (hashtable.get(msgId) != null) {
                        MyThread mt = hashtable.get(msgId);
                        mt.stopthread();
                        hashtable.remove(msgId);
                    }
                    NmsReadedBurnDetailsActivity.this.finish();
                    delete();
                }
                if (recLen >= 0) {
                    textview.setText(Integer.toString(recLen)+getResources().getString(R.string.STR_NMS_SECOND));
                    Intent intent = new Intent();
                    intent.setAction(NmsIpMessageStatus.NMS_READEDBURN_TIME_ACTION);
                    intent.putExtra(NmsIpMessageStatus.NMS_IP_MSG_TIME, recLen);
                    intent.putExtra(NmsIpMessageStatus.NMS_IP_MSG_POSITION, position);
                    intent.putExtra(NmsIpMessageStatus.NMS_IP_MSG_SYS_ID, msgId);
                    intent.putExtra(NmsIpMessageStatus.NMS_IP_MSG_IPDB_ID, ipDbId);
                    NmsService.getInstance().sendBroadcast(intent);
                }
                recLen--;
            }
            super.handleMessage(msg);
        }
    };

    private void delete() {
        NmsCommonUtils.deletefile(mSession.thumbPath);
        if (!mSession.thumbPath.equals(mSession.path)) {
            NmsCommonUtils.deletefile(mSession.path);
        }

    }

    public class MyThread implements Runnable { // thread
        boolean runflag = true;

        public synchronized void stopthread() {
            runflag = false;
        }

        public synchronized boolean getrunflag() {
            return runflag;
        }

        @Override
        public void run() {
            runflag = true;
            while (getrunflag()) {
                try {
                    Message message = new Message();
                    message.what = 1;
                    handler.sendMessage(message);
                    Thread.sleep(1000); // sleep 1000ms
                } catch (Exception e) {
                }
            }
        }
    }

    @Override
    protected void onDestroy() {
        unregisterReceiver(broadcastReceiver);
        if (bitmap != null) {
            imageview.setImageBitmap(null);
            bitmap.recycle();
            bitmap = null;
        }
        super.onDestroy();
    }

    private BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            finish();
        }
    };

    @Override
    protected void onResume() {
        super.onResume();

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(NmsIpMessageConsts.ACTION_READENBURN);
        registerReceiver(broadcastReceiver, intentFilter);
    }

}
