
package com.mediatek.DataUsageLockScreenClient;

import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.ConnectivityManager;
import android.net.NetworkPolicyManager;
import android.net.NetworkPolicy;
import android.net.INetworkStatsService;
import android.net.INetworkStatsSession;
import android.net.INetworkPolicyManager;
import android.net.NetworkTemplate;
import android.os.Bundle;
import android.os.HandlerThread;
import android.os.Handler;
import android.os.Message;
import android.os.ServiceManager;
import android.os.RemoteException;
import android.os.Process;
import android.os.PowerManager;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.provider.Telephony.SIMInfo;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.Slog;
import android.view.MotionEvent;
import android.view.View;

import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.util.Objects;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.keyguard.ext.KeyguardLayerInfo;
import com.mediatek.keyguard.ext.IKeyguardLayer;
import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.DataUsageLockScreenClient.R;
import com.mediatek.telephony.TelephonyManagerEx;
import com.mediatek.ngin3d.Actor;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Text;
import com.mediatek.ngin3d.android.StageTextureView;
import com.mediatek.ngin3d.animation.Animation;
import com.mediatek.ngin3d.animation.BasicAnimation;
import com.mediatek.ngin3d.animation.Mode;
import com.mediatek.ngin3d.animation.PropertyAnimation;

import java.math.BigDecimal;
import java.util.List;
import java.util.Calendar;
import java.util.Random;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import static android.net.NetworkPolicyManager.computeLastCycleBoundary;
import static android.telephony.TelephonyManager.SIM_STATE_READY;
import static android.Manifest.permission.MANAGE_NETWORK_POLICY;
import static android.Manifest.permission.READ_NETWORK_USAGE_HISTORY;

public class DataUsageStageView extends StageTextureView {

    public static final boolean DEBUG = true;
    private static final String TAG = "DataUsageStageView";

    private Context mContext;

    /* SIM card info */
    private List<SimInfoRecord> simList;
    private int mCardCount = 0;
    private int mCard1ColorNum;
    private int mCard2ColorNum;
    private long mSimId1;
    private long mSimId2;
    private int mSlot1;
    private int mSlot2;
    private int mSim1ShowLockScreen;
    private int mSim2ShowLockScreen;
    private int mNowDate;

    /* 3D actors */
    private Image mCard1Scale;
    private Image mCard2Scale;

    private Container mContainerText1; // ContainerText = text + background
    private Container mContainerText2;

    private Container mContainerCircle1;
    private Container mContainerCircle2;

    private Actor mActorCard1;
    private Actor mActorCard2;

    /* data/network/telephony service */
    private NetworkPolicyManager mPolicyManager;
    private NetworkPolicy mPolicy;
    private INetworkStatsService mStatsService;

    /* Data info: limit, usage */
    private long mCard1Limit;
    private long mCard2Limit;

    /* SIMCardData object: contain data info & circle info */
    private SIMCardData mCard1;
    private SIMCardData mCard2;

    /* Animation related & layout & position */
    private Random mRandom = new Random(System.currentTimeMillis());
    private static int sOldRandomX = 0;

    private int mDisplayWidth;
    private int mDisplayHeight;

    private int mCard1LimitPx;
    private int mCard2LimitPx;

    private int mTextW;
    private int mTextH;

    private float mTextScale1;
    private float mTextScale2;
    private int mTitleBarH;

    private int mLimitCircleMaxPx;
    private int mLimitCircleMinPx;

    private boolean mIsScale1Run;
    private boolean mIsScale2Run;
    private boolean mIsApkRun;
    private boolean mInterceptTouch;
    private boolean mIsLandscapeMode;

    private int mBoundBufferX;
    private int mBoundBufferY;

    private float mCard1OldX;
    private float mCard1OldY;
    private float mCard2OldX;
    private float mCard2OldY;


    /* msg/event handler */
    private static final int MSG_DATA_STATS_CHANGED = 0;
    private static final int MSG_FLOATING_SIM1_ONE_CARD = 1;
    private static final int MSG_FLOATING_SIM1_TWO_CARD = 2;
    private static final int MSG_FLOATING_SIM2_ONE_CARD = 3;
    private static final int MSG_FLOATING_SIM2_TWO_CARD = 4;
    private static final int MSG_SIM_STATS_CHANGED = 5 ;
    private HandlerThread mHandlerThread;
    private Handler mHandler;
    private Object mSyncObjForHandler = new Object();
    public static final String ACTION_NETWORK_STATS_UPDATED =
            "com.android.server.action.NETWORK_STATS_UPDATED";

    private ExecutorService mExecutorService = null;

    private static int mTotalNewCount = 0;
    private int mViewID;

    /**
     * The method to print log
     *
     * @param tag the tag of the class
     * @param msg the log message to print
     */
    private void log(String tag, String msg) {
        if (DataUsageStageView.DEBUG) {
            Log.d(tag, "[" + mViewID + "] " + msg);
        }
    }

    @Override
    public String toString() {
        return "[" + mViewID + "] " + super.toString();
    }

    // used for handler messages
    private static final int TIMEOUT = 1;

    // The default amount of time we stay awake (used for all key input)
    protected static final int AWAKE_INTERVAL_DEFAULT_MS = 10000;
    private int mWakelockSequence;

    /** High level access to the power manager for WakeLocks */
    private PowerManager mPM;

    /**
     * Used to keep the device awake while the keyguard is showing, i.e for
     * calls to {@link #pokeWakelock()}
     */
    private PowerManager.WakeLock mWakeLock;
    private TelephonyManagerEx mTelephonyManager;

    // Listen card1 state
    private final PhoneStateListener mPhoneStateListener_1 = new PhoneStateListener() {
        public void onServiceStateChanged(ServiceState serviceState) {
            // checking in synchronized handleDataUsageView() to avoid concurrent access
            createInternal(0, serviceState);
        }
    };

    // Listen card1 state
    private final PhoneStateListener mPhoneStateListener_2 = new PhoneStateListener() {
        public void onServiceStateChanged(ServiceState serviceState) {
            // checking in synchronized handleDataUsageView() to avoid concurrent access
            createInternal(1, serviceState);
        }
    };

    public DataUsageStageView(Context context) {
        super(context);
        mContext = context;
        mViewID = mTotalNewCount;
        mTotalNewCount++;

        // Store shader cache here
        setCacheDir(mContext, "/data/data/com.android.systemui/lockscreen");

        log(TAG, "DataUsageStageView()");

        // listen service state
        startListenState();

        // wake lock
        mPM = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
        mWakeLock = mPM.newWakeLock(
                PowerManager.FULL_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP,
                "DataUsageStageView");
        mWakeLock.setReferenceCounted(false);

        // register DataUsage Update
        registerDataUsageUpdated();

        /// M: Force set it not opaque, so that background views can be drawn
        setOpaque(false);

        // set limit fps
        mStage.setMaxFPS(10);
    }

    public View create() {
        return createInternal(0, null);
    }

    private static int mTotalRunInstance = 0;
    public class DataUsageCreateRunnable implements Runnable {
        private int mID = 0;
        private int mSim = 0;
        private ServiceState mServiceState = null;

        public DataUsageCreateRunnable() {
            super();
            mID = mTotalRunInstance;
            mTotalRunInstance++;
        }

        public void run() {
            Log.v(TAG, "[" + mViewID + "] [" + mID + "] create, handleDataUsageLockScreenView() , mServiceState:" + mServiceState);
            handleDataUsageView(mSim, mServiceState);
            Log.v(TAG, "[" + mViewID + "] [" + mID + "] create, end");
            mServiceState = null;
        }

        public void setSim(int sim, ServiceState serviceState) {
            mSim = sim;
            mServiceState = serviceState;
        }
    }

    public View createInternal(int sim, ServiceState serviceState) {
        DataUsageCreateRunnable runInstance = new DataUsageCreateRunnable();
        runInstance.setSim(sim, serviceState);

        if (null == mExecutorService) {
            mExecutorService = Executors.newSingleThreadExecutor();
        }

        Log.v(TAG, "[" + mViewID + "] mExecutorService.submit() " + runInstance);
        mExecutorService.submit(runInstance);
        return this;

    }

