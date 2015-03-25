package com.mediatek.ppl;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;
import java.util.Random;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds;
import android.provider.ContactsContract.PhoneLookup;
import android.telephony.SmsManager;
import android.util.Log;

import com.android.internal.os.storage.ExternalStorageFormatter;
import com.mediatek.common.ppl.IPplAgent;
import com.mediatek.ppl.MessageManager.PendingMessage;
import com.mediatek.ppl.PplService.Intents;

public class PplManager {
    private static final String TAG = "PPL/PPLManager";

    private final PplService mService;
    private IPplAgent mAgent;
    private ControlData mEditBuffer;
    private List<PendingMessage> mPendingMessages;
    private ControlData mCache;
    private MessageManager mMessageManager;
    private SimTracker mSimTracker;
    private boolean mHasPendingWipeRequest;
    private PlatformManager mPlatformManager;
    private Intent mLockIntent = null;
    private byte[][] mLockedSimFingerPrintList = null;
    private Map<Long, Integer> mManualMessageResults = null;

    // the max length is set in layout.xml
    public static final int MAX_PASSWORD_LENGTH = 12;
    public static final int MIN_PASSWORD_LENGTH = 6;

    public static final int NO_LOCK = 0;
    public static final int NORMAL_LOCK = 1;
    public static final int SIM_LOCK = 2;

    public static final int ACTION_KEEP = 0;
    public static final int ACTION_COMMIT = 1;
    public static final int ACTION_CLEAR = 2;
    
    /* Interval for message resending in millisecond. */
    private static final long RETRY_INTERVAL = 10000;
    private static final long WIPE_DELAY = 10000; // millisecond
    /* File name used to check whether the wipe operation is completed. */
    private static final String WIPE_RESULT_INDICATOR = "WIPE_RESULT_INDICATOR";

    public PplManager(PplService service, SimTracker simTracker) {
        mService = service;
        mSimTracker = simTracker;
        mManualMessageResults = new HashMap<Long, Integer>();
        mHasPendingWipeRequest = false;
        mPlatformManager = PplApplication.getPlatformManager();
        mPendingMessages = new ArrayList<PendingMessage>();
        mAgent = PplApplication.getPlatformManager().getPPLAgent();
        mMessageManager = new MessageManager(service);
        mEditBuffer = new ControlData();
        try {
            mCache = ControlData.buildControlData(mAgent.readControlData());
        } catch (RemoteException e) {
            throw new Error(e);
        }
    }

    private static byte[][] cloneSimList(byte[][] input) {
        byte[][] result = null;
        if (input != null) {
            result = input.clone();
            for (int i = 0; i < input.length; ++i) {
                if (input[i] != null) {
                    result[i] = input[i].clone();
                }
            }
        }
        return result;
    }

    /**
     * Check whether PPL is enabled.
     * 
     * @return
     */
    public boolean isEnabled() {
        return mCache.isEnabled();
    }

    /**
     * Lock the phone and send messages to emergency contacts. This method should only be invoked when PPL is enabled.
     * 
     * @param simLock   Whether the locked is triggered by SIM change.
     * @param simId     Which SIM card will be used to send the message. If simLock is true, then this parameter is
     *                  ignored and the message will be sent via all SIM cards.
     * @param toNumber  Which number the message will be sent to. If simLock is true, then this parameter is ignored
     *                  and the message will be sent to all emergency contacts.
     */
    public synchronized void lock(boolean simLock, int simId, String toNumber) {
        Log.d(TAG, "lock(" + simLock + ", " + simId + ", " + toNumber + ")");
        if (!mCache.isEnabled()) {
            throw new Error("Cannot lock while it is disabled.");
        }
        if (simLock) {
            mCache.setSimLock(true);
            try {
                mAgent.writeControlData(mCache.encode());
            } catch (RemoteException e) {
                throw new Error(e);
            }
        } else {
            mCache.setLock(true);
            writeControlData(mCache);
        }

        doLock(simLock);

        if (simLock) {
            sendMessage(
                    MessageManager.Type.SIM_CHANGED,
                    null,
                    PendingMessage.ALL_SIM_ID,
                    mMessageManager.buildMessage(MessageManager.Type.SIM_CHANGED));
        } else {
            sendMessage(
                    MessageManager.Type.LOCK_RESPONSE,
                    toNumber,
                    simId,
                    mMessageManager.buildMessage(MessageManager.Type.LOCK_RESPONSE));
        }
    }

    /**
     * Lock the phone.
     * 
     * @param simLock   whether the locking is triggered by SIM change.
     */
    public synchronized void doLock(boolean simLock) {
        Intent intent = new Intent(PplService.Intents.INTENT_NOTIFY_LOCK);
        intent.putExtra(PplService.Intents.NOTIFICATION_KEY_IS_SIM_LOCK, simLock);
        if (mLockIntent != null) {
            mService.removeStickyBroadcast(mLockIntent);
        }
        mLockIntent = intent;
        mService.sendStickyBroadcast(intent);
        PplApplication.getPlatformManager().setMobileDataEnabled(false);
    }

