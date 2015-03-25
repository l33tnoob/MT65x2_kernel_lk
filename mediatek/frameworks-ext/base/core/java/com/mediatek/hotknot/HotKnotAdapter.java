package com.mediatek.hotknot;

import android.annotation.SdkConstant;
import android.annotation.SdkConstant.SdkConstantType;
import android.app.Activity;
import android.content.Context;
import android.net.Uri;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import com.mediatek.hotknot.HotKnotActivityManager;
import com.mediatek.hotknot.IHotKnotAdapter;


/**
 * Represents the local HotKnot adapter.
 * <p>
 * Use the helper {@link #getDefaultAdapter(Context)} to get the default HotKnot
 * adapter for this Android device.
 *
 */
public final class HotKnotAdapter {
    static final String TAG = "HotKnot";

    static final String HOTKNOT_SERVICE = "hotknot_service";

    /**
     * Intent to start an Activity when a HotKnot data message (payload) is discovered.
     * <p>The system inspects the MIME type of HotKnot message and sends this intent which contains the MIME type in its type field.
     * This allows Activity to register IntentFilters targeting specific MIME type. Activities should register the
     * most specific intent filters
     */
    @SdkConstant(SdkConstantType.ACTIVITY_INTENT_ACTION)
    public static final String ACTION_MESSAGE_DISCOVERED = "com.mediatek.hotknot.action.MESSAGE_DISCOVERED";

    /**
     * Mandatory extra containing a byte array of the discovered message data for the
     * {@link #ACTION_MESSAGE_DISCOVERED} intents.
     */
    public static final String EXTRA_DATA = "com.mediatek.hotknot.extra.DATA";


    /**
     * Broadcast action: The state of local HotKnot adapter has been
     * changed,e.g.<p>HotKnot has been enabled or disabled.
     * <p>Always contains the extra field {@link #EXTRA_ADAPTER_STATE}
     */
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_ADAPTER_STATE_CHANGED =
        "com.mediatek.hotknot.action.ADAPTER_STATE_CHANGED";

    /**
     * Used as an int extra field in {@link #ACTION_ADAPTER_STATE_CHANGED}
     * intents to request the current power state. Possible values:
     * {@link #STATE_DISABLED},
     * {@link #STATE_ENABLED},
     */
    public static final String EXTRA_ADAPTER_STATE = "com.mediatek.hotknot.extra.ADAPTER_STATE";

    /**
     * Activity action: Shows HotKnot settings.
     * <p>
     * This shows UI that allows HotKnot to be turned on or off.
     * <p>
     * In some cases, a matching Activity may not exist, so be sure you
     * safeguard against this.
     * <p>
     * Input: Nothing.
     * <p>
     * Output: Nothing
     * @see #isEnabled
     */    
    public static final String ACTION_HOTKNOT_SETTINGS = "mediatek.settings.HOTKNOT_SETTINGS";

    /**
     * Feature for {@link android.content.pm.PackageManager#getSystemAvailableFeatures} and
     * {@link android.content.pm.PackageManager#hasSystemFeature}:
     * The device can communicate using HotKnot
     * @hide
     */
    public static final String FEATURE_NFC = "android.hardware.nfc";
    
    /**
     * HotKnot is disabled.
     */
    public static final int STATE_DISABLED = 1;
    
    /**
     * HotKnot is enabled.
     */
    public static final int STATE_ENABLED = 2;

    /**
     * The operation was successful.
     * Return code for setHotKnotMessage API call.
     */
    public static final int ERROR_SUCCESS = 0;

    /**
     * HotKnotMessage data size over 1 kilobytes limit.
     * Return code for setHotKnotMessage API call.
     */
    public static final int ERROR_DATA_TOO_LARGE = 1;

    static IHotKnotAdapter mService;
    final  HotKnotActivityManager mHotKnotActivityManager;
    final  Context mContext;
    private static HotKnotAdapter mHotKnotAdapter = null;