    private void handleDataUsageView(int sim, ServiceState serviceState) {
        log(TAG, "handleDataUsageView start");
        synchronized (this) {
            boolean doNothing = true;

            if (null == serviceState) {
                doNothing = false;
            } else if (serviceState != null && sim == 0) {
                // copied from mPhoneStateListener_1.onServiceStateChanged() SIM1
                if (serviceState.getState() == ServiceState.STATE_IN_SERVICE ||
                        serviceState.getState() == ServiceState.STATE_EMERGENCY_ONLY) {
                    log(TAG, "mPSListener_1->mCardCount=" + mCardCount + " mSim1Show=" + mSim1ShowLockScreen);
                    if (mCardCount == 0 && mSim1ShowLockScreen == 1) {
                        doNothing = false;
                    }
                } else {
                    if (mIsApkRun == true) {
                        if (mContainerCircle1!=null && mContainerCircle1.getVisible()) {
                            mStage.removeAll();
                            doNothing = false;
                        }
                    }
                }
            } else if (serviceState != null) {
                // copied from mPhoneStateListener_2.onServiceStateChanged() SIM2
                if (serviceState.getState() == ServiceState.STATE_IN_SERVICE ||
                        serviceState.getState() == ServiceState.STATE_EMERGENCY_ONLY) {
                    log(TAG, "mPSListener_2->mCardCount=" + mCardCount + " mSim2Show=" + mSim2ShowLockScreen);
                    if (mCardCount == 0 && mSim2ShowLockScreen == 1) {
                        doNothing = false;
                    }
                } else {
                    if (mIsApkRun == true) {
                        if (mContainerCircle2!=null && mContainerCircle2.getVisible()) {
                            mStage.removeAll();
                            doNothing = false;
                        }
                    }
                }
            }

            log(TAG, "doNothing:" + doNothing);
            if (true == doNothing) {
                return;
            }

            // Initial parameter
            initialParameter();

            // Show on LockScreen
            getShowLockScreen();

            log(TAG, "in flight mode : " + Settings.System.getInt(mContext.getContentResolver(), Settings.System.AIRPLANE_MODE_ON, 0));
            // check ready to display or not
            if (Settings.System.getInt(mContext.getContentResolver(), Settings.System.AIRPLANE_MODE_ON, 0) == 1) {
                mCardCount = 0;
                return;
            }

            // get SimId SlotId and color
            getSimCardInfo();

            // set circle size Px
            setOutCircleSize();

            if (!initSimCardData()) {
                mCardCount = 0;
                return;
            }

            mIsApkRun = true;

            // Add actor
            addDataUsageActor();

            // start floating
            startCircleFloating();
        }
        log(TAG, "handleDataUsageView end");
    }

