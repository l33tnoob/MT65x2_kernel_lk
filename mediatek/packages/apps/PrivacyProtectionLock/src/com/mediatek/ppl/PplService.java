package com.mediatek.ppl;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.app.Notification;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.common.ppl.IPplManager;
import com.mediatek.ppl.MessageManager.PendingMessage;
import com.mediatek.ppl.MessageManager.Type;

/**
 * Control service of PPL. Configuration activities bind to this service to access the control data, while Keyguard
 * binds to this service to request unlock operation after user inputs unlock password. It monitors SIM status and lock
 * the phone if lock criteria is satisfied. 
 * User sent remote SMS instructions are filtered by our plug-in in framework and are sent to this service via
 * startService(). This service will then execute the corresponding operation respectively and will broadcast LOCK,
 * UNLOCK and WIPE event to related components.
 */
public class PplService extends Service {
    private static final String TAG = "PPL/PPLService";

    /**
     * A collection of intents we used.
     */
    public static class Intents {
        // OUT: Send commands to listeners
        public static final String INTENT_NOTIFY_WIPE = "com.mediatek.ppl.NOTIFY_WIPE";
        public static final String INTENT_NOTIFY_MOUNT_SERVICE_WIPE = "com.mediatek.ppl.NOTIFY_MOUNT_SERVICE_WIPE";
        public static final String INTENT_NOTIFY_LOCK = "com.mediatek.ppl.NOTIFY_LOCK";
        public static final String INTENT_NOTIFY_UNLOCK = "com.mediatek.ppl.NOTIFY_UNLOCK";
        public static final String NOTIFICATION_KEY_IS_SIM_LOCK = "SimLock";

        // IN: Supported requests. The requester should provide password as an argument.
        public static final String INTENT_REQUEST_WIPE = "com.mediatek.ppl.REQUEST_WIPE";
        public static final String INTENT_REQUEST_LOCK = "com.mediatek.ppl.REQUEST_LOCK";
        public static final String INTENT_REQUEST_UNLOCK = "com.mediatek.ppl.REQUEST_UNLOCK";
        public static final String REQUEST_KEY_PASSWORD = "Password";
        public static final String INTENT_MOUNT_SERVICE_WIPE_RESPONSE = "com.mediatek.ppl.MOUNT_SERVICE_WIPE_RESPONSE";

        public static final String INTENT_REMOTE_INSTRUCTION_RECEIVED = "com.mediatek.ppl.REMOTE_INSTRUCTION_RECEIVED";
        public static final String INSTRUCTION_KEY_TYPE = "Type";
        public static final String INSTRUCTION_KEY_FROM = "From";
        public static final String INSTRUCTION_KEY_TO = "To";
        public static final String INSTRUCTION_KEY_SIM_ID = "SimId";

        public static final String INTENT_ENABLE = "com.mediatek.ppl.ENABLE";
        public static final String INTENT_DISABLE = "com.mediatek.ppl.DISABLE";

        public static final String PPL_MANAGER_SERVICE = "com.mediatek.ppl.service";

        public static final String UI_QUIT_SETUP_WIZARD = "com.mediatek.ppl.UI_QUIT_SETUP_WIZARD";
        public static final String UI_NO_SIM = "com.mediatek.ppl.UI_NO_SIM";
    }

    private PplManager mPPLManager = null;
    private EventReceiver mReceiver = null;
    private InternalControllerBinder mPrivateBinder = null;
    private SimTracker mSimTracker = null;
    private Handler mHandler = null;
    private List<Activity> mSensitiveActivityList = null;
    private boolean mPendingMessageProcessed = false;
	private boolean[] mSimReady;
	public static final String ACTION_SIM_STATE_CHANGED = "android.intent.action.SIM_STATE_CHANGED";

    public static final int MAX_CONTACTS = 3;