    /**
     * A callback to be invoked when the system successfully delivers your {@link HotKnotMessage}
     * to another device.
     * @see #setOnHotKnotCompleteCallback
     */
    public interface OnHotKnotCompleteCallback {
        /**
         * Called on successful HotKnot message sent.
         *
         * <p>This callback is usually made on a binder thread (not the UI thread).
         *
         * @see #setHotKnotMessageCallback
         */
        public void onHotKnotComplete(int reason);
    }

    /**
     * A callback to be invoked when another device capable of HotKnot message transfer is within range.
     * <p>Implement this interface and pass it to {@link
     * HotKnotAdapter#setHotKnotMessageCallback setHotKnotMessageCallback()} in order to create an
     * {@link HotKnotMessage} at the moment when another device is within range for HotKnot. Using this
     * callback allows you to create a message with data that might vary based on the
     * content currently visible to the user. Alternatively, you can call {@link
     * #setHotKnotMessage setHotKnotMessage()} if {@link HotKnotMessage} always contains the
     * same data.
     */
    public interface CreateHotKnotMessageCallback {
        /**
         * Called to provide a {@link HotKnotMessage} to send.
         *
         * <p>This callback is usually made on a binder thread (not the UI thread).
         *
         * <p>Called when this device is in range of another device. It allows applications to
         * create HotKnot message only when it is required.
         *
         * <p>HotKnot exchange cannot occur until this method returns, so do not
         * block for too long.
         *
         * <p>The Android operating system usually shows a system UI
         * on top of your Activity at this time, so do not try to request
         * input from the user to complete the callback. The user probably will not see it.
         *
         * @return HotKnot message to send, or null to not provide a message
         */
        public HotKnotMessage createHotKnotMessage();
    }

    /**
     * A callback to be invoked when another device capable of HotKnot Beam transfer is within range.
     * <p>Implement this interface and pass it to {@link
     * HotKnotAdapter#setHotKnotBeamUrisCallback setHotKnotBeamUrisCallback()} in order to create one or more URIs
     * at the moment when another device is within range for HotKnot. Using this
     * callback allows you to send large files that might vary based on the
     * content currently visible to the user. Alternatively, you can call {@link
     * #setHotKnotBeamUris setHotKnotBeamUris()} if the URI is the same.     
     */  
    public interface CreateHotKnotBeamUrisCallback {
        public Uri[] createHotKnotBeamUris();
    }