    public void destroy() {
        mIsApkRun = false;
        mContext.unregisterReceiver(mStatsReceiver);
        mContext.unregisterReceiver(mSimStateReceiver);
        mStatsReceiver = null;
        mSimStateReceiver = null;

        synchronized(mSyncObjForHandler) {
            if (mHandlerThread != null) {
                log(TAG, "mHandlerThread is not null!");
                mHandlerThread.getLooper().quit();
                mHandlerThread = null;
                mHandler = null;
                mHandlerCallback = null;
            }
        }

        stopListenState();

        mExecutorService.shutdown();
        mExecutorService = null;

        log(TAG, "destroy()");
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int touchX = (int) event.getX();
        int touchY = (int) event.getY();
 
        synchronized (this) {
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    // wake lock
                    mWakeLock.acquire();
                    mHandlerWakeLock.removeMessages(TIMEOUT);
                    mWakelockSequence++;
                    Message msg = mHandlerWakeLock.obtainMessage(TIMEOUT, mWakelockSequence, 0);
                    mHandlerWakeLock.sendMessageDelayed(msg, AWAKE_INTERVAL_DEFAULT_MS);

                    // hittest
                    mActorCard1 = null;
                    mActorCard2 = null;
                    log(TAG, "onTouchEvent->ACTION_DOWN");
                    if (mCardCount == Constants.ONE_CARD) {
                        log(TAG, "onTouchEvent->mCardCount == Constants.ONE_CARD");
                        if (mSim1ShowLockScreen == 1 && mCard1Limit > 0) {
                            if (mContainerCircle1 != null) {
                                mActorCard1 = mContainerCircle1.hitTest(new Point(touchX, touchY));
                            }
                        } else if (mSim2ShowLockScreen == 1 && mCard2Limit > 0) {
                            if (mContainerCircle2 != null) {
                                mActorCard2 = mContainerCircle2.hitTest(new Point(touchX, touchY));
                            }
                        } else {
                            // do nothing in this case.
                        }

                    } else if (mCardCount == Constants.TWO_CARD) {
                        log(TAG, "onTouchEvent->mCardCount == Constants.TWO_CARD");
                        if (mSim1ShowLockScreen == 1 && mCard1Limit > 0) {
                            if (mContainerCircle1 != null) {
                                mActorCard1 = mContainerCircle1.hitTest(new Point(touchX, touchY));
                            }
                        }

                        if (mSim2ShowLockScreen == 1 && mCard2Limit > 0) {
                            if (mContainerCircle2 != null) {
                                mActorCard2 = mContainerCircle2.hitTest(new Point(touchX, touchY));
                            }
                        }
                    } else {
                        // mCardCount should not be 0
                    }

                    if (mActorCard1 != null && mActorCard2 == null) {
                        log(TAG, "onTouchEvent->mActorCard1 != null");
                        if (!mIsScale1Run) {
                            scaleOneAnimation();
                        }
                        mInterceptTouch = true;

                    } else if (mActorCard1 == null && mActorCard2 != null) {
                        log(TAG, "onTouchEvent->mActorCard2 != null");
                        if (!mIsScale2Run) {
                            scaleTwoAnimation();
                        }
                        mInterceptTouch = true;
                    } else {
                        log(TAG, "onTouchEvent->mActorCard1=mActorCard2=null");
                        mInterceptTouch = false;
                    }
                    break;
            }
        }
        return mInterceptTouch;
    }

    private void scaleOneAnimation() {

        mIsScale1Run = true;
        mCard1Scale.setVisible(true);
        mContainerText1.setVisible(true);

        // show text One
        showUsageText(true);

        Animation.Listener listen = new Animation.Listener() {
            public void onCompleted(Animation animation) {
                BasicAnimation opacityLText2 = new PropertyAnimation(
                        mContainerText1, "opacity", 255, 0).setDuration(3500);

                BasicAnimation opacityLScale2 = new PropertyAnimation(mCard1Scale,
                        "opacity", 255, 0).setDuration(3500);

                opacityLText2.start();
                opacityLScale2.start();

                mIsScale1Run = false;
            }
        };

        BasicAnimation moveScaleLeft = new PropertyAnimation(
                mContainerText1, "position",
                new Point(mTextScale1 + (mTextW / (float) 2), 0),
                new Point(mTextScale1 + (mTextW / (float) 2),
                        (mNowDate * (getResources().getInteger(R.integer.SCALE_BUFFER_PX)
                                + getResources().getInteger(R.integer.SCALE_LINE_PX)))
                                - (mTextH / (float) 2)
                                - getResources().getInteger(R.integer.SCALE_LINE_PX)))
                .setMode(Mode.EASE_IN_OUT_SINE);

        int time = setTimer(mNowDate);
        moveScaleLeft.setDuration(time);
        moveScaleLeft.start();

        BasicAnimation opacityLText1 = new PropertyAnimation(mContainerText1,
                "opacity", 255, 255).setDuration(time);
        BasicAnimation opacityLScale1 = new PropertyAnimation(mCard1Scale,
                "opacity", 255, 255).setDuration(time);

        opacityLText1.start();
        opacityLScale1.start();
        opacityLText1.addListener(listen);
    }

    private void scaleTwoAnimation() {

        mIsScale2Run = true;
        mCard2Scale.setVisible(true);
        mContainerText2.setVisible(true);

        // show text Two
        showUsageText(false);

        Animation.Listener listen = new Animation.Listener() {
            public void onCompleted(Animation animation) {
                BasicAnimation opacityRText2 = new PropertyAnimation(
                        mContainerText2, "opacity", 255, 0).setDuration(3500);

                BasicAnimation opacityRScale2 = new PropertyAnimation(mCard2Scale,
                        "opacity", 255, 0).setDuration(3500);

                opacityRText2.start();
                opacityRScale2.start();

                mIsScale2Run = false;
            }
        };

        BasicAnimation moveScaleRight = new PropertyAnimation(
                mContainerText2, "position",
                new Point(mTextScale2 + (mTextW / (float) 2), 0),
                new Point(mTextScale2 + (mTextW / (float) 2),
                        (mNowDate * (getResources().getInteger(R.integer.SCALE_BUFFER_PX)
                                + getResources().getInteger(R.integer.SCALE_LINE_PX)))
                                - (mTextH / (float) 2)
                                - getResources().getInteger(R.integer.SCALE_LINE_PX)))
                .setMode(Mode.EASE_IN_OUT_SINE);

        int time = setTimer(mNowDate);
        moveScaleRight.setDuration(time);
        moveScaleRight.start();
        BasicAnimation opacityRText1 = new PropertyAnimation(mContainerText2,
                "opacity", 255, 255).setDuration(time);
        BasicAnimation opacityRScale2 = new PropertyAnimation(mCard2Scale,
                "opacity", 255, 255).setDuration(time);

        opacityRText1.start();
        opacityRScale2.start();
        opacityRText1.addListener(listen);
    }

    private int setTimer(int date) {
        int time;
        if (date <= 10)
            time = 500;
        else if (date > 10 && date <= 20)
            time = 1000;
        else
            time = 2000;
        return time;
    }

    private void simcard1Floating(int cardCount) {
        float randomX;
        float randomY;

        synchronized(mSyncObjForHandler) {
            if (mHandler == null) {
                return;
            }

            switch (cardCount) {
                case Constants.ONE_CARD:
                    randomX = mCard1.mBound_X[0] + (float) mRandom.nextInt(mBoundBufferX);
                    randomY = mCard1.mBound_Y[0] + (float) mRandom.nextInt(mBoundBufferY);

                    // Formula for slope
                    if ((randomY - mCard1.mBound_Y[0]) * (mCard1.mBound_X[1] - mCard1.mBound_X[0])
                            < (mCard1.mBound_Y[1] - mCard1.mBound_Y[0])
                            * (randomX - mCard1.mBound_X[0])
                            || ((randomY - mCard1.mBound_Y[1])
                                    * (mCard1.mBound_X[2] - mCard1.mBound_X[2])
                                    > (mCard1.mBound_Y[2] - mCard1.mBound_Y[1])
                                    * (randomX - mCard1.mBound_X[2]))) {

                        if (sOldRandomX != 0 && (sOldRandomX - randomX) > 100) {
                            randomX = randomX + 100;
                        } else if (sOldRandomX != 0 && (randomX - sOldRandomX) > 100) {
                            randomX = randomX - 100;
                        } else {
                            // do nothing (randomX)
                        }
                        sOldRandomX = (int) randomX;

                        moveOneFloating(randomX, randomY - mTitleBarH, cardCount);
                    } else {
                        mHandler.obtainMessage(MSG_FLOATING_SIM1_ONE_CARD).sendToTarget();
                    }
                    break;

                case Constants.TWO_CARD:
                    if (mIsLandscapeMode) {
                        randomX = mCard1.mBound1_X[4] - (float) mRandom.nextInt(mBoundBufferX);
                        randomY = mCard1.mBound1_Y[4] - (float) mRandom.nextInt(mBoundBufferY);

                        if (((randomY - mCard1.mBound1_Y[0]) * (mCard1.mBound1_X[1] - mCard1.mBound1_X[0])
                                > (mCard1.mBound1_Y[1] - mCard1.mBound1_Y[0])
                                * (randomX - mCard1.mBound1_X[0]))
                                && ((randomY - mCard1.mBound1_Y[2])
                                        * (mCard1.mBound1_X[3] - mCard1.mBound1_X[2])
                                        < (mCard1.mBound1_Y[3] - mCard1.mBound1_Y[2])
                                        * (randomX - mCard1.mBound1_X[2]))) {

                            moveOneFloating(randomX, randomY - mTitleBarH, cardCount);
                        } else {
                            mHandler.obtainMessage(MSG_FLOATING_SIM1_TWO_CARD).sendToTarget();
                        }
                    } else {
                        randomX = mCard1.mBound1_X[0] + (float) mRandom.nextInt(mBoundBufferX);
                        randomY = mCard1.mBound1_Y[2] + (float) mRandom.nextInt(mBoundBufferY);

                        if (((randomY - mCard1.mBound1_Y[0]) * (mCard1.mBound1_X[1] - mCard1.mBound1_X[0])
                            < (mCard1.mBound1_Y[1] - mCard1.mBound1_Y[0])
                            * (randomX - mCard1.mBound1_X[0]))
                            && ((randomY - mCard1.mBound1_Y[2])
                                    * (mCard1.mBound1_X[3] - mCard1.mBound1_X[2])
                                    > (mCard1.mBound1_Y[3] - mCard1.mBound1_Y[2])
                                    * (randomX - mCard1.mBound1_X[2]))) {

                            moveOneFloating(randomX, randomY - mTitleBarH, cardCount);
                        } else {
                            mHandler.obtainMessage(MSG_FLOATING_SIM1_TWO_CARD).sendToTarget();
                        }
                    }
                    break;
            }
        }
    }

    private void moveOneFloating(float randomX, float randomY, final int cardCount) {

        Animation.Listener listen = new Animation.Listener() {
            public void onCompleted(Animation animation) {
                simcard1Floating(cardCount);
            }
        };

        BasicAnimation card1Floating = new PropertyAnimation(mContainerCircle1,
                "position", new Point(mCard1OldX, mCard1OldY), new Point(randomX, randomY))
                .setMode(Mode.EASE_IN_OUT_SINE);

        card1Floating.setDuration(5000);
        card1Floating.start();

        card1Floating.addListener(listen);
        mCard1OldX = randomX;
        mCard1OldY = randomY;
    }

    private void simcard2Floating(int cardCount) {
        float randomX;
        float randomY;

        synchronized(mSyncObjForHandler) {
            if (mHandler == null) {
                return;
            }

            switch (cardCount) {
                case Constants.ONE_CARD:
                    randomX = mCard2.mBound_X[0] + (float) mRandom.nextInt(mBoundBufferX);
                    randomY = mCard2.mBound_Y[0] + (float) mRandom.nextInt(mBoundBufferY);

                    // Formula for slope
                    if ((randomY - mCard2.mBound_Y[0]) * (mCard2.mBound_X[1] - mCard2.mBound_X[0])
                            < (mCard2.mBound_Y[1] - mCard2.mBound_Y[0])
                            * (randomX - mCard2.mBound_X[0])
                            || ((randomY - mCard2.mBound_Y[1])
                                    * (mCard2.mBound_X[2] - mCard2.mBound_X[2])
                                    > (mCard2.mBound_Y[2] - mCard2.mBound_Y[1])
                                    * (randomX - mCard2.mBound_X[2]))) {

                        if (sOldRandomX != 0 && (sOldRandomX - randomX) > 100) {
                            randomX = randomX + 100;
                        } else if (sOldRandomX != 0 && (randomX - sOldRandomX) > 100) {
                            randomX = randomX - 100;
                        } else {
                            // do nothing (randomX)
                        }
                        sOldRandomX = (int) randomX;

                        moveTwoFloating(randomX, randomY - mTitleBarH, cardCount);

                    } else {
                        mHandler.obtainMessage(MSG_FLOATING_SIM2_ONE_CARD).sendToTarget();
                    }
                    break;

                case Constants.TWO_CARD:
                    if (mIsLandscapeMode) {
                        randomX = mCard2.mBound2_X[4] + (float) mRandom.nextInt(mBoundBufferX);
                        randomY = mCard2.mBound2_Y[4] + (float) mRandom.nextInt(mBoundBufferY);

                        if (((randomY - mCard2.mBound2_Y[0]) * (mCard2.mBound2_X[1] - mCard2.mBound2_X[0])
                                < (mCard2.mBound2_Y[1] - mCard2.mBound2_Y[0])
                                * (randomX - mCard2.mBound2_X[0]))
                                && ((randomY - mCard2.mBound2_Y[2])
                                        * (mCard2.mBound2_X[3] - mCard2.mBound2_X[2])
                                        > (mCard2.mBound2_Y[3] - mCard2.mBound2_Y[2])
                                        * (randomX - mCard2.mBound2_X[2]))) {

                            moveTwoFloating(randomX, randomY - mTitleBarH, cardCount);

                        } else {
                            mHandler.obtainMessage(MSG_FLOATING_SIM2_TWO_CARD).sendToTarget();
                        }
                    } else {
                        randomX = mCard2.mBound2_X[0] + (float) mRandom.nextInt(mBoundBufferX);
                        randomY = mCard2.mBound2_Y[1] + (float) mRandom.nextInt(mBoundBufferY);

                        // Formula for Slope
                        if (((randomY - mCard2.mBound2_Y[0]) * (mCard2.mBound2_X[1] - mCard2.mBound2_X[0])
                            > (mCard2.mBound2_Y[1] - mCard2.mBound2_Y[0])
                            * (randomX - mCard2.mBound2_X[0]))
                            && ((randomY - mCard2.mBound2_Y[2])
                                    * (mCard2.mBound2_X[3] - mCard2.mBound2_X[2])
                                    < (mCard2.mBound2_Y[3] - mCard2.mBound2_Y[2])
                                    * (randomX - mCard2.mBound2_X[2]))) {

                            moveTwoFloating(randomX, randomY - mTitleBarH, cardCount);

                        } else {
                            mHandler.obtainMessage(MSG_FLOATING_SIM2_TWO_CARD).sendToTarget();
                        }
                    }
                    break;
            }
        }
    }

    private void moveTwoFloating(float randomX, float randomY, final int cardCount) {

        Animation.Listener listen = new Animation.Listener() {
            public void onCompleted(Animation animation) {
                simcard2Floating(cardCount);
            }
        };

        BasicAnimation card2Floating = new PropertyAnimation(mContainerCircle2,
                "position", new Point(mCard2OldX, mCard2OldY), new Point(randomX, randomY))
                .setMode(Mode.EASE_IN_OUT_SINE);

        card2Floating.setDuration(5000);
        card2Floating.start();

        card2Floating.addListener(listen);
        mCard2OldX = randomX;
        mCard2OldY = randomY;
    }

    // mStage add method
    private void addDataUsageActor() {
        // add Card1 Actors to stage
        if (mSlot1 > -1 && mSim1ShowLockScreen == 1 && mCard1LimitPx > 0) {
            mStage.add(mCard1Scale);
            mContainerCircle1 = mCard1.getConDataCircle();
            mStage.add(mContainerCircle1);
            mContainerText1 = mCard1.getContainerText();
            mStage.add(mContainerText1);
            mCard1Scale.setVisible(false);
            mContainerText1.setVisible(false);
        }

        // add Card2 Actors to stage
        if (mSlot2 > -1 && mSim2ShowLockScreen == 1 && mCard2LimitPx > 0) {
            mStage.add(mCard2Scale);
            mContainerCircle2 = mCard2.getConDataCircle();
            mStage.add(mContainerCircle2);
            mContainerText2 = mCard2.getContainerText();
            mStage.add(mContainerText2);
            mCard2Scale.setVisible(false);
            mContainerText2.setVisible(false);
        }
    }

    // set data limit out circle px
    private void setOutCircleSize() {
        int card1LimitMb = 0;
        int card2LimitMb = 0;

        if (FeatureOption.MTK_GEMINI_SUPPORT == false) {
            card1LimitMb = getSimCardLimit(Constants.SINGLE_CARD);

            if (mSlot1 > -1 && mSim1ShowLockScreen == 1) {
                if (card1LimitMb == 0)
                    mCard1LimitPx = 0;
                else
                    mCard1LimitPx = mLimitCircleMaxPx;
            }
        } else {
            // get card 1
            card1LimitMb = getSimCardLimit(Constants.GEMINI_CARD_ONE);

            // get card 2
            card2LimitMb = getSimCardLimit(Constants.GEMINI_CARD_TWO);

            // comparison size
            // spec: out circle size
            if (mCardCount == Constants.TWO_CARD) {
                log(TAG, "Gemini support->Constants.TWO_CARD");

                if (card1LimitMb == card2LimitMb) {
                    if (card1LimitMb == 0 && card2LimitMb == 0) {
                        mCard1LimitPx = 0;
                        mCard2LimitPx = 0;
                    } else {
                        mCard1LimitPx = mLimitCircleMaxPx;
                        mCard2LimitPx = mLimitCircleMaxPx;
                    }
                } else if (card1LimitMb > card2LimitMb) {
                    mCard1LimitPx = mLimitCircleMaxPx;
                    mCard2LimitPx = setLittleCirclePX(card2LimitMb, card1LimitMb);
                } else {
                    mCard2LimitPx = mLimitCircleMaxPx;
                    mCard1LimitPx = setLittleCirclePX(card1LimitMb, card2LimitMb);
                }
                log(TAG, "mCard1LimitPx=" + mCard1LimitPx + " mCard2LimitPx=" + mCard2LimitPx);

            } else if (mCardCount == Constants.ONE_CARD) {
                log(TAG, "Gemini support->Constants.ONE_CARD");
                // GEMINI_CARD_ONE
                if (mSlot1 > -1 && mSim1ShowLockScreen == 1) {
                    if (card1LimitMb == 0)
                        mCard1LimitPx = 0;
                    else
                        mCard1LimitPx = mLimitCircleMaxPx;
                    log(TAG, "mCard1LimitPx=" + mCard1LimitPx);
                } else { // GEMINI_CARD_TWO
                    if (card2LimitMb == 0)
                        mCard2LimitPx = 0;
                    else
                        mCard2LimitPx = mLimitCircleMaxPx;
                    log(TAG, "mCard2LimitPx=" + mCard2LimitPx);
                }
            } else {
                // in this case, it should not be 0.
            }
        }
    }

    private int getSimCardLimit(int cardNum) {
        int limitMb = 0;
        switch (cardNum) {
            case Constants.SINGLE_CARD:
            case Constants.GEMINI_CARD_ONE:
                if (mSimId1 > -1 && mSim1ShowLockScreen == 1) {
                    mCard1Limit = getDataUsageLimit(mSlot1);
                    log(TAG, "getSimCardLimit->mCard1Limit=" + mCard1Limit);
                    limitMb = (int) (mCard1Limit / Constants.MB_UNIT);
                    log(TAG, "limitMb=" + limitMb);
                    if (limitMb == 0) {
                        mCard1LimitPx = 0;
                        break;
                    }
                } else {
                    mCard1LimitPx = 0;
                }
                break;
            case Constants.GEMINI_CARD_TWO:
                if (mSimId2 > -1 && mSim2ShowLockScreen == 1) {
                    mCard2Limit = getDataUsageLimit(mSlot2);
                    log(TAG, "getSimCardLimit->mCard2Limit=" + mCard2Limit);
                    limitMb = (int) (mCard2Limit / Constants.MB_UNIT);
                    log(TAG, "limitMb=" + limitMb);
                    if (limitMb == 0) {
                        mCard2LimitPx = 0;
                        break;
                    }
                } else {
                    mCard2LimitPx = 0;
                }
                break;
        }
        return limitMb;
    }

    // setOutCircleSize submethod, formula from UI spec.
    private int setLittleCirclePX(int littleSize, int bigSize) {
        // Spec:
        if (littleSize == 0) {
            return 0;
        } else {
            return (int) ((mLimitCircleMaxPx - mLimitCircleMinPx) * ((double) ((double) littleSize / (double) bigSize)) + mLimitCircleMinPx);
        }
    }

    // get SIM card 1 or 2 or both & get data usage color
    private void getSimCardInfo() {
        int simnumber = SimInfoManager.getInsertedSimCount(mContext);
        log(TAG, "getSimCardInfo, simnumber= " + simnumber);

        mCardCount = 0;
        simList = SimInfoManager.getInsertedSimInfoList(mContext);
        if (simList == null) {
            log(TAG, "simList = NULL");
            return;
        }


        for (SimInfoRecord info : simList) {
            if (info.mSimSlotId == PhoneConstants.GEMINI_SIM_1) {
                mSimId1 = info.mSimInfoId;
                SIMInfo localSimInfo = SIMInfo.getSIMInfoById(mContext, mSimId1);
                if (localSimInfo != null) {
                    mSlot1 = localSimInfo.mSlot;
                } else {
                    log(TAG, "SIMInfo.getSIMInfoById(" + mSimId1 + ") = null");
                }
                mCard1ColorNum = info.mColor;
                log(TAG, "mSimId1:" + mSimId1 + ", mSlot1:" + mSlot1 + ", mCard1ColorNum:"
                        + mCard1ColorNum);
            } else if (info.mSimSlotId == PhoneConstants.GEMINI_SIM_2) {
                mSimId2 = info.mSimInfoId;
                SIMInfo localSimInfo = SIMInfo.getSIMInfoById(mContext, mSimId2);
                if (localSimInfo != null) {
                    mSlot2 = localSimInfo.mSlot;
                } else {
                    log(TAG, "SIMInfo.getSIMInfoById(" + mSimId2 + ") = null");
                }
                mCard2ColorNum = info.mColor;
                log(TAG, "mSimId2:" + mSimId2 + ", mSlot2:" + mSlot2 + ", mCard2ColorNum:"
                        + mCard2ColorNum);
            } else {
                // do nothing
            }
            mCardCount++;
        }
        log(TAG, "mCardCount = " + mCardCount);
    }

    // get data usage limit (Gemini)
    public long getDataUsageLimit(int slotId) {
        long limiteBytes;

        final TelephonyManagerEx telephony = TelephonyManagerEx.getDefault();
        //(TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);

        String subsId = null;
        if (FeatureOption.MTK_GEMINI_SUPPORT == false) { // Single SIM
            subsId = telephony.getSubscriberId(0);
        } else { // Gemini support
            subsId = telephony.getSubscriberId(slotId);
        }

        mPolicyManager = (NetworkPolicyManager) mContext
                .getSystemService(Context.NETWORK_POLICY_SERVICE);
        mPolicy = getNetworkPolicy(subsId, mPolicyManager);

        if (mPolicy == null)
            return 0;

        // mPolicy.limitBytes = -1 means policy LIMIT_DISABLED
        limiteBytes = mPolicy.limitBytes;
        log(TAG, "mPolicy.limiteBytes = " + limiteBytes);
        return limiteBytes;
    }

    // intent ACTION_NETWORK_STATS_UPDATED
    private void registerDataUsageUpdated() {
        synchronized(mSyncObjForHandler) {
            mHandlerThread = new HandlerThread(TAG);
            mHandlerThread.start();
            mHandler = new Handler(mHandlerThread.getLooper(), mHandlerCallback);

            final IntentFilter statsFilter = new IntentFilter(ACTION_NETWORK_STATS_UPDATED);
            mContext.registerReceiver(mStatsReceiver, statsFilter, READ_NETWORK_USAGE_HISTORY, mHandler);

            final IntentFilter simStateFilter = new IntentFilter(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
            mContext.registerReceiver(mSimStateReceiver, simStateFilter, null, mHandler);
        }
    }

    // get data usage at present (Gemini)
    private long getCurrentDataUsage(long simId, int slot) {
        // a. get Start Date
        long nowTime = System.currentTimeMillis(); // currentTime.

        log(TAG, "getCurrentDataUsage() simId:" + simId + ", slot:" + slot);
        log(TAG, "getCurrentDataUsage() nowTime:" + nowTime);

        final TelephonyManagerEx telephony = TelephonyManagerEx.getDefault();
        //(TelephonyManager) mContextgetSystemService(Context.TELEPHONY_SERVICE);
        String subsId = null;

        log(TAG, "getCurrentDataUsage() telephony:" + telephony);

        if (FeatureOption.MTK_GEMINI_SUPPORT == false) { // Single SIM
            subsId = telephony.getSubscriberId(0);
        } else { // Gemini support
            subsId = telephony.getSubscriberId(slot);
        }

        log(TAG, "getCurrentDataUsage() subsId:" + subsId);

        mPolicyManager = (NetworkPolicyManager) mContext
                .getSystemService(Context.NETWORK_POLICY_SERVICE);
        mPolicy = getNetworkPolicy(subsId, mPolicyManager);

        // Protection : restart device mPolicy = null
        if (mPolicy == null) {
            log(TAG, "getCurrentDataUsage() mPolicy:" + mPolicy);
            return 0;
        }

        mStatsService = INetworkStatsService.Stub.asInterface(ServiceManager
                .getService(Context.NETWORK_STATS_SERVICE));

        long ethernetBytes = 0;
        INetworkStatsSession mStatsSession;
        final long cycleStart = computeLastCycleBoundary(nowTime, mPolicy);

        log(TAG, "getCurrentDataUsage() cycleStart:" + cycleStart);

        try {
            mStatsSession = mStatsService.openSession();
            NetworkTemplate template = NetworkTemplate.buildTemplateMobileAll(subsId);
            log(TAG, "getCurrentDataUsage() NetworkTemplate:" + template);

            if (FeatureOption.MTK_GEMINI_SUPPORT == false) { // Single SIM
                ethernetBytes = mStatsSession.getSummaryForNetwork(
                        template,
                        cycleStart, nowTime).getTotalBytes();
                log(TAG, "getCurrentDataUsage() getSummaryForNetwork:" + mStatsSession.getSummaryForNetwork(template, cycleStart, nowTime));
            } else { // Gemini support
                ethernetBytes = mStatsSession.getSummaryForNetwork(
                        template,
                        cycleStart, nowTime).getTotalBytes();
                log(TAG, "getCurrentDataUsage() getSummaryForNetwork:" + mStatsSession.getSummaryForNetwork(template, cycleStart, nowTime));
            }
            log(TAG, "return->getCurrentDataUsage=" + ethernetBytes);

        } catch (RemoteException ex) {
            ex.printStackTrace();
        } catch (NullPointerException ex) {
            ex.printStackTrace();
        }
        return ethernetBytes;
    }

    private int getSlotBySubscriberId(String subscriberId, ITelephonyEx iTel) {
        log(TAG, "getSlotBySubscriberId= " + subscriberId);
        if (subscriberId == null)
            return -1;

        for (int i = 0; i < PhoneConstants.GEMINI_SIM_NUM; i++) {
            try {
                if (iTel == null) {
                    log(TAG, "getSlotBySubscriberId  ITelephonyEx is not ready");
                    return -1;
                }
                if (iTel.hasIccCard(i) == true) {
                    String subId = iTel.getSubscriberId(i);
                    if (Objects.equal(subscriberId, subId))
                        return i;
                }
            } catch (RemoteException e) {
                // ignored; service lives in system_server
            }
        }

        return -1;
    }

    private NetworkPolicy getNetworkPolicy(String subscriberId, NetworkPolicyManager NPM) {
        mContext.enforceCallingOrSelfPermission(MANAGE_NETWORK_POLICY, TAG);

        NetworkPolicy npolicy = null;
        if (subscriberId != "" && subscriberId != null) {
            ITelephonyEx iTel = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
            int slotId = getSlotBySubscriberId(subscriberId, iTel);

            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                if (slotId < 0) {
                    log(TAG, slotId + " <= getSlotBySubscriberId(" + subscriberId + ") " + iTel);
                    return npolicy;
                }
                int state = TelephonyManagerEx.getDefault().getSimState(slotId);
                if (state != SIM_STATE_READY) {
                    log(TAG, "getNetworkPolicy getSimState != SIM_STATE_READY no action SlotId="
                                    + slotId + " subscriberId="
                                    + subscriberId);
                    return npolicy;
                }
            } else {
                TelephonyManager telephony = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
                if (telephony.getSimState() != SIM_STATE_READY) {
                    log(TAG, "getNetworkPolicy getSimState != SIM_STATE_READY no action subscriberId="
                                    + subscriberId);
                    return npolicy;
                }
            }

            if (NPM != null) {
                NetworkPolicy[] networkpolicies = NPM.getNetworkPolicies();
                for (NetworkPolicy policy : networkpolicies) {
                    if (policy.template.getSubscriberId() == null)
                        continue;

                    if (subscriberId.equals(policy.template.getSubscriberId())) {
                        log(TAG, "getNetworkPolicy Hit policy:" + policy);
                        npolicy = policy;
                    }
                }
            }
        }
        return npolicy;
    }

    private void updateDataUsageView() {
        int usageSize;

        // Single SIM
        if (FeatureOption.MTK_GEMINI_SUPPORT == false) {
            if (mSim1ShowLockScreen == 1 && mSlot1 > -1) {
                updateCardData(Constants.SINGLE_CARD);
            }
        } else { // Gemini support
            switch (mCardCount) {
                case Constants.ONE_CARD:
                    if (mSim2ShowLockScreen == 1 && mSlot2 > -1) {
                        updateCardData(Constants.GEMINI_CARD_TWO);
                    } else if (mSim1ShowLockScreen == 1 && mSlot1 > -1) {
                        updateCardData(Constants.GEMINI_CARD_ONE);
                    }
                    break;
                case Constants.TWO_CARD:
                    if (mSim1ShowLockScreen == 1 && mSlot1 > -1) {
                        updateCardData(Constants.GEMINI_CARD_ONE);
                    }

                    if (mSim2ShowLockScreen == 1 && mSlot2 > -1) {
                        updateCardData(Constants.GEMINI_CARD_TWO);
                    }
                    break;
            }
        }
    }

    private void updateCardData(int cardtype) {
        int usageSize;

        switch (cardtype) {
            case Constants.SINGLE_CARD:
            case Constants.GEMINI_CARD_ONE:
                if (mCard1 == null)
                    break;

                if (0 == getCurrentDataUsage(mSimId1, mSlot1)) {
                    mCard1.setBubbleSize(0);
                } else {
                    if (cardtype == Constants.SINGLE_CARD)
                        usageSize = setBubbleView(Constants.SINGLE_CARD);
                    else
                        usageSize = setBubbleView(Constants.GEMINI_CARD_ONE);
                    mCard1.setBubbleSize(usageSize);
                }
                break;
            case Constants.GEMINI_CARD_TWO:
                if (mCard2 == null)
                    break;

                if (0 == getCurrentDataUsage(mSimId2, mSlot2)) {
                    mCard2.setBubbleSize(0);
                } else {
                    usageSize = setBubbleView(Constants.GEMINI_CARD_TWO);
                    mCard2.setBubbleSize(usageSize);
                }
                break;
            default:
                break;
        }
    }

    // return data usage size
    private int setBubbleView(int cardNumber) {
        long dataLimit = 0;
        long dataUsage = 0;
        int usageSize = 10;
        double answer;

        switch (cardNumber) {
            case Constants.SINGLE_CARD:
            case Constants.GEMINI_CARD_ONE:
                if (mCard1Limit > 0) {
                    dataLimit = mCard1Limit / Constants.KB_UNIT;
                    dataUsage = getCurrentDataUsage(mSimId1, mSlot1) / Constants.KB_UNIT;
                    log(TAG, "CARD_ONE:dLimit=" + dataLimit + ", dUsage=" + dataUsage);

                    if (dataUsage == 0) {
                        return 0;
                    }
                }
                break;
            case Constants.GEMINI_CARD_TWO:
                if (mCard2Limit > 0) {
                    dataLimit = mCard2Limit / Constants.KB_UNIT;
                    dataUsage = getCurrentDataUsage(mSimId2, mSlot2) / Constants.KB_UNIT;
                    log(TAG, "CARD_TWO:dLimit=" + dataLimit + ", dUsage=" + dataUsage);
                    if (dataUsage == 0) {
                        return 0;
                    }
                }
                break;
            default:
                break;
        }

        // Formula
        // dataUsage : dataLimit = usageSize : 100
        // -> usageSize = (dataUsage * 100) / dataLimit)
        answer = (float) ((float) (dataUsage * 100) / (float) dataLimit);
        log(TAG, "return->usageSize=" + answer);

        if (answer < 1 && answer > 0)
            usageSize = 1;
        else
            usageSize = (int) answer;

        return usageSize;
    }

    // round double value "1.2345 -> 1.23"
    private double round(double value, int type) {
        BigDecimal bd = new BigDecimal(value);
        bd = bd.setScale(type, BigDecimal.ROUND_HALF_UP);
        double d = bd.doubleValue();
        bd = null;

        return d;
    }

    private void showUsageText(boolean isTextOne) {
        double cardLimit;
        double cardUsage;

        // get current data usage
        log(TAG, "showDataUsageText->isTextOne:" + isTextOne);
        if (isTextOne) {
            cardLimit = (double) mCard1Limit;
            cardUsage = (double) getCurrentDataUsage(mSimId1, mSlot1);
        } else {
            cardLimit = (double) mCard2Limit;
            cardUsage = (double) getCurrentDataUsage(mSimId2, mSlot2);
        }
        log(TAG, "cardLimit(bytes)=" + cardLimit + " cardUsage(bytes)=" + cardUsage);

        // cardLimit->GB
        if (cardLimit >= Constants.GB_UNIT) {
            cardLimit = cardLimit / Constants.GB_UNIT;

            // cardUsage->GB
            if (cardUsage >= Constants.GB_UNIT) {
                cardUsage = cardUsage / Constants.GB_UNIT;
                if (isTextOne)
                    mCard1.getText().setText(round(cardUsage, 2) + "/" + round(cardLimit, 1) + "GB");
                else
                    mCard2.getText().setText(round(cardUsage, 2) + "/" + round(cardLimit, 1) + "GB");

            // cardUsage->MB
            } else if (cardUsage >= Constants.MB_UNIT && cardUsage < Constants.GB_UNIT) {
                cardUsage = cardUsage / Constants.GB_UNIT;
                if (isTextOne)
                    mCard1.getText().setText(round(cardUsage, 3) + "/" + round(cardLimit, 1) + "GB");
                else
                    mCard2.getText().setText(round(cardUsage, 3) + "/" + round(cardLimit, 1) + "GB");

            // cardUsage->KB
            } else if (cardUsage >= Constants.KB_UNIT && cardUsage < Constants.MB_UNIT) {
                cardUsage = cardUsage / Constants.KB_UNIT;
                if (isTextOne)
                    mCard1.getText().setText((int) cardUsage + "KB/" + round(cardLimit, 1) + "GB");
                else
                    mCard2.getText().setText((int) cardUsage + "KB/" + round(cardLimit, 1) + "GB");
            } else {
                if (isTextOne)
                    mCard1.getText().setText("0/" + round(cardLimit, 1) + "GB");
                else
                    mCard2.getText().setText("0/" + round(cardLimit, 1) + "GB");
            }

        } else { // cardLimit -> MB

            // cardUsage -> GB
            if (cardUsage >= Constants.GB_UNIT) {
                cardUsage = cardUsage / Constants.GB_UNIT;
                cardLimit = cardLimit / Constants.GB_UNIT;
                if (isTextOne)
                    mCard1.getText().setText(round(cardUsage, 1) + "/" + round(cardLimit, 3) + "GB");
                else
                    mCard2.getText().setText(round(cardUsage, 1) + "/" + round(cardLimit, 3) + "GB");

            // cardUsage -> MB
            } else if (cardUsage >= Constants.MB_UNIT && cardUsage < Constants.GB_UNIT) {
                cardUsage = cardUsage / Constants.MB_UNIT;
                cardLimit = cardLimit / Constants.MB_UNIT;
                if (isTextOne)
                    mCard1.getText().setText((int) cardUsage + "/" + (int) cardLimit + "MB");
                else
                    mCard2.getText().setText((int) cardUsage + "/" + (int) cardLimit + "MB");

            // cardUsage -> KB (< 1MB)
            } else if (cardUsage >= Constants.KB_UNIT && cardUsage < Constants.MB_UNIT) {
                cardUsage = cardUsage / Constants.MB_UNIT;
                cardLimit = cardLimit / Constants.MB_UNIT;
                if (isTextOne)
                    mCard1.getText().setText(round(cardUsage, 3) + "/" + (int) cardLimit + "MB");
                else
                    mCard2.getText().setText(round(cardUsage, 3) + "/" + (int) cardLimit + "MB");

            } else {
                cardLimit = cardLimit / Constants.MB_UNIT;
                if (isTextOne)
                    mCard1.getText().setText("0/" + (int) cardLimit + "MB");
                else
                    mCard2.getText().setText("0/" + (int) cardLimit + "MB");
            }
        }
    }

    // setting -> data usage -> switch button " Show on LockScreen"
    private void getShowLockScreen() {
        try {
            mSim1ShowLockScreen = Settings.System.getInt(mContext.getContentResolver(),
                    Settings.System.DATAUSAGE_ON_LOCKSCREEN_SIM1);
            mSim2ShowLockScreen = Settings.System.getInt(mContext.getContentResolver(),
                    Settings.System.DATAUSAGE_ON_LOCKSCREEN_SIM2);
            log(TAG, "mSim1ShowLockScreen=" + mSim1ShowLockScreen + " mSim2ShowLockScreen="
                    + mSim2ShowLockScreen);
        } catch (SettingNotFoundException e) {
            e.printStackTrace();
        }
    }

    // Initial SIM card parameter
    private void initialParameter() {
        mIsLandscapeMode = false;

        // set wall paper -> TRANSPARENT
        mStage.setBackgroundColor(Color.TRANSPARENT);

        // get data usage text W & H
        Bitmap textframe = BitmapFactory.decodeResource(getResources()
                , R.drawable.label_orange);
        mTextW = textframe.getWidth();
        mTextH = textframe.getHeight();

        mSimId1 = -1;
        mSimId2 = -1;
        mSlot1 = -1;
        mSlot2 = -1;
        mSim1ShowLockScreen = 0;
        mSim2ShowLockScreen = 0;

        mIsApkRun = false;

        mTextScale1 = getResources().getInteger(R.integer.SCALE_ONE_TEXT);
        mTextScale2 = getResources().getInteger(R.integer.SCALE_TWO_TEXT);
        mTitleBarH = getResources().getInteger(R.integer.TITLE_BAR_H);

        log(TAG, "mTextScale1: " + mTextScale1);
        log(TAG, "mTextScale2: " + mTextScale2);
        log(TAG, "mTitleBarH: " + mTitleBarH);

        int barHigh = getNavBarHeight(mContext);
        mTitleBarH = mTitleBarH + (barHigh - 96);
        log(TAG, "mTitleBarH: " + mTitleBarH);

        mLimitCircleMaxPx = getResources().getInteger(R.integer.LIMIT_CIRCLE_MAX_PX);
        mLimitCircleMinPx = getResources().getInteger(R.integer.LIMIT_CIRCLE_MIN_PX);

        log(TAG, "mLimitCircleMaxPx: " + mLimitCircleMaxPx);
        log(TAG, "mLimitCircleMinPx: " + mLimitCircleMinPx);

        // draw Date text -> Time
        Calendar c = Calendar.getInstance();
        mNowDate = c.get(Calendar.DAY_OF_MONTH);

    }

    public static int getNavBarHeight(Context context) {
        if (context == null)
            return -1;

        final boolean hasNavBar = context.getResources().getBoolean(
                                      com.android.internal.R.bool.config_showNavigationBar);
        final int navBarHeight = hasNavBar ? context.getResources().getDimensionPixelSize(
                                      com.android.internal.R.dimen.navigation_bar_height) : 0;
        return navBarHeight;
    }

    private void startCircleFloating() {
        log(TAG, "startCircleFloating, mCardCount: " + mCardCount);
        if (mCardCount == Constants.ONE_CARD) {
            if (mSlot1 > -1 && mSim1ShowLockScreen == 1) {
                simcard1Floating(Constants.ONE_CARD);
                log(TAG, "card 1");
            } else if (mSlot2 > -1 && mSim2ShowLockScreen == 1) {
                simcard2Floating(Constants.ONE_CARD);
                log(TAG, "card 2");
            }
        } else if (mCardCount == Constants.TWO_CARD) {
            log(TAG, "mSim1ShowLockScreen:" + mSim1ShowLockScreen + ", Sim2:" + mSim2ShowLockScreen);
            if (mSim1ShowLockScreen == 1 && mSim2ShowLockScreen == 1) {
                if (mCard1LimitPx > 0 && mCard2LimitPx > 0) {
                    simcard1Floating(Constants.TWO_CARD);
                    simcard2Floating(Constants.TWO_CARD);
                } else if (mCard1LimitPx > 0 && mCard2LimitPx == 0) {
                    simcard1Floating(Constants.ONE_CARD);
                } else if (mCard2LimitPx > 0 && mCard1LimitPx == 0) {
                    simcard2Floating(Constants.ONE_CARD);
                } else {
                    // do nothing: no start floating
                }
            } else if (mSim1ShowLockScreen == 1 && mSim2ShowLockScreen == 0) {
                simcard1Floating(Constants.ONE_CARD);
            } else if (mSim1ShowLockScreen == 0 && mSim2ShowLockScreen == 1) {
                simcard2Floating(Constants.ONE_CARD);
            } else {
                // shouldn't be here.
            }
        } else {
            // no card inserted
        }
    }

    private boolean initSimCardData() {
        boolean isInitSimData = true;
        int card1UsageSize;
        int card2UsageSize;

        if (FeatureOption.MTK_GEMINI_SUPPORT == false) { // Single SIM
            log(TAG, "initSimCardData - Single SIM");
            if (mSim1ShowLockScreen == 1 && mCard1LimitPx > 0) {
                card1UsageSize = setBubbleView(Constants.SINGLE_CARD);
                mCard1 = new SIMCardData(mContext, Constants.SINGLE_CARD, mCard1ColorNum,
                        mCard1LimitPx, card1UsageSize, mCardCount, mIsLandscapeMode);
                getCard1Value();
            } else {
                log(TAG, "Single SIM->return this;");
                isInitSimData = false;
                if (mSim1ShowLockScreen == 0 )
                    stopListenState();
            }

        } else { // Gemini support
            log(TAG, "initSimCardData - GEMINI support, mCardCount:" + mCardCount);
            log(TAG, "initSimCardData - mSim1ShowLockScreen:" + mSim1ShowLockScreen + ",mCard1LimitPx:" + mCard1LimitPx);
            log(TAG, "initSimCardData - mSim2ShowLockScreen:" + mSim2ShowLockScreen + ",mCard2LimitPx:" + mCard2LimitPx);
            switch (mCardCount) {
                case Constants.ONE_CARD:
                    if (mSim1ShowLockScreen == 1 && mCard1LimitPx > 0) {
                        card1UsageSize = setBubbleView(Constants.GEMINI_CARD_ONE);
                        mCard1 = new SIMCardData(mContext, Constants.GEMINI_CARD_ONE, mCard1ColorNum
                                , mCard1LimitPx, card1UsageSize, Constants.ONE_CARD, mIsLandscapeMode);
                        getCard1Value();
                    } else if (mSim2ShowLockScreen == 1 && mCard2LimitPx > 0) {
                        card2UsageSize = setBubbleView(Constants.GEMINI_CARD_TWO);
                        mCard2 = new SIMCardData(mContext, Constants.GEMINI_CARD_TWO, mCard2ColorNum
                                , mCard2LimitPx, card2UsageSize, Constants.ONE_CARD, mIsLandscapeMode);
                        getCard2Value();
                    } else {
                        log(TAG, "Constants.ONE_CARD->return this;");
                        isInitSimData = false;
                        if((mSim1ShowLockScreen == 0 && mSlot1 >-1) || (mSim2ShowLockScreen == 0 && mSlot2 >-1))
                            stopListenState();
                    }
                    break;

                case Constants.TWO_CARD:
                    if (mSim1ShowLockScreen == 1 && mSim2ShowLockScreen == 1) {
                        if (mCard1LimitPx > 0 && mCard2LimitPx > 0) {
                            card1UsageSize = setBubbleView(Constants.GEMINI_CARD_ONE);
                            mCard1 = new SIMCardData(mContext, Constants.GEMINI_CARD_ONE, mCard1ColorNum
                                    , mCard1LimitPx, card1UsageSize, Constants.TWO_CARD, mIsLandscapeMode);
                            getCard1Value();

                            card2UsageSize = setBubbleView(Constants.GEMINI_CARD_TWO);
                            mCard2 = new SIMCardData(mContext, Constants.GEMINI_CARD_TWO,mCard2ColorNum
                                    , mCard2LimitPx, card2UsageSize, Constants.TWO_CARD, mIsLandscapeMode);
                            getCard2Value();

                        } else if (mCard1LimitPx > 0 && mCard2LimitPx == 0) {
                            card1UsageSize = setBubbleView(Constants.GEMINI_CARD_ONE);
                            mCard1 = new SIMCardData(mContext, Constants.GEMINI_CARD_ONE, mCard1ColorNum
                                    , mCard1LimitPx, card1UsageSize, Constants.ONE_CARD, mIsLandscapeMode);
                            getCard1Value();
                        } else if (mCard1LimitPx == 0 && mCard2LimitPx > 0) {
                            card2UsageSize = setBubbleView(Constants.GEMINI_CARD_TWO);
                            mCard2 = new SIMCardData(mContext, Constants.GEMINI_CARD_TWO, mCard2ColorNum
                                    , mCard2LimitPx, card2UsageSize, Constants.ONE_CARD, mIsLandscapeMode);
                            getCard2Value();
                        } else {
                            log(TAG, "Constants.TWO_CARD->return this;");
                            isInitSimData = false;
                        }
                    } else if (mSim1ShowLockScreen == 1 && mSim2ShowLockScreen == 0) {
                        if (mCard1LimitPx > 0) {
                            card1UsageSize = setBubbleView(Constants.GEMINI_CARD_ONE);
                            mCard1 = new SIMCardData(mContext, Constants.GEMINI_CARD_ONE, mCard1ColorNum
                                    , mCard1LimitPx, card1UsageSize, Constants.ONE_CARD, mIsLandscapeMode);
                            getCard1Value();
                        } else {
                            log(TAG, "Constants.TWO_CARD->card 1->return this;");
                            isInitSimData = false;
                        }

                    } else if (mSim1ShowLockScreen == 0 && mSim2ShowLockScreen == 1) {
                        if (mCard2LimitPx > 0) {
                            card2UsageSize = setBubbleView(Constants.GEMINI_CARD_TWO);
                            mCard2 = new SIMCardData(mContext, Constants.GEMINI_CARD_TWO, mCard2ColorNum
                                    , mCard2LimitPx, card2UsageSize, Constants.ONE_CARD, mIsLandscapeMode);
                            getCard2Value();
                        } else {
                            log(TAG, "Constants.TWO_CARD->card 2->return this;");
                            isInitSimData = false;
                        }

                    } else { // no need show both for sim1 & sim2
                        log(TAG, "Constants.TWO_CARD->NO show->return this;");
                        isInitSimData = false;
                        stopListenState();
                    }
                    break;

                default:
                    // no card inserted
                    log(TAG, "GEMINI support->no card inserted->return this");
                    isInitSimData = false;
                    stopListenState();
                    break;
            }
        }

        // get data usage floating bound
        if (isInitSimData) {
            if (mCard1 != null) {
                mBoundBufferX = mCard1.getBoundBufferX();
                mBoundBufferY = mCard1.getBoundBufferY();
            } else {
                mBoundBufferX = mCard2.getBoundBufferX();
                mBoundBufferY = mCard2.getBoundBufferY();
            }
        }

        log(TAG, "initSimCardData->isInitSimData=" + isInitSimData);
        return isInitSimData;
    }

    private void getCard1Value() {
        mCard1Scale = mCard1.getScale();
        mCard1OldX = mCard1.mUsageFirstPointX;
        mCard1OldY = mCard1.mUsageFirstPointY;
    }

    private void getCard2Value() {
        mCard2Scale = mCard2.getScale();
        mCard2OldX = mCard2.mUsageFirstPointX;
        mCard2OldY = mCard2.mUsageFirstPointY;
    }

    private BroadcastReceiver mStatsReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            log(TAG, "mStatsReceiver action = " + action + ", mHandler = " + mHandler);

            synchronized (mSyncObjForHandler) {
                if (mHandler == null) {
                    return;
                }

                if (ACTION_NETWORK_STATS_UPDATED.equals(action)) {
                    Message msg = mHandler.obtainMessage(MSG_DATA_STATS_CHANGED);
                    if (null != msg) {
                        msg.sendToTarget();
                        log(TAG, "ACTION_NETWORK_STATS_UPDATED");
                    }
                }
            }
        }
    };

    private BroadcastReceiver mSimStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            log(TAG, "mSimStateReceiver action = " + action + ", mHandler = " + mHandler);

            synchronized(mSyncObjForHandler) {
                if (mHandler == null) {
                    return;
                }

                if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(action)) {
                    String stateExtra = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                    log(TAG, "ACTION_SIM_STATE_CHANGED " + stateExtra);
                    Message msg = null;
                    if (FeatureOption.MTK_GEMINI_SUPPORT) {
                        int slotId = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, PhoneConstants.GEMINI_SIM_1);
                        if (IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(stateExtra) ||
                            IccCardConstants.INTENT_VALUE_ICC_NOT_READY.equals(stateExtra)) {
                            msg = mHandler.obtainMessage(MSG_SIM_STATS_CHANGED, slotId, 0);
                            if (null != msg) {
                                msg.sendToTarget();
                            }
                        } else if (IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(stateExtra)) {
                            msg = mHandler.obtainMessage(MSG_SIM_STATS_CHANGED, slotId, 1);
                            if (null != msg) {
                                msg.sendToTarget();
                            }
                        }
                    } else {
                        if (IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(stateExtra) ||
                            IccCardConstants.INTENT_VALUE_ICC_NOT_READY.equals(stateExtra)) {
                            msg = mHandler.obtainMessage(MSG_SIM_STATS_CHANGED, 0, 0);
                            if (null != msg) {
                                msg.sendToTarget();
                            }
                        } else if (IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(stateExtra)) {
                            msg = mHandler.obtainMessage(MSG_SIM_STATS_CHANGED, 0, 1);
                            if (null != msg) {
                                msg.sendToTarget();
                            }
                        }
                    }
                }
            }
        }
    };

    // update Data Usage callback
    private Handler.Callback mHandlerCallback = new Handler.Callback() {
        public boolean handleMessage(Message msg) {
            boolean val = true;

            switch (msg.what) {
                case MSG_DATA_STATS_CHANGED:
                    synchronized (DataUsageStageView.this) {
                        if (mIsApkRun) {
                            // update circle
                            updateDataUsageView();
                            log(TAG, "mHandlerCallback->updateDataUsageView()");
                        }
                    }
                    break;
                case MSG_SIM_STATS_CHANGED:
                    final int slotId = msg.arg1;
                    final int simState = msg.arg2;
                    log(TAG, "mHandler MSG_SIM_STATE_CHANGED, slotId:" + slotId + ",simState:" + simState);
                    ServiceState mState = new ServiceState();
                    if (simState == 0) { // absent sim
                        mState.setState(ServiceState.STATE_POWER_OFF);
                    } else { // imsi ready
                        mState.setState(ServiceState.STATE_IN_SERVICE);
                    }
                    createInternal(slotId, mState);
                    break;
                case MSG_FLOATING_SIM1_ONE_CARD:
                    simcard1Floating(Constants.ONE_CARD);
                    break;
                case MSG_FLOATING_SIM1_TWO_CARD:
                    simcard1Floating(Constants.TWO_CARD);
                    break;
                case MSG_FLOATING_SIM2_ONE_CARD:
                    simcard2Floating(Constants.ONE_CARD);
                    break;
                case MSG_FLOATING_SIM2_TWO_CARD:
                    simcard2Floating(Constants.TWO_CARD);
                    break;
                default:
                    return false;
            }
            return val;
        }
    };

    // WakeLcok
    private Handler mHandlerWakeLock = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case TIMEOUT:
                    handleTimeout(msg.arg1);
                    return;
            }
        }
    };

    /**
     * Handles the message sent by {@link #pokeWakelock}
     *
     * @param seq used to determine if anything has changed since the message
     *            was sent
     * @see #TIMEOUT
     */
    private void handleTimeout(int seq) {
        if (seq == mWakelockSequence) {
            mWakeLock.release();
        }
    }

    // listen service state
    private void startListenState() {
        log(TAG, "startListenState()");

        // (TelephonyManagerEx) mContext.getSystemService(Context.TELEPHONY_SERVICEEX);
        // this behavior may get null, so replace it
        mTelephonyManager = TelephonyManagerEx.getDefault();

        log(TAG, "mTelephonyManager: " + mTelephonyManager);
        if (mTelephonyManager != null) {
            if (FeatureOption.MTK_GEMINI_SUPPORT) {
                mTelephonyManager.listen(mPhoneStateListener_1,
                        PhoneStateListener.LISTEN_SERVICE_STATE, PhoneConstants.GEMINI_SIM_1);
                mTelephonyManager.listen(mPhoneStateListener_2,
                        PhoneStateListener.LISTEN_SERVICE_STATE, PhoneConstants.GEMINI_SIM_2);
    
            } else {
                mTelephonyManager.listen(mPhoneStateListener_1, PhoneStateListener.LISTEN_SERVICE_STATE, PhoneConstants.GEMINI_SIM_1);
            }
        }
    }

    // stop listen service state
    private void stopListenState() {
        log(TAG, "stopListenState()");
        log(TAG, "mTelephonyManager: " + mTelephonyManager);
        if (mTelephonyManager != null) {
            if (FeatureOption.MTK_GEMINI_SUPPORT) { // gemini support
                mTelephonyManager.listen(mPhoneStateListener_1,
                        PhoneStateListener.LISTEN_NONE, PhoneConstants.GEMINI_SIM_1);
                mTelephonyManager.listen(mPhoneStateListener_2,
                        PhoneStateListener.LISTEN_NONE, PhoneConstants.GEMINI_SIM_2);
            } else { // single card
                mTelephonyManager.listen(mPhoneStateListener_1, PhoneStateListener.LISTEN_NONE, PhoneConstants.GEMINI_SIM_1);
            }
        }
    }
}