    @Override
    public void onCreate() {
//        stayForeground();
        mHandler = new Handler();
        // mSimTracker should be initialized before EventReceiver is registered
        mSimTracker = new SimTracker(PlatformManager.SIM_NUMBER, this);
        mSimReady = new boolean[mSimTracker.slotNumber];
        // TODO Should we also remove needLock() from ExternalControllerBinder ?
        // mSimTracker.takeSnapshot();
        mReceiver = new EventReceiver();
        mReceiver.initialize();
        mPPLManager = new PplManager(this, mSimTracker);
        mPrivateBinder = new InternalControllerBinder();
        mSensitiveActivityList = new ArrayList<Activity>();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand(" + intent + ")");
        if (intent == null) {
            return START_NOT_STICKY;
        }

        String action = intent.getAction();
        if (action == null) {
            return START_NOT_STICKY;
        }

        Log.d(TAG, "action is " + action);

        if (action.equals(Intents.INTENT_ENABLE)) {
            Log.i(TAG, "Privacy Protection Lock is enabled.");
            return START_STICKY;
        } else if (!mPPLManager.isEnabled()) {
            Log.i(TAG, "Privacy Protection Lock is disabled. Exit.");
            stopSelf();
            return START_NOT_STICKY;
        } else {
            if (action.equals(Intents.INTENT_REMOTE_INSTRUCTION_RECEIVED)) {
                byte type = intent.getByteExtra(Intents.INSTRUCTION_KEY_TYPE, MessageManager.Type.INVALID);
                String from = intent.getStringExtra(Intents.INSTRUCTION_KEY_FROM);
                String to = intent.getStringExtra(Intents.INSTRUCTION_KEY_TO);
                int simId = intent.getIntExtra(Intents.INSTRUCTION_KEY_SIM_ID, -1);
                Log.d(TAG, "Remote instruction: " + type + ", " + from + ", " + to + ", " + simId);
                if (type == MessageManager.Type.INVALID || simId == -1) {
                    throw new Error("Invalid instruction broadcast");
                }
                switch (type) {
                case Type.LOCK_REQUEST:
                    if (from == null) {
                        throw new Error("Invalid instruction broadcast");
                    }
                    mPPLManager.lock(false, simId, from);
                    break;
                case Type.RESET_PW_REQUEST:
                    if (from == null) {
                        throw new Error("Invalid instruction broadcast");
                    }
                    mPPLManager.resetPassword(simId, from);
                    break;
                case Type.UNLOCK_REQUEST:
                    if (from == null) {
                        throw new Error("Invalid instruction broadcast");
                    }
                    mPPLManager.unlock(null, simId, from);
                    break;
                case Type.WIPE_REQUEST:
                    if (from == null) {
                        throw new Error("Invalid instruction broadcast");
                    }
                    mPPLManager.wipe(simId, from);
                    break;
                default:
                    Log.w(TAG, "Unsupported remote instruction type");
                    break;
                }
            } else if (action.equals(Intents.INTENT_DISABLE)) {
                if (!mPPLManager.hasPendingMessageInNvram()) {
                    stopSelf();
                }
            } else if (action.equals(Intent.ACTION_BOOT_COMPLETED)) {
                Log.w(TAG, "Boot complete start");
                performBootCheck();
                Log.w(TAG, "Boot complete end");
            } else if (action.equals(PplService.ACTION_SIM_STATE_CHANGED)) {
            	Log.w(TAG, PplService.ACTION_SIM_STATE_CHANGED + " received");
            	performBootCheck();
            	if (!mPPLManager.hasPendingWipeRequest()) {
                    String state = intent.getStringExtra("ss");
                    int simId = intent.getIntExtra("simId", -1);
                    Log.d(TAG, "stateExtra = " + state + ", simId = " + simId);
                    if (simId != -1 && state != null && !"NOT_READY".equals(state)) {
                        mSimReady[simId] = true;
                    }
                    if (allReady()) {
                        mPPLManager.onSimStateChanged();
                    	if (!mPendingMessageProcessed) {
                    		mPPLManager.processPendingMessagesInNvram();
                    		mPendingMessageProcessed = true;
                    	}
                    }
            	}
            } else {
                Log.w(TAG, "Unsupported action: " + action);
            }
        }

        return START_STICKY;
    }