    /**
     * Helpers to get the default HotKnot adapter.
     */
    public static HotKnotAdapter getDefaultAdapter(Context context) {
        if (context == null) {
            throw new IllegalArgumentException("context cannot be null");
        }

        context = context.getApplicationContext();
        if (context == null) {
            throw new IllegalArgumentException(
                "context not associated with any application (using a mock context?)");
        }

        try {
            IBinder b = ServiceManager.getService(HOTKNOT_SERVICE);
            if(b == null) {
                Log.i("debug", "The binder is null");
                return null;
            }
            mService = IHotKnotAdapter.Stub.asInterface(b);
            mHotKnotAdapter = new HotKnotAdapter(context);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return mHotKnotAdapter;
    }

    HotKnotAdapter(Context context) {
        mContext = context;
        mHotKnotActivityManager = new HotKnotActivityManager(this);
    }

    /**
     * Returns true if this HotKnot adapter has any feature enabled.
     *
     * <p>If this method returns false, the HotKnot hardware will guaranteed not to
     * generate or respond to any HotKnot communication over its HotKnot link.
     * <p>Applications can use this to check if HotKnot is enabled. Applications
     * can request Settings UI allowing the user to toggle HotKnot using:
     * <p><pre>startActivity(new Intent({@link HotKnotAdapter#ACTION_HOTKNOT_SETTINGS
     * HotKnotAdapter.ACTION_HOTKNOT_SETTINGS}))</pre>
     *
     * @return true if this HotKnot adapter has any features enabled
     */
    public boolean isEnabled() {                
        try {
            return mService.isEnabled();
        } catch (RemoteException e) {
            return false;
        }
    }

    /**
     * Enables HotKnot hardware.
     *
     * <p>This call is asynchronous. Listen to
     * {@link #ACTION_ADAPTER_STATE_CHANGED} broadcasts to find out when the
     * operation is completed.
     *
     * <p>If this returns true, either HotKnot will be enabled, or
     * a {@link #ACTION_ADAPTER_STATE_CHANGED} broadcast will be sent
     * to indicate a state transition. If this returns false,
     * there will be some problems that prevent an attempt to enable
     * HotKnot.
     *
     * @hide
     */
    public boolean enable() {
        try {
            return mService.enable();
        } catch (RemoteException e) {
            return false;
        }
    }

    /**
     * Disables HotKnot hardware.
     *
     * <p>No HotKnot feature will work after this call, and the hardware
     * will not perform or respond to any HotKnot communication.
     *
     * <p>This call is asynchronous. Listen to
     * {@link #ACTION_ADAPTER_STATE_CHANGED} broadcasts to find out when the
     * operation is completed.
     *
     * <p>If this returns true, either HotKnot will be disabled, or
     * a {@link #ACTION_ADAPTER_STATE_CHANGED} broadcast will be sent
     * to indicate a state transition. If this returns false,
     * there will be some problems that prevent an attempt to disable HotKnot
     *
     * @hide
     */

    public boolean disable() {
        try {
            return mService.disable(true);
        } catch (RemoteException e) {
            return false;
        }
    }


    /**
        * Sets up a static {@link HotKnotMessage} to send using HotKnot.
        *
        * <p>This method may be called any time before Activity's onDestroy,
        * but the HotKnot message is only made available for HotKnot sent when the
        * specified activity(s) are in resumed (foreground) state. The recommended
        * approach is calling this method during your Activity's onCreate (see the sample
        * code below). This method does not immediately perform any I/O or blocking work,
        * so it is safe to call on your main thread.
        *
        * <p>Only one HotKnot message can be sent by the currently resumed Activity.
        * If both {@link #setHotKnotMessage} and
        * {@link #setHotKnotMessageCallback} are set,
        * the callback will take priority.
        * <p>If {@link #setHotKnotMessage} is called with a null HotKnot message,
        * and/or {@link #setHotKnotMessageCallback} is called with a null callback,
        * HotKnot exchange will be completely disabled for specified activity(s).
        *
        * <p>The API allows multiple Activities to be specified at a time,
        * but it is strongly recommended to register only one at a time
        * and to do so during Activity's onCreate. For example:
        * <pre>
        * protected void onCreate(Bundle savedInstanceState) {
        *     super.onCreate(savedInstanceState);
        *     HotKnotAdapter hotKnotAdapter = HotKnotAdapter.getDefaultAdapter(this);
        *     if (hotKnotAdapter == null) return;  // HotKnot not available on this device
        *     hotKnotAdapter.setHotKnotMessage(HotKnotMessage, this);
        * }</pre>
        * Only one call per Activity is necessary. The Android
        * OS will automatically release its references to the HotKnot message and the
        * Activity object when it is destroyed if you follow this pattern.
        *
        * <p>If your Activity is to dynamically generate an HotKnot message,
        * set a callback using {@link #setHotKnotMessageCallback} instead
        * of a static message.
        *
        * <p class="note">Do not pass in an Activity that has already been through
        * Activity's onDestroy. This is guaranteed if you call this API
        * during Activity's onCreate.
        *
        * <p class="note">Requires "android.permission.HOTKNOT" permission.
        *
        * @param message HotKnot message to send over HotKnot, or null to disable
        * @param activity activity for which the HotKnot message will be sent
        * @param activities optional additional activities, however we strongly recommend
        *        to only register one at a time, and to do so in that Activity's
        *        Activity's onCreate.
        */
    public void setHotKnotMessage(HotKnotMessage message, Activity activity,
                                  Activity ... activities) {
        try {
            if (activity == null) {
                throw new NullPointerException("activity cannot be null");
            }
            mHotKnotActivityManager.setHotKnotMessage(activity, message, 0);
            for (Activity a : activities) {
                if (a == null) {
                    throw new NullPointerException("activities cannot contain null");
                }
                mHotKnotActivityManager.setHotKnotMessage(a, message, 0);
            }
        } catch (IllegalStateException e) {
            // Prevent new applications from making this mistake, re-throw
            throw(e);
        }
    }

    /**
     * @hide
     */
    public void setHotKnotMessage(HotKnotMessage message, Activity activity, int flags) {
        if (activity == null) {
            throw new NullPointerException("activity cannot be null");
        }
        mHotKnotActivityManager.setHotKnotMessage(activity, message, flags);
    }

    /**
     * Sets up a callback that dynamically generates HotKnot messages to send using HotKnot.
     *
     * <p>This method may be called any time before Activity's onDestroy,
     * but the HotKnot message callback can only occur when the
     * specified activity(s) are in resumed (foreground) state. The recommended
     * approach is calling this method during your Activity's onCreate (see the sample
     * code below). This method does not immediately perform any I/O or blocking work,
     * so it is safe to call on your main thread.
     *
     * <p>Only one HotKnot message can be sent by the currently resumed Activity.
     * If both {@link #setHotKnotMessage} and
     * {@link #setHotKnotMessageCallback} are set,
     * the callback will take priority.
     *
     * <p>If {@link #setHotKnotMessage} is called with a null HotKnot message,
     * and/or {@link #setHotKnotMessageCallback} is called with a null callback,
     * HotKnot exchange will be completely disabled for specified activity(s).
     *
     * <p>The API allows multiple Activities to be specified at a time,
     * but it is strongly recommended to register only one at a time
     * and to do so during Activity's onCreate. For example:
     * <pre>
     * protected void onCreate(Bundle savedInstanceState) {
     *     super.onCreate(savedInstanceState);
     *     HotKnotAdapter hotKnotAdapter = HotKnotAdapter.getDefaultAdapter(this);
     *     if (hotKnotAdapter == null) return;  // HotKnot not available on this device
     *     hotKnotAdapter.setHotKnotMessageCallback(callback, this);
     * }</pre>
     * Only one call per Activity is necessary. The Android
     * OS will automatically release its references to the callback and the
     * Activity object when it is destroyed if you follow this pattern.
     *
     * <p class="note">Do not pass in an Activity that has already been through
     * Activity's onDestroy. This is guaranteed if you call this API
     * during Activity's onCreate.
     *
     * <p class="note">Requires "android.permission.HOTKNOT" permission.
     *
     * @param callback callback, or null to disable
     * @param activity activity for which the HotKnot message will be sent
     * @param activities optional additional activities, however we strongly recommend
     *        to only register one at a time, and to do so in that Activity's
     *        Activity's onCreate.
     */
    public void setHotKnotMessageCallback(CreateHotKnotMessageCallback callback, Activity activity,
                                          Activity ... activities) {
        try {
            if (activity == null) {
                throw new NullPointerException("activity cannot be null");
            }
            mHotKnotActivityManager.setHotKnotMessageCallback(activity, callback, 0);
            for (Activity a : activities) {
                if (a == null) {
                    throw new NullPointerException("activities cannot contain null");
                }
                mHotKnotActivityManager.setHotKnotMessageCallback(a, callback, 0);
            }
        } catch (IllegalStateException e) {
            // Prevent new applications from making this mistake, re-throw
            throw(e);
        }
    }

    /**
     * @hide
     */
    public void setHotKnotMessageCallback(CreateHotKnotMessageCallback callback, Activity activity,
                                          int flags) {
        if (activity == null) {
            throw new NullPointerException("activity cannot be null");
        }
        mHotKnotActivityManager.setHotKnotMessageCallback(activity, callback, flags);
    }

    /**
     * Sets up a callback on successful HotKnot.
     *
     * <p>This method may be called any time before Activity's onDestroy,
     * but the callback can only occur when the
     * specified activity(s) are in resumed (foreground) state. The recommended
     * approach is calling this method during your Activity's onCreate (see the sample
     * code below). This method does not immediately perform any I/O or blocking work,
     * so it is safe to call on your main thread.
     *
     * <p>The API allows multiple Activities to be specified at a time,
     * but it is strongly recommended to register only one at a time
     * and to do so during Activity's onCreate. For example:
     * <pre>
     * protected void onCreate(Bundle savedInstanceState) {
     *     super.onCreate(savedInstanceState);
     *     HotKnotAdapter hotKnotAdapter = HotKnotAdapter.getDefaultAdapter(this);
     *     if (hotKnotAdapter == null) return;  // HotKnot not available on this device
     *     hotKnotAdapter.setOnHotKnotCompleteCallback(callback, this);
     * }</pre>
     * Only one call per Activity is necessary. The Android
     * OS will automatically release its references to the callback and the
     * Activity object when it is destroyed if you follow this pattern.
     *
     * <p class="note">Do not pass in an Activity that has already been through
     * Activity's onDestroy. This is guaranteed if you call this API
     * during Activity's onCreate.
     *
     * <p class="note">Requires "android.permission.HOTKNOT" permission.     
     *
     * @param callback callback, or null to disable
     * @param activity activity for which the HotKnot message will be sent
     * @param activities optional additional activities, however we strongly recommend
     *        to only register one at a time, and to do so in that Activity's
     *        Activity's onCreate.
     */
    public void setOnHotKnotCompleteCallback(OnHotKnotCompleteCallback callback,
            Activity activity, Activity ... activities) {
        try {
            if (activity == null) {
                throw new NullPointerException("activity cannot be null");
            }
            mHotKnotActivityManager.setOnHotKnotCompleteCallback(activity, callback);
            for (Activity a : activities) {
                if (a == null) {
                    throw new NullPointerException("activities cannot contain null");
                }
                mHotKnotActivityManager.setOnHotKnotCompleteCallback(a, callback);
            }
        } catch (IllegalStateException e) {
            // Prevent new applications from making this mistake, re-throw
            throw(e);
        }
    }
    
    /**
     * Sets up one or more URIs to send using HotKnot. Every
     * URI you provide must have either scheme 'file' or scheme 'content'.
     *
     * <p>For the data provided through this method, HotKnot tries to
     * switch to alternate transports such as Wi-Fi Direct to achieve a fast
     * transfer speed. Hence this method is very suitable
     * for transferring large files such as pictures or songs.
     *
     * <p>The receiving side stores the content of each URI in
     * a file and presents a notification to the user to open the file
     * with a android.content.Intent with action android.content.Intent.ACTION_VIEW.
     * If multiple URIs are sent, android.content.Intent will refer
     * to the first of the stored files.
     *
     * <p>This method may be called any time before Activity's onDestroy,
     * but the URI(s) are only made available for HotKnot when the
     * specified activity(s) are in resumed (foreground) state. The recommended
     * approach is calling this method during your Activity's onCreate (see the sample code below).
     * This method does not immediately perform any I/O or blocking work,
     * so it is safe to call on your main thread.
     *
     * <p>{@link #setHotKnotBeamUris} and {@link #setHotKnotBeamUrisCallback}
     * have priority over both {@link #setHotKnotMessage} and
     * {@link #setHotKnotMessageCallback}.
     *
     * <p>If {@link #setHotKnotBeamUris} is called with a null URI array,
     * and/or {@link #setHotKnotBeamUrisCallback} is called with a null callback,
     * the URI push will be completely disabled for specified activity(s).
     *
     * <p>Code example:
     * <pre>
     * protected void onCreate(Bundle savedInstanceState) {
     *     super.onCreate(savedInstanceState);
     *     HotKnotAdapter hotKnotAdapter = HotKnotAdapter.getDefaultAdapter(this);
     *     if (hotKnotAdapter == null) return;  // HotKnot not available on this device
     *     hotKnotAdapter.setHotKnotBeamUris(new Uri[] {uri1, uri2}, this);
     * }</pre>
     * Only one call per Activity is necessary. The Android
     * OS will automatically release its references to the URI(s) and the
     * Activity object when it is destroyed if you follow this pattern.
     *
     * <p>If your Activity is to dynamically supply URI(s),
     * set a callback using {@link #setHotKnotBeamUrisCallback} instead
     * of using this method.
     *
     * <p class="note">Do not pass in an Activity that has already been through
     * Activity's onDestroy. This is guaranteed if you call this API
     * during Activity's onCreate.
     *
     * <p class="note">Requires "android.permission.HOTKNOT" permission.     
     *     
     * @param uris an array of URI(s) to be sent over HotKnot
     * @param activity activity for which the URI(s) will be pushed     
     */
    public void setHotKnotBeamUris(Uri[] uris, Activity activity) {
        if (activity == null) {
            throw new NullPointerException("activity cannot be null");
        }
        if (uris != null) {
            for (Uri uri : uris) {
                if (uri == null) throw new NullPointerException("Uri not " +
                        "allowed to be null");
                String scheme = uri.getScheme();
                if (scheme == null || (!scheme.equalsIgnoreCase("file") &&
                        !scheme.equalsIgnoreCase("content"))) {
                    throw new IllegalArgumentException("URI needs to have " +
                            "either scheme file or scheme content");
                }
            }
        }
        mHotKnotActivityManager.setHotKnotContent(activity, uris);
    }        

    /**
     * Sets up a callback that dynamically generates one or more URIs
     * to send using HotKnot. Every URI the callback provides
     * must have either scheme 'file' or scheme 'content'.
     *
     * <p>For the data provided through this callback, HotKnot tries to
     * switch to alternate transports such as Wi-Fi Direct to achieve a fast
     * transfer speed. Hence this method is very suitable
     * for transferring large files such as pictures or songs.
     *
     * <p>The receiving side stores the content of each URI in
     * a file and present a notification to the user to open the file
     * with a android.content.Intent with action android.content.Intent.ACTION_VIEW.
     * If multiple URIs are sent, android.content.Intent will refer
     * to the first of the stored files.
     *
     * <p>This method may be called any time before Activity's onDestroy,
     * but the URI(s) are only made available for HotKnot when the
     * specified activity(s) are in resumed (foreground) state. The recommended
     * approach is calling this method during your Activity's
     * Activity's onCreate (see the sample code below).
     * This method does not immediately perform any I/O or blocking work,
     * so it is safe to call on your main thread.
     *
     * <p>{@link #setHotKnotBeamUris} and {@link #setHotKnotBeamUrisCallback}
     * have priority over both {@link #setHotKnotMessage} and
     * {@link #setHotKnotMessageCallback}.
     *
     * <p>If {@link #setHotKnotBeamUris} is called with a null URI array,
     * and/or {@link #setHotKnotBeamUrisCallback} is called with a null callback,
     * the URI push will be completely disabled for specified activity(s).
     *
     * <p>Code example:
     * <pre>
     * protected void onCreate(Bundle savedInstanceState) {
     *     super.onCreate(savedInstanceState);
     *     HotKnotAdapter hotKnotAdapter = HotKnotAdapter.getDefaultAdapter(this);
     *     if (hotKnotAdapter == null) return;  // HotKnot not available on this device
     *     hotKnotAdapter.setHotKnotBeamUrisCallback(callback, this);
     * }</pre>
     * Only one call per Activity is necessary. The Android
     * OS will automatically release its references to the URI(s) and the
     * Activity object when it is destroyed if you follow this pattern.
     *
     * <p class="note">Do not pass in an Activity that has already been through
     * Activity's onDestroy. This is guaranteed if you call this API
     * during Activity's onCreate}.
     *
     * <p class="note">Requires "android.permission.HOTKNOT" permission.     
     * 
     * @param callback callback, or null to disable
     * @param activity activity for which the URI(s) will be pushed     
     */
    public void setHotKnotBeamUrisCallback(CreateHotKnotBeamUrisCallback callback, Activity activity) {
        if (activity == null) {
            throw new NullPointerException("activity cannot be null");
        }
        mHotKnotActivityManager.setHotKnotContentCallback(activity, callback);
    }

}