    /**
     * Unlock the phone with password or unconditionally if password is null. You should not expose the unconditionally
     * unlocking interface to external invokers. This method will update the trusted SIM information stored if the
     * phone is unlocked successfully.
     * 
     * @param password   null if password check should be bypassed, the requested password otherwise.
     * @param simId      Which SIM card will be used to send the message. PendingMessage.INVALID_SIM_ID means we do not
     *                   send message.
     * @param toNumber   Which number the message will be sent to.
     * @return           Whether the phone is unlocked successfully. If PPL is enabled, then the phone will be unlocked
     *                   if and only if the password is correct or null. If PPL is not enabled, then this method will
     *                   always return false;
     */
    public synchronized boolean unlock(byte[] password, int simId, String toNumber) {
        if (mCache.isEnabled()) {
            if (password == null || checkPassword(password, mCache.salt, mCache.secret)) {
                mCache.setLock(false);
                mCache.setSimLock(false);
                updateSimInfo();
                writeControlData(mCache);
                if (mLockIntent != null) {
                    mService.removeStickyBroadcast(mLockIntent);
                    mLockIntent = null;
                }
                Intent intent = new Intent(PplService.Intents.INTENT_NOTIFY_UNLOCK);
                mService.sendBroadcast(intent);
                if (simId != PendingMessage.INVALID_SIM_ID) {
                    sendMessage(
                            MessageManager.Type.UNLOCK_RESPONSE,
                            toNumber,
                            simId,
                            mMessageManager.buildMessage(MessageManager.Type.UNLOCK_RESPONSE));
                }
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    /**
     * Start an edit session. The invoker will get an ControlData object as the edit buffer. The buffer can be used
     * across multiply edit sessions as long as you do not invoke finishEdit() with ACTION_CLEAR. The edit operated on
     * the buffer will be applied until you invoke finishEdit() with ACTION_COMMIT.
     * 
     * @return The edit buffer.
     */
    public synchronized ControlData startEdit() {
        Log.d(TAG, "startEdit()");
        if (mEditBuffer == null) {
            mEditBuffer = new ControlData();
        }
        Log.i(TAG, "mEditBuffer: " + mEditBuffer);
        Log.i(TAG, "mCache: " + mCache);
        return mEditBuffer;
    }

    /**
     * Load ControlData from storage to edit buffer. This will override the edit buffer entirely.
     */
    public synchronized void loadCurrentData() {
        mEditBuffer = mCache.clone();
    }

    /**
     * Finish current edit session. If action is ACTION_COMMIT, then the edit buffer will be written to storage.
     * NOTE: The pending message list in edit buffer will be ignored and the pending message list in storage will be
     *       cleared.
     * 
     * @param action    ACTION_KEEP:    leave the edit buffer as it is.
     *                  ACTION_COMMIT:  write the edit buffer to storage.
     *                  ACTION_CLEAR:   clear edit buffer.
     */
    public synchronized void finishEdit(int action) {
        Log.d(TAG, "finishEdit(" + action + ")");
        switch (action) {
        case ACTION_KEEP:
            break;
        case ACTION_COMMIT:
            mCache = mEditBuffer.clone();
            mCache.PendingMessageList = null;
            writeControlData(mCache);
            break;
        case ACTION_CLEAR:
            mEditBuffer = null;
            break;
        default:
            throw new Error("Unsupported action " + action);
        }
        Log.i(TAG, "mEditBuffer: " + mEditBuffer);
        Log.i(TAG, "mCache: " + mCache);
    }

    /**
     * Wipe the phone and send message to emergency contacts. This method will:
     *  0. Acquire partial wake lock. (We will not release as the phone will reboot later.)
     *  1. Create a indicator file for result checking after reboot.
     *  2. Set wipe pending flag for checking after reboot.
     *  3. Send message to emergency contacts.
     *  4. Turn off USB Mass Storage mode.
     *  5. Format SD cards.
     *  6. Reboot and do factory reset.
     * 
     * @param simId     Which SIM card will be used to send the message.
     * @param toNumber  Which number the message will be sent to.
     */
    public synchronized void wipe(int simId, String toNumber) {
        Log.d(TAG, "wipe(" + simId + ", " + toNumber + ")");
        // Acquire partial wake lock to prevent the phone from going to sleep when we are waiting for our posted
        // delayed runnable to run.
        PplApplication.getPlatformManager().acquireWakeLock();
        try {
            FileOutputStream fos = mService.openFileOutput(WIPE_RESULT_INDICATOR, Context.MODE_PRIVATE);
            fos.write(0xFF);
            fos.flush();
            fos.close();
        } catch (FileNotFoundException e) {
            throw new Error(e);
        } catch (IOException e) {
            throw new Error(e);
        }
        mCache.setWipeFlag(true);
        writeControlData(mCache);
        sendMessage(
                MessageManager.Type.WIPE_STARTED,
                toNumber,
                simId,
                mMessageManager.buildMessage(MessageManager.Type.WIPE_STARTED));
        // Completed message will be sent after reboot. So we save it as a pending message in NVRAM.
        // TODO Should we send it after wipe reboot?
        savePendingMessageToNvram(MessageManager.Type.WIPE_COMPLETED, toNumber);
        mService.postRun(new Runnable() {
			@Override
			public void run() {
		        wipe();
			}
        }, WIPE_DELAY);
    }

    /**
     * Wipe the phone. This method will:
     *  1. Turn off USB Mass Storage mode.
     *  2. Format SD cards.
     *  3. Reboot and do factory reset.
     *  
     *  NOTE: This method will not set flags or send message to emergency contacts. It is intended to be used for
     *        re-wiping after previous failure.
     *  NOTE: The actual wipe operation is triggered in executeWipe(). After the invocation of formatSDCard(), we will
     *        get the result via a broadcast. We will invoke executeWipe() in the receiver.
     */
    public synchronized void wipe() {
        mHasPendingWipeRequest = true;
        formatSDCard();
    }

    /**
     * Send the wipe broadcast and trigger factory reset. This will reboot the phone and wipe all the data in it.
     */
    private synchronized void executeWipe() {
        Intent intent = new Intent(PplService.Intents.INTENT_NOTIFY_WIPE);
        mService.sendBroadcast(intent);
        intent = new Intent(ExternalStorageFormatter.FORMAT_AND_FACTORY_RESET);
        intent.setComponent(ExternalStorageFormatter.COMPONENT_NAME);
        intent.putExtra("lawmo_wipe", true);
        mService.startService(intent);
        Log.d(TAG, "Factory reset command issued.");
        mHasPendingWipeRequest = false;
    }

    /**
     * Check whether this is a pending wipe request. If so, invoke executeWipe() to do the wipe. This method is
     * intended to be used in broadcast receiver for SD card formating result.
     */
    public synchronized void checkPendingWipeRequest() {
        if (mHasPendingWipeRequest) {
            executeWipe();
        }
    }
    
    public synchronized boolean hasPendingWipeRequest() {
    	return mHasPendingWipeRequest;
    }

    /**
     * Check whether the wipe operation is successful or not after reboot.
     * 
     * @return  The result of wiping.
     */
    public synchronized boolean wipeSucceeded() {
        File filesDir = mService.getFilesDir();
        File indicatorFile = new File(filesDir, WIPE_RESULT_INDICATOR);
        return !indicatorFile.exists();
    }

    /**
     * Check whether there is pending wipe flag in storage.
     * 
     * @return
     */
    public synchronized boolean hasPendingWipeFlag() {
        return mCache.hasWipeFlag();
    }

    /**
     * Clear pending wipe flag in storage.
     */
    public synchronized void clearPendingWipeFlag() {
        mCache.setWipeFlag(false);
        writeControlData(mCache);
    }

    /*
     * Update SIM information in storage with the snapshot of current SIM information in phone.
     */
    private synchronized void updateSimInfo() {
        mSimTracker.takeSnapshot();
        byte[][] list = buildSimFingerPrintList(mSimTracker);
        mCache.SimFingerPrintList = new LinkedList<byte[]>();
        for (byte[] sim : list) {
            mCache.SimFingerPrintList.add(sim);
        }
    }

    /**
     * Enable PPL. By enabling, we will start PplService for future operation.
     * 
     * @param config        The configuration used to enable PPL. If config is null, then we only set the enable flag.
     * @param updateSimInfo Whether update the SIM information in storage or not. If config is not null, then this flag
     *                      has no effect.
     */
    public synchronized void enable(ControlData config, boolean updateSimInfo) {
        Log.d(TAG, "enable(" + config + ")");
        if (updateSimInfo) {
            updateSimInfo();
        }

        if (config == null) { // enable only
            // sanity check
            if (!mCache.isProvisioned()) {
                throw new Error("Enable without provision");
            }
            mCache.setEnable(true);
            writeControlData(mCache);
        } else { // enable with new configuration
            // sanity check
            if (!config.isProvisioned() || !config.isEnabled()) {
                throw new Error("Inconsistent config state: " + config.status);
            }
            mCache = config;
            writeControlData(mCache);
        }

        Intent intent = new Intent(Intents.INTENT_ENABLE);
        intent.setClass(mService, PplService.class);
        mService.startService(intent);
    }

    /**
     * Disable PPL. By disabling, we will stop PplService.
     */
    public synchronized void disable() {
        mCache.setEnable(false);
        writeControlData(mCache);
        Intent intent = new Intent(Intents.INTENT_DISABLE);
        intent.setClass(mService, PplService.class);
        mService.startService(intent);
    }

    /**
     * Generate a new password and send it to emergency contacts. The secret in storage will be updated.
     * 
     * @param simId     Which SIM card will be used to send the message.
     * @param number    Which number the message will be sent to.
     */
    public synchronized void resetPassword(int simId, String number) {
        List<String> numbers = new LinkedList<String>();
        numbers.add(number);
        resetPassword(simId, numbers);
    }

    /**
     * Generate a new password and send it to emergency contacts. The secret in storage will be updated.
     * 
     * @param simId     Which SIM card will be used to send the message.
     * @param numbers   Numbers the message will be sent to.
     */
    private synchronized void resetPassword(int simId, List<String> numbers) {
        StringBuilder sb = new StringBuilder();
        Random rand = new Random();
        for (int i = 0; i < 8; ++i) {
            sb.append(rand.nextInt(10));
        }
        String newPassword = sb.toString();
        byte[] salt = new byte[ControlData.SALT_SIZE];
        byte[] newSecret = generateSecrets(newPassword.getBytes(), salt);
        System.arraycopy(salt, 0, mCache.salt, 0, salt.length);
        System.arraycopy(newSecret, 0, mCache.secret, 0, newSecret.length);
        writeControlData(mCache);
        // TODO finish all activities?
        for (String number : numbers) {
            sendMessage(MessageManager.Type.RESET_PW_RESPONSE, number, simId,
                    mMessageManager.buildMessage(MessageManager.Type.RESET_PW_RESPONSE, newPassword));
        }
    }

    /**
     * Check the lock flags to tell whether we should lock the phone.
     * 
     * @return  Lock flags OR-ed together.
     */
    public synchronized int needLock() {
        if (!mCache.isEnabled()) {
            return NO_LOCK;
        }
        int result = NO_LOCK;
        if (mCache.isLocked()) {
            result |= NORMAL_LOCK;
        }
        if (mCache.isSimLocked()) {
            result |= SIM_LOCK;
        }
        return result;
    }

    /**
     * This method will be invoked when SIM state is changed. It checks the SIM status and decides whether we should
     * lock the phone.
     */
    public synchronized void onSimStateChanged() {
        // dirty hack
        if (!mSimTracker.isTelephonyManagerReady()) {
            Log.w(TAG, "onSimStateChanged(): telephony manager is not ready");
            return;
        } else {
            Log.w(TAG, "onSimStateChanged(): telephony manager is ready");
        }
        mSimTracker.takeSnapshot();
        Log.d(TAG, "[onSimStateChanged] mSimTracker is " + mSimTracker);

        if (mSimTracker.getInsertedSim().length == 0) {
            mService.sendBroadcast(new Intent(PplService.Intents.UI_NO_SIM));
        }
        if (mCache.isEnabled()) {
            // Check for late initialization done
            boolean isSerialNumbersReady = true;
            String[] serialNumbers = mSimTracker.serialNumbers;
            boolean[] inserted = mSimTracker.inserted;
            for (int i = 0; i < serialNumbers.length; ++i) {
                if (inserted[i] && serialNumbers[i] == null) {
                    isSerialNumbersReady = false;
                    break;
                }
            }

            Log.d(TAG, "[onSimStateChanged] isSerialNumbersReady " + isSerialNumbersReady);

            if (mCache.SimFingerPrintList == null || mCache.SimFingerPrintList.size() == 0) {
                if (mSimTracker.getInsertedSim().length != 0 && isSerialNumbersReady) {
                    byte[][] current = ControlData.sortSimFingerPrints(buildSimFingerPrintList(mSimTracker));
                    if (mLockedSimFingerPrintList == null || isSimListChanged(mLockedSimFingerPrintList, current)) {
                        mLockedSimFingerPrintList = cloneSimList(current);
                        Log.w(TAG, "onSimStateChanged(): sim lock 1");
                        lock(true, PendingMessage.ALL_SIM_ID, null);
                    }
                }
                return;
            }

            if (isSerialNumbersReady) {
                Object[] objectArray = mCache.SimFingerPrintList.toArray();
                byte[][] array = new byte[objectArray.length][];
                for (int i = 0; i < array.length; ++i) {
                    array[i] = (byte[]) objectArray[i];
                }
                byte[][] previous = ControlData.sortSimFingerPrints(array);
                byte[][] current = ControlData.sortSimFingerPrints(buildSimFingerPrintList(mSimTracker));

                Log.d(TAG, "----- length of previous is " + previous.length);
                for (int i=0; i<previous.length; ++i) {
                    Log.d(TAG, "----- previous [" + i + "] is " + (new String(previous[i])));
                }

                Log.d(TAG, "----- length of current is " + current.length);
                for (int i=0; i<current.length; ++i) {
                    Log.d(TAG, "----- current [" + i + "] is " + (new String(current[i])));
                }

                if (isSimListChanged(previous, current)) {
                    if (mLockedSimFingerPrintList == null || isSimListChanged(mLockedSimFingerPrintList, current)) {
                        mLockedSimFingerPrintList = cloneSimList(current);
                    	Log.w(TAG, "onSimStateChanged(): sim lock 2");
                        lock(true, PendingMessage.ALL_SIM_ID, null);
                    }
                }
            }
        }
    }

    /**
     * Resends pending messages in storage. If there are pending messages for password reset operation, then all the
     * requests of this type will be merged and one new password will be generated then sent to all the numbers.
     */
    public synchronized void processPendingMessagesInNvram() {
        Log.d(TAG, "processPendingMessagesInNvram: hasPendingMessageInNvram? " + hasPendingMessageInNvram());
        if (hasPendingMessageInNvram()) {
            List<String> resetActions = new LinkedList<String>();
            ListIterator<PendingMessage> iter = mCache.PendingMessageList.listIterator();
            while (iter.hasNext()) {
                PendingMessage pm = iter.next();
                Log.d(TAG, "Found Pending Message: " + pm);
                if (pm.type == MessageManager.Type.RESET_PW_RESPONSE) {
                    iter.remove();
                    resetActions.add(pm.number);
                } else {
                    sendMessageToSingleNumber(
                            pm.type,
                            pm.number,
                            PendingMessage.ANY_SIM_ID,
                            mMessageManager.buildMessage(pm.type),
                            false,
                            pm.id);
                }
            }
            if (resetActions.size() > 0) {
                writeControlData(mCache);
                resetPassword(PendingMessage.ANY_SIM_ID, resetActions);
            }
        }
    }

    /**
     * Processing method for message sending result. If the sending is failed, then resend it after RETRY_INTERVAL
     * milliseconds. For long messages (MessageManager.Type.INSTRUCTION_DESCRIPTION), only mark it as succeeded if all
     * the segments are sent successfully.
     * 
     * @param intent        Intent set in sending method.
     * @param resultCode    Result code from SMS framework.
     */
    public synchronized void processMessageSentAction(Intent intent, int resultCode) {
        Log.d(TAG, "processMessageSentAction(" + intent + ", " + resultCode + ")");
        if (resultCode == SmsManager.RESULT_ERROR_GENERIC_FAILURE) {
        	if (intent.hasExtra("errorCode")) {
        		Log.d(TAG, "errorCode for generic failure is " + intent.getIntExtra("errorCode", -1));
        	}
        }
        Uri dataUri = intent.getData();
        if (dataUri == null ||
            !MessageManager.SMS_PENDING_INTENT_DATA_SCHEME.equals(dataUri.getScheme()) ||
            !MessageManager.SMS_PENDING_INTENT_DATA_AUTH.equals(dataUri.getAuthority())) {
            return;
        }
        List<String> segments = dataUri.getPathSegments();
        long id = Long.parseLong(segments.get(0));
        int total = Integer.parseInt(segments.get(1));
        int index = Integer.parseInt(segments.get(2));
        byte type = intent.getByteExtra(PendingMessage.KEY_TYPE, MessageManager.Type.INVALID);
        String number = intent.getStringExtra(PendingMessage.KEY_NUMBER);

        Log.d(TAG, "id is " + id + ", type is " + type + ", number is " + number);
        Log.d(TAG, "index is " + index + ", total is " + total);

        if (number == null) {
            throw new Error("Number is null");
        }
        int simId = intent.getIntExtra(PendingMessage.KEY_SIM_ID, PendingMessage.INVALID_SIM_ID);
        if (simId == PendingMessage.INVALID_SIM_ID) {
            throw new Error("SIM ID is missing");
        }
        if (type == MessageManager.Type.INSTRUCTION_DESCRIPTION) {
            int value = 0;
            if (mManualMessageResults.containsKey(id)) {
                // if contains id, then the value should not be 0
                value = mManualMessageResults.get(id);
                if (value > 0) {
                    if (resultCode == Activity.RESULT_OK) {
                        value += 1;
                    } else {
                        value = (-value) - 1;
                    }
                } else {
                    value = value - 1;
                }
            } else {
                value = (resultCode == Activity.RESULT_OK) ? 1 : -1;
            }
            mManualMessageResults.put(id, value);
            if (Math.abs(value) == total) {
                String message = mService.getResources().getString(
                        (value > 0) ?
                            R.string.toast_instruction_send_succeeded :
                            R.string.toast_instruction_send_failed);
                mService.showToast(message);
                mManualMessageResults.remove(id);
            }
        } else {
            if (resultCode == Activity.RESULT_OK) {
                if (type == MessageManager.Type.LOCK_RESPONSE ||
                    type == MessageManager.Type.UNLOCK_RESPONSE ||
                    type == MessageManager.Type.WIPE_STARTED ||
                    type == MessageManager.Type.WIPE_COMPLETED) {
                    Log.d(TAG, "A: " + removePendingMessageInQueue(id, type, number));
                    Log.d(TAG, "B: " + removePendingMessageFromNvram(id, type, number));
                } else {
                	Log.d(TAG, "C: " + removePendingMessageInQueue(id, type, number, simId));
                	Log.d(TAG, "D: " + removePendingMessageFromNvram(id, type, number, simId));
                }
            } else {
                Log.d(TAG, "Message send fail, check message from queue next.");
                final PendingMessage pm = findPendingMessageInQueue(id, type, number, simId);
                if (pm != null) {
                    Log.d(TAG, "pending message is not null.");
                    mService.postRun(new Runnable() {
                        @Override
                        public void run() {
                            sendMessage(pm, false, false);
                        }
                    }, RETRY_INTERVAL);
                } else {
                    Log.d(TAG, "pending message is null, not send.");
                }
            }
        }
    }

    /**
     * Build message according to the type specified.
     * 
     * @param type  Type of the message.
     * @param args  Arguments used to construct the message. The number and types of the arguments depends on the type.
     * @return      Message content.
     */
    public String buildMessage(byte type, Object... args) {
        return mMessageManager.buildMessage(type, args);
    }

    /**
     * Save message sending request to storage.
     * 
     * @param type      Type of the message.
     * @param number    Which number the message will be sent to.
     */
    private synchronized void savePendingMessageToNvram(byte type, String number) {
        PendingMessage request = new PendingMessage(PendingMessage.getNextId(), type, number, PendingMessage.ALL_SIM_ID, null);
        if (mCache.PendingMessageList == null) {
            mCache.PendingMessageList = new LinkedList<PendingMessage>();
        }
        mCache.PendingMessageList.add(request);
        writeControlData(mCache);
    }

    /*
     * Send the message stored in request. If the id in request is PendingMessage.INVALID_ID, then this method will
     * generate one for it. The message request will be stored as pending in queue in memory and/or storage according
     * to the arguments.
     * 
     * @param request       Message request.
     * @param addToQueue    Whether add this request to queue in memory. Instruction description messages will not be
     *                      saved to storage no matter what this value is.
     * @param addToNvram    Whether add this request to storage. If addToQueue is false, then this argument is ignored
     *                      and the message will not be saved to storage. SIM change messages and instruction
     *                      description messages will not be saved to storage no matter what this value is.
     */
    private synchronized void sendMessage(PendingMessage request, boolean addToQueue, boolean addToNvram) {
        if (request.content == null) {
            throw new Error("Message content should not be null");
        }
        PendingMessage pm = request.clone();

        if (pm.type == MessageManager.Type.INSTRUCTION_DESCRIPTION) {
            addToQueue = false;
        }

        if (addToNvram) {
            addToNvram = (pm.type != MessageManager.Type.SIM_CHANGED && pm.type != MessageManager.Type.INSTRUCTION_DESCRIPTION);
        }

        if (pm.id == PendingMessage.INVALID_ID) {
            pm.id = PendingMessage.getNextId();
            Log.d(TAG, "Generate pending message id: " + pm.id);
        }

        if (addToQueue) {
            mPendingMessages.add(pm);
            if (addToNvram) {
                if (mCache.PendingMessageList == null) {
                    mCache.PendingMessageList = new LinkedList<PendingMessage>();
                }
                mCache.PendingMessageList.add(pm);
                writeControlData(mCache);
            }
        }

        Intent intent = new Intent(MessageManager.SMS_SENT_ACTION);
        intent.putExtra(PendingMessage.KEY_TYPE, pm.type);
        intent.putExtra(PendingMessage.KEY_NUMBER, pm.number);
        intent.putExtra(PendingMessage.KEY_SIM_ID, pm.simId);
        intent.putExtra(PendingMessage.KEY_FIRST_TRIAL, addToQueue);

        Log.d(TAG, "pm.id " + pm.id + ", pm.type " + pm.type + ", pm.number " + pm.number);
        Log.d(TAG, "pm.content " + pm.content + ", pm.simId " + pm.simId);
        mPlatformManager.sendTextMessage(pm.number, pm.id, pm.content, intent, pm.simId);
    }

    /**
     * Send message.
     * 
     * @param type      Type of the message.
     * @param number    Which number the message will be sent to. If this argument is null, then the message will be
     *                  sent to all emergency contacts.
     * @param simId     Which SIM card will be used to send the message.
     * @param content   Content of the message.
     */
    public synchronized void sendMessage(byte type, String number, int simId, String content) {
        Log.d(TAG, "sendMessage(" + type + ", " + number + ", " + simId + ", " + content + ")");
        if (number != null) {
            // Send to single number
            sendMessageToSingleNumber(type, number, simId, content);
        } else {
            // Send to all number
            Log.d(TAG, "mCache.TrustedNumberList is " + mCache.TrustedNumberList);
            if (mCache.TrustedNumberList != null) {
                Log.d(TAG, "mCache.TrustedNumberList.size() is " + mCache.TrustedNumberList.size());
                for (String trustedNumber : mCache.TrustedNumberList) {
                    sendMessageToSingleNumber(type, trustedNumber, simId, content);
                }
            }
        }
    }

    /*
     * Send message to single number.
     * 
     * @param type      Type of the message.
     * @param number    Which number the message will be sent to.
     * @param simId     Which SIM card will be used to send the message.
     * @param content   Content of the message.
     */
    private void sendMessageToSingleNumber(byte type, String number, int simId, String content) {
        sendMessageToSingleNumber(type, number, simId, content, true, PendingMessage.INVALID_ID);
    }

    /*
     * Send message to single number.
     * 
     * @param type          Type of the message.
     * @param number        Which number the message will be sent to.
     * @param simId         Which SIM card will be used to send the message.
     * @param content       Content of the message.
     * @param addToNvram    Whether add this request to storage.
     * @param id            ID of the message.
     */
    private void sendMessageToSingleNumber(byte type, String number, int simId, String content, boolean addToNvram, long id) {
    	Log.d(TAG, "sendMessageToSingleNumber: type " + type + ", simId " + simId);
        if (simId == PendingMessage.ANY_SIM_ID) {
        	int i = 0;
            for (; i < mSimTracker.inserted.length; ++i) {
                if (mSimTracker.inserted[i]) {
                    sendMessage(new PendingMessage(id, type, number, i, content), true, addToNvram);
                    break;
                }
            }
            if (i == mSimTracker.inserted.length) {
            	Log.w(TAG, "sendMessageToSingleNumber: No SIM found.");
            }
        } else if (simId == PendingMessage.ALL_SIM_ID) {
            for (int i = 0; i < mSimTracker.inserted.length; ++i) {
                if (mSimTracker.inserted[i]) {
                    sendMessage(new PendingMessage(id, type, number, i, content), true, addToNvram);
                }
            }
        } else if (simId >= 0) {
            sendMessage(new PendingMessage(id, type, number, simId, content), true, addToNvram);
        }
    }

    /*
     * Remove pending message from queue in memory. Only the message matches the arguments will be removed.
     * 
     * @param id        Id of the message.
     * @param type      Type of the message.
     * @param number    Which number the message will be sent to.
     * @return          Whether the message is found.
     */
    private boolean removePendingMessageInQueue(long id, byte type, String number) {
        Log.d(TAG, "+removePendingMessageInQueue(long id, byte type, String number)");
        dumpList(mPendingMessages);
        boolean found = false;
        ListIterator<PendingMessage> iter = mPendingMessages.listIterator();
        while (iter.hasNext()) {
            PendingMessage pm = iter.next();
            if (id == pm.id && type == pm.type && number.equals(pm.number)) {
                iter.remove();
                found = true;
            }
        }
        dumpList(mPendingMessages);
        Log.d(TAG, "-removePendingMessageInQueue(long id, byte type, String number)");
        return found;
    }

    /*
     * Remove pending message from queue in memory. Only the message matches the arguments will be removed.
     * 
     * @param id        Id of the message.
     * @param type      Type of the message.
     * @param number    Which number the message will be sent to.
     * @param simId     Which SIM card will be used to send the message.
     * @return          Whether the message is found.
     */
    private boolean removePendingMessageInQueue(long id, byte type, String number, int simId) {
        Log.d(TAG, "+removePendingMessageInQueue(long id, byte type, String number, int simId)");
        dumpList(mPendingMessages);
        boolean found = false;
        ListIterator<PendingMessage> iter = mPendingMessages.listIterator();
        while (iter.hasNext()) {
            PendingMessage pm = iter.next();
            if (id == pm.id && type == pm.type && number.equals(pm.number) && simId == pm.simId) {
                iter.remove();
                found = true;
            }
        }
        dumpList(mPendingMessages);
        Log.d(TAG, "-removePendingMessageInQueue(long id, byte type, String number, int simId)");
        return found;
    }

    /**
     * Remove pending messages from storage. Only the message matches the arguments will be removed.
     * 
     * @param id        Id of the message.
     * @param type      Type of the message.
     * @param number    Which number the message will be sent to.
     * @return          Whether the message is found.
     */
    public synchronized boolean removePendingMessageFromNvram(long id, byte type, String number) {
        Log.d(TAG, "+removePendingMessageFromNvram(long id, byte type, String number)");
        if (mCache.PendingMessageList == null) {
            return false;
        }
        dumpList(mCache.PendingMessageList);
        boolean found = false;
        ListIterator<PendingMessage> iter = mCache.PendingMessageList.listIterator();
        while (iter.hasNext()) {
            PendingMessage pm = iter.next();
            if (id == pm.id && type == pm.type && number.equals(pm.number)) {
                iter.remove();
                found = true;
            }
        }
        writeControlData(mCache);
        dumpList(mCache.PendingMessageList);
        Log.d(TAG, "-removePendingMessageFromNvram(long id, byte type, String number)");
        return found;
    }

    /*
     * Remove pending messages from storage. Only the message matches the arguments will be removed.
     * 
     * @param id        Id of the message.
     * @param type      Type of the message.
     * @param number    Which number the message will be sent to.
     * @param simId     Which SIM card will be used to send the message.
     * @return          Whether the message is found.
     */
    public synchronized boolean removePendingMessageFromNvram(long id, byte type, String number, int simId) {
        Log.d(TAG, "+removePendingMessageFromNvram(long id, byte type, String number, int simId)");
        if (mCache.PendingMessageList == null) {
            return false;
        }
        dumpList(mCache.PendingMessageList);
        boolean found = false;
        ListIterator<PendingMessage> iter = mCache.PendingMessageList.listIterator();
        while (iter.hasNext()) {
            PendingMessage pm = iter.next();
            if (id == pm.id && type == pm.type && number.equals(pm.number) && simId == pm.simId) {
                iter.remove();
                found = true;
            }
        }
        writeControlData(mCache);
        dumpList(mCache.PendingMessageList);
        Log.d(TAG, "-removePendingMessageFromNvram(long id, byte type, String number, int simId)");
        return found;
    }

    /*
     * Find the pending message in queue in memory. Only the message matches the arguments will be returned.
     * 
     * @param id        Id of the message.
     * @param type      Type of the message.
     * @param number    Which number the message will be sent to.
     * @param simId     Which SIM card will be used to send the message.
     * @return
     */
    private synchronized PendingMessage findPendingMessageInQueue(long id, byte type, String number, int simId) {
        if (mPendingMessages != null) {
            for (PendingMessage pm : mPendingMessages) {
                if (id == pm.id && type == pm.type && number.equals(pm.number) && simId == pm.simId) {
                    return pm;
                }
            }
        }
        return null;
    }

    /**
     * Check whether there are pending messages in storage.
     * 
     * @return
     */
    public synchronized boolean hasPendingMessageInNvram() {
        return (mCache.PendingMessageList != null && mCache.PendingMessageList.size() > 0);
    }

    /**
     * Check whether the password is correct.
     *
     * @param password
     * @return
     */
    public synchronized boolean verifyPassword(byte[] password) {
        return checkPassword(password, mCache.salt, mCache.secret);
    }

    /**
     * Check whether the password is correct.
     *
     * SHA1(password:salt) == secret
     *
     * @param password
     * @param salt
     * @param secret
     * @return
     */
    private static boolean checkPassword(final byte[] password, final byte[] salt, final byte[] secret) {
        MessageDigest md = null;
        try {
            md = MessageDigest.getInstance("SHA-1");
            byte[] buffer = new byte[salt.length + password.length];
            System.arraycopy(password, 0, buffer, 0, password.length);
            System.arraycopy(salt, 0, buffer, password.length, salt.length);
            byte[] digest = md.digest(buffer);
            if (secret.length != digest.length) {
                return false;
            }
            for (int i = 0; i < secret.length; ++i) {
                if (secret[i] != digest[i]) {
                    return false;
                }
            }
            return true;
        } catch (NoSuchAlgorithmException e) {
            throw new Error(e);
        }
    }

    /**
     * Generate new secrets, including new password and new salt.
     * 
     * @param password
     * @param salt
     * @return
     */
    public static byte[] generateSecrets(final byte[] password, byte[] salt) {
        // generate salts
        Random random = new Random();
        random.nextBytes(salt);
        // generate secret
        MessageDigest md = null;
        try {
            md = MessageDigest.getInstance("SHA-1");
            byte[] buffer = new byte[salt.length + password.length];
            System.arraycopy(password, 0, buffer, 0, password.length);
            System.arraycopy(salt, 0, buffer, password.length, salt.length);
            return md.digest(buffer);
        } catch (NoSuchAlgorithmException e) {
            throw new Error(e);
        }
    }

    /*
     * Turn off USB Mass Storage mode and format SD cards.
     * 
     * @return  true if it is turned off, false if the caller will be notified asynchronously.
     */
    private void formatSDCard() {
        Intent intent = new Intent(PplService.Intents.INTENT_NOTIFY_MOUNT_SERVICE_WIPE);
        mService.sendBroadcast(intent);
    }

    private void writeControlData(ControlData config) {
        try {
            mAgent.writeControlData(config.encode());
        } catch (RemoteException e) {
            throw new Error(e);
        }
    }

    /*
     * Compare lhs and rhs for difference. lhs and rhs should be sorted.
     * 
     * @param lhs   Sorted SIM finger prints.
     * @param rhs   Sorted SIM finger prints.
     * @return
     */
    private boolean isSimListChanged(byte[][] lhs, byte[][] rhs) {
        if (lhs.length != rhs.length) {
            return true;
        } else {
            for (int i = 0; i < lhs.length; ++i) {
                if (ControlData.compareSimFingerPrints(lhs[i], rhs[i]) != 0) {
                    return true;
                }
            }
            return false;
        }
    }

    /**
     * Query name for number.
     * 
     * @param context
     * @param number
     * @return
     */
    public static String getContactNameByPhoneNumber(Context context, String number) {
        ContentResolver contentResolver = context.getContentResolver();
        Uri uri = Uri.withAppendedPath(ContactsContract.PhoneLookup.CONTENT_FILTER_URI, Uri.encode(number));
        String[] projection = new String[] { PhoneLookup.DISPLAY_NAME, PhoneLookup._ID };
        Cursor cursor = contentResolver.query(uri, projection, null, null, null);
        if (cursor == null) {
            return null;
        }
        String id = null;
        if (cursor.moveToFirst()) {
            id = cursor.getString(cursor.getColumnIndexOrThrow(PhoneLookup._ID));
        }
        cursor.close();
        if (id == null) {
            return null;
        }

        // Build the Entity URI.
        Uri.Builder b = Uri.withAppendedPath(ContactsContract.Contacts.CONTENT_URI, id).buildUpon();
        b.appendPath(ContactsContract.Contacts.Entity.CONTENT_DIRECTORY);
        Uri contactUri = b.build();
        Log.d(TAG, "XXX: contactUri is " + contactUri);
        // Create the projection (SQL fields) and sort order.
        projection = new String[] {
                ContactsContract.Contacts.Entity.RAW_CONTACT_ID,
                ContactsContract.Contacts.Entity.DATA1,
                ContactsContract.Contacts.Entity.MIMETYPE
        };
        String sortOrder = ContactsContract.Contacts.Entity.RAW_CONTACT_ID + " ASC";
        cursor = context.getContentResolver().query(contactUri, projection, null, null, sortOrder);
        if (cursor == null) {
            return null;
        }
        String name = null;
        int mimeIdx = cursor.getColumnIndex(ContactsContract.Contacts.Entity.MIMETYPE);
        int dataIdx = cursor.getColumnIndex(ContactsContract.Contacts.Entity.DATA1);
        if (cursor.moveToFirst()) {
            do {
                String mime = cursor.getString(mimeIdx);
                if (mime.equalsIgnoreCase(CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE)) {
                    name = cursor.getString(dataIdx);
                }
            } while (cursor.moveToNext());
        }
        cursor.close();

        return name;
    }

    public static class ContactQueryResult {
        public String name;
        public ArrayList<String> phones;

        public ContactQueryResult() {
            name = null;
            phones = new ArrayList<String>();
        }
    }

    /**
     * Query name and numbers for specified contact.
     * 
     * @param context
     * @param uri       URI of contact.
     * @return
     */
    public static ContactQueryResult getContactInfo(Context context, Uri uri) {
        ContactQueryResult result = new ContactQueryResult();
        String id = null;
        Cursor cursor = context.getContentResolver().query(uri, null, null, null, null);
        if (cursor == null) {
            return result;
        }
        if (cursor.moveToFirst()) {
            id = cursor.getString(cursor.getColumnIndex(ContactsContract.Contacts._ID));
        }
        cursor.close();
        // Build the Entity URI.
        Uri.Builder b = Uri.withAppendedPath(ContactsContract.Contacts.CONTENT_URI, id).buildUpon();
        b.appendPath(ContactsContract.Contacts.Entity.CONTENT_DIRECTORY);
        Uri contactUri = b.build();
        // Create the projection (SQL fields) and sort order.
        String[] projection = { ContactsContract.Contacts.Entity.RAW_CONTACT_ID,
                ContactsContract.Contacts.Entity.DATA1, ContactsContract.Contacts.Entity.MIMETYPE };
        String sortOrder = ContactsContract.Contacts.Entity.RAW_CONTACT_ID + " ASC";
        cursor = context.getContentResolver().query(contactUri, projection, null, null, sortOrder);
        if (cursor == null) {
            return result;
        }
        String mime;
        int mimeIdx = cursor.getColumnIndex(ContactsContract.Contacts.Entity.MIMETYPE);
        int dataIdx = cursor.getColumnIndex(ContactsContract.Contacts.Entity.DATA1);
        if (cursor.moveToFirst()) {
            do {
                mime = cursor.getString(mimeIdx);
                if (mime.equalsIgnoreCase(CommonDataKinds.Phone.CONTENT_ITEM_TYPE)) {
                    result.phones.add(cursor.getString(dataIdx));
                } else if (mime.equalsIgnoreCase(CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE)) {
                    result.name = cursor.getString(dataIdx);
                }
            } while (cursor.moveToNext());
        }
        cursor.close();
        return result;
    }

    /**
     * Build SIM finger prints in SimTracker from serial number.
     * 
     * @param simTracker
     * @return
     */
    public static byte[][] buildSimFingerPrintList(SimTracker simTracker) {
        byte[][] result = new byte[simTracker.slotNumber][];
        int count = 0;
        for (int i = 0; i < simTracker.slotNumber; ++i) {
            if (simTracker.inserted[i] && simTracker.serialNumbers[i] != null) {
                result[count] = new byte[ControlData.SIM_FINGERPRINT_LENGTH];
                byte[] data = simTracker.serialNumbers[i].getBytes();
                System.arraycopy(data, 0, result[count], 0, data.length);
                count += 1;
            }
        }

        return Arrays.copyOf(result, count);
    }

    /*
     * For debugging only.
     */
    private void dumpList(List<PendingMessage> list) {
    	for (int i = 0; i < list.size(); ++i) {
    		Log.d(TAG, "[" + i + "]: " + list.get(i));
    	}
    }
}