    private void performBootCheck() {
        // Check lock first.
        int lockFlag = mPPLManager.needLock();
        Log.w(TAG, "lockFlag is " + lockFlag);
        if (lockFlag != PplManager.NO_LOCK) {
            mPPLManager.doLock((lockFlag & PplManager.SIM_LOCK) == PplManager.SIM_LOCK);
        }
        if (mPPLManager.hasPendingWipeRequest()) {
        	Log.w(TAG, "Wiping...do nothing");
        	return;
        }
        Log.w(TAG, "hasPendingWipeRequest? " + mPPLManager.hasPendingWipeFlag());
        // Check for pending wipe operation.
        if (mPPLManager.hasPendingWipeFlag()) {
            Log.w(TAG, "wipeSucceeded? " + mPPLManager.wipeSucceeded());
            if (mPPLManager.wipeSucceeded()) {
                mPPLManager.clearPendingWipeFlag();
            } else {
                mPPLManager.wipe();
            }
        }
    }

    /*
     * Check whether all the SIM slots are ready. By "ready", we mean we can check whether there is SIM card is
     * inserted.
     */
    private boolean allReady() {
        String s = "mSimReady:";
        for (int i = 0; i < mSimReady.length; ++i) {
            s += mSimReady[i];
            s += ", ";
        }
        Log.d(TAG, s);
        for (int i = 0; i < mSimReady.length; ++i) {
            if (!mSimReady[i]) {
                return false;
            }
        }
        return true;
    }

    @Override
    public IBinder onBind(Intent intent) {
        if (intent == null) {
            return null;
        }
        String action = intent.getAction();
        Log.d(TAG, "Bind request of action: " + action);
        if (action == null || !action.equals(Intents.PPL_MANAGER_SERVICE)) {
            return null;
        }

        String from = intent.getPackage();
        Log.d(TAG, "Bind request from " + from);

        if (from != null && from.equals("com.mediatek.ppl")) {
            return mPrivateBinder;
        } else {
            return new ExternalControllerBinder();
        }
    }

    @Override
    public void onDestroy() {
        mReceiver.destroy();
        mSimReady = null;
        // TODO restart service
    }

    private class EventReceiver extends BroadcastReceiver {
        public void initialize() {
            IntentFilter filter = new IntentFilter();
            filter.addAction(Intents.INTENT_MOUNT_SERVICE_WIPE_RESPONSE);
            filter.addAction(Intent.ACTION_SCREEN_OFF);
            registerReceiver(this, filter);

            filter = new IntentFilter();
            filter.addAction(MessageManager.SMS_SENT_ACTION);
            filter.addDataAuthority(MessageManager.SMS_PENDING_INTENT_DATA_AUTH, null);
            filter.addDataScheme(MessageManager.SMS_PENDING_INTENT_DATA_SCHEME);
            registerReceiver(this, filter);
        }

        public void destroy() {
            unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "EventReceiver: onReceive(" + intent + ")");
            String action = intent.getAction();
            if (action == null) {
                return;
            }
            Log.d(TAG, "EventReceiver: action is " + action);

