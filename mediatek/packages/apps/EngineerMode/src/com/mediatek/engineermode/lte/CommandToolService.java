package com.mediatek.engineermode.lte;

import android.app.Service;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.List;

/*
 * AT command tool should be able to run commands at background.
 */
public class CommandToolService extends Service {
    private static final String TAG = "EM/CommandToolService";
    private static final int MSG_SEND_NEXT_COMMAND = 1;
    private static final int MSG_AT_COMMAND = 2;

    private List<String> mCommands = new ArrayList<String>();
    private int mInterval = 1;
    private String mOutput = new String();
    private boolean mSending = false;

    private OnUpdateResultListener mOnUpdateResultListener;
    private final CommandToolServiceBinder mBinder = new CommandToolServiceBinder();

    public interface OnUpdateResultListener {
        void onUpdateResult();
    }

    public class CommandToolServiceBinder extends Binder {
        CommandToolService getService() {
            return CommandToolService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Xlog.v(TAG, "Enter onStartCommand");
        return START_NOT_STICKY;
    }

    public void startTest(List<String> commands, int interval) {
        Xlog.v(TAG, "startTest");
        mCommands = commands;
        mInterval = interval;
        mOutput = "";
        mSending = true;
        mHander.sendEmptyMessage(MSG_SEND_NEXT_COMMAND);
    }

    public void stopTest() {
        Xlog.v(TAG, "stopTest");
        if (mSending) {
            mSending = false;
            updateResult("Stopped\n");
        }
    }

    private final Handler mHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Xlog.d(TAG, "handleMessage() " + msg.what);
            switch (msg.what) {
            case MSG_SEND_NEXT_COMMAND:
                if (!mSending) {
                    return;
                }
                if (mCommands.size() > 0) {
                    sendAtCommand(mCommands.remove(0), MSG_AT_COMMAND);
                    mHander.sendEmptyMessageDelayed(MSG_SEND_NEXT_COMMAND, mInterval * 1000);
                } else {
                    mSending = false;
                    updateResult("Finished\n");
                }
                break;
            }
        }
    };

    private void sendAtCommand(String str, int message) {
        Xlog.d(TAG, "sendAtCommand() " + str);
        updateResult("Send " + str +"\n");
        String cmd[] = new String[2];
        cmd[0] = str;
        cmd[1] = "";
        if (str.length() > 3 && str.endsWith("?")) {
            cmd[1] = str.substring(2, str.length()-1);
        }
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            GeminiPhone mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
            mGeminiPhone.invokeOemRilRequestStringsGemini(cmd,
                    mAtCmdHander.obtainMessage(message), PhoneConstants.GEMINI_SIM_1);
        } else {
            Phone mPhone = (Phone) PhoneFactory.getDefaultPhone();
            mPhone.invokeOemRilRequestStrings(cmd, mAtCmdHander.obtainMessage(message));
        }
    }

    private final Handler mAtCmdHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Xlog.d(TAG, "handleMessage() " + msg.what);
            switch (msg.what) {
            case MSG_AT_COMMAND:
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Object result = ar.result;
                    if (result != null && (result instanceof String[])) {
                        String[] data = (String[]) result;
                        if (data.length > 0) {
                            updateResult("Return: ");
                            for (int i = 0; i < data.length; i++) {
                                updateResult(data[i] + "\n");
                            }
                        }
                    }
                } else {
                    Xlog.e(TAG, "Exception: " + ar.exception);
                    updateResult("Exception: " + ar.exception + "\n");
                }
                break;
            default:
                break;
            }
        }
    };

    private void updateResult(String result) {
        mOutput += result;
        if (mOnUpdateResultListener != null) {
            mOnUpdateResultListener.onUpdateResult();
        }
    }

    public String getOutput() {
        return mOutput;
    }

    public boolean isRunning() {
        return mSending;
    }

    public void setOnUpdateResultListener(OnUpdateResultListener listener) {
        mOnUpdateResultListener = listener;
    }
}