            if (action.equals(MessageManager.SMS_SENT_ACTION)) {
                mPPLManager.processMessageSentAction(intent, getResultCode());
            } else if (action.equals(Intents.INTENT_MOUNT_SERVICE_WIPE_RESPONSE)) {
                mPPLManager.checkPendingWipeRequest();
            } else if (action.equals(Intent.ACTION_SCREEN_OFF)) {
                for (Activity activity : mSensitiveActivityList) {
                    activity.finish();
                }
            }
        }

    }

    /**
     * Binder used by internal clients such as activities.
     */
    public class InternalControllerBinder extends Binder {

        /**
         * Get the edit buffer of ControlData. The buffer may contain previously editing result.
         * 
         * @return edit buffer in PPLManager
         */
        public ControlData startEdit() {
            return mPPLManager.startEdit();
        }

        /**
         * Load the current system control data to the edit buffer.
         */
        public void loadCurrentData() {
            mPPLManager.loadCurrentData();
        }

        /**
         * Finish the editing and perform extra actions according to the parameter specified.
         * 
         * @param action PPLManager.ACTION_KEEP   Keep the editing result and leave it to the buffer for later access.
         *               PPLManager.ACTION_COMMIT Commit the editing result to system control data.
         *               PPLManager.ACTION_CLEAR  Discard previous editing result with empty control data.
         */
        public void finishEdit(int action) {
            mPPLManager.finishEdit(action);
        }

        /**
         * Enable PPL.
         * 
         * @param updateSimInfo Whether update the SIM information with the values in current system.
         */
        public void enable(boolean updateSimInfo) {
            mPPLManager.enable(null, updateSimInfo);
        }

        /**
         * Disable PPL.
         */
        public void disable() {
            mPPLManager.disable();
        }

        /**
         * Check whether the password is correct.
         * 
         * @param password
         * @return
         */
        public boolean verifyPassword(String password) {
            return mPPLManager.verifyPassword(password.getBytes());
        }

        /**
         * Send instruction description message.
         * 
         * @param simId The SIM user selected via UI.
         */
        public void sendInstructionDescriptionMessage(int simId) {
            Log.d(TAG, "sendInstructionDescriptionMessage(" + simId + ")");
            mPPLManager.sendMessage(MessageManager.Type.INSTRUCTION_DESCRIPTION, null, simId,
                    mPPLManager.buildMessage(MessageManager.Type.INSTRUCTION_DESCRIPTION));
        }

        /**
         * Get a list of the IDs of current inserted SIM cards.
         * 
         * @return
         */
        public int[] getInsertedSim() {
            mSimTracker.takeSnapshot();
            return mSimTracker.getInsertedSim();
        }
        
        public boolean isEnabled() {
            return mPPLManager.isEnabled();
        }
        
        public void registerSensitiveActivity(Activity activity) {
            if (!mSensitiveActivityList.contains(activity)) {
                mSensitiveActivityList.add(activity);
            }
        }
        
        public void unregisterSensitiveActivity(Activity activity) {
            mSensitiveActivityList.remove(activity);
        }
    }

    /**
     * Binder used by external clients such as Keyguard.
     */
    private class ExternalControllerBinder extends IPplManager.Stub {

        /**
         * Reset the password. It is provided here for possible future requirement to reset password via lock screen.
         */
        @Override
        public void resetPassword() throws RemoteException {
            Log.d(TAG, "resetPassword()");
            mPPLManager.resetPassword(-1, null);
        }

        /**
         * Check whether we should lock the phone.
         */
        @Override
        public int needLock() throws RemoteException {
            Log.d(TAG, "needLock()");
            return mPPLManager.needLock();
        }

        /**
         * Lock the phone.
         */
        @Override
        public void lock() throws RemoteException {
            Log.d(TAG, "lock()");
            mPPLManager.lock(false, -1, null);
        }

        /**
         * Interface for Keyguard to unlock the phone.
         * 
         * @param password  The password used to unlock the phone. If the password is null, then return false;
         * @return
         */
        @Override
        public boolean unlock(String password) throws RemoteException {
            Log.d(TAG, "unlock(" + password + ")");
            if (password == null) {
                return false;
            } else {
                return mPPLManager.unlock(password.getBytes(), PendingMessage.INVALID_SIM_ID, null);
            }
        }
    }

    /**
     * Show the toast message.
     * 
     * @param message   The message to show.
     */
    void showToast(final String message) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                Toast toast = Toast.makeText(
                        PplService.this,
                        message,
                        Toast.LENGTH_SHORT);
                toast.show();
            }
        });
    }

    /**
     * Run the Runnable r after delayMillis milliseconds.
     * 
     * @param r
     * @param delayMillis
     */
    void postRun(Runnable r, long delayMillis) {
        mHandler.postDelayed(r, delayMillis);
    }
    
//    private void stayForeground() {
//        startForeground(1, new Notification());
//    }
}
