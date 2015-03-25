package com.mediatek.hotknot;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Binder;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.mom.IMobileManagerService;
import com.mediatek.common.mom.SubPermissions;

/**
 * Manages HotKnot APIs that are coupled to the life-cycle of an Activity.
 *
 * <p>Uses {@link Application#registerActivityLifecycleCallbacks} to hook
 * into activity life-cycle events such as onPause() and onResume().
 *
 * @hide
 */

public final class HotKnotActivityManager extends IHotKnotCallback.Stub implements Application.ActivityLifecycleCallbacks {
    static final String TAG = HotKnotAdapter.TAG;
    static final Boolean DBG = true;

    final HotKnotAdapter mAdapter;    
    private static IMobileManagerService mMobileManagerService = null;
 
    // All objects in the lists are protected by this
    final List<HotKnotApplicationState> mApps;  // Application(s) that have HotKnot state. Usually one
    final List<HotKnotActivityState> mActivities;  // Activities that have HotKnot state

    /**
     * HotKnot State associated with an {@link Application}.
     */
    class HotKnotApplicationState {
        int refCount = 0;
        final Application app;
        public HotKnotApplicationState(Application app) {
            this.app = app;
        }
        public void register() {
            refCount++;
            if (refCount == 1) {
                this.app.registerActivityLifecycleCallbacks(HotKnotActivityManager.this);
            }
        }
        public void unregister() {
            refCount--;
            if (refCount == 0) {
                this.app.unregisterActivityLifecycleCallbacks(HotKnotActivityManager.this);
            } else if (refCount < 0) {
                Log.e(TAG, "-ve refcount for " + app);
            }
        }
    }

    HotKnotApplicationState findAppState(Application app) {
        for (HotKnotApplicationState appState : mApps) {
            if (appState.app == app) {
                return appState;
            }
        }
        return null;
    }

    void registerApplication(Application app) {
        HotKnotApplicationState appState = findAppState(app);
        if (appState == null) {
            appState = new HotKnotApplicationState(app);
            mApps.add(appState);
        }
        appState.register();
    }

    void unregisterApplication(Application app) {
        HotKnotApplicationState appState = findAppState(app);
        if (appState == null) {
            Log.e(TAG, "app was not registered " + app);
            return;
        }
        appState.unregister();
    } 

    /**
     * HotKnot state associated with an {@link Activity}
     */
    class HotKnotActivityState {
        boolean resumed = false;
        Activity activity;
        HotKnotMessage hotKnotMessage = null;  // static HotKnot message
        HotKnotAdapter.CreateHotKnotMessageCallback hotKnotMessageCallback = null;
        HotKnotAdapter.OnHotKnotCompleteCallback onHotKnotCompleteCallback = null;                
        HotKnotAdapter.CreateHotKnotBeamUrisCallback uriCallback = null;

        Uri[] uris = null;
        int flags = 0;
        Binder token;
        int clientId = 0;

        public HotKnotActivityState(Activity activity) {
            if (activity.getWindow().isDestroyed()) {
                throw new IllegalStateException("activity is already destroyed");
            }
            // Check if activity is resumed right now, as we will not
            // immediately get a callback for that.
            resumed = activity.isResumed();

            this.activity = activity;
            this.token = new Binder();
            this.clientId = this.token.hashCode();
            registerApplication(activity.getApplication());
        }
        public void destroy() {
            unregisterApplication(activity.getApplication());
            resumed = false;
            activity = null;
            hotKnotMessage = null;
            hotKnotMessageCallback = null;
            onHotKnotCompleteCallback = null;                        
            uriCallback = null;
            uris = null;
            token = null;
        }
        @Override
        public String toString() {
            StringBuilder s = new StringBuilder("[").append(" ");
            s.append(hotKnotMessage).append(" ").append(hotKnotMessageCallback != null).append(" ");
            s.append(onHotKnotCompleteCallback != null).append(" ").append("]");
            s.append(uriCallback != null).append(" ");
            if (uris != null) {
                for (Uri uri : uris) {
                    s.append(" [").append(uri).append("]");
                }
            }
            return s.toString();
        }
    }

    /** find activity state from mActivities */
    synchronized HotKnotActivityState findActivityState(Activity activity) {
        for (HotKnotActivityState state : mActivities) {
            if (state.activity == activity) {
                return state;
            }
        }
        return null;
    }

    /** find or create activity state from mActivities */
    synchronized HotKnotActivityState getActivityState(Activity activity) {
        HotKnotActivityState state = findActivityState(activity);
        if (state == null) {
            state = new HotKnotActivityState(activity);
            mActivities.add(state);
        }
        return state;
    }

    synchronized HotKnotActivityState findResumedActivityState() {
        for (HotKnotActivityState state : mActivities) {
            if (state.resumed) {
                return state;
            }
        }
        return null;
    }

    synchronized HotKnotActivityState findActivityStateByClientId(int clientId) {
        for (HotKnotActivityState state : mActivities) {
            if (state.clientId == clientId) {
                return state;
            }
        }
        return null;
    }

    synchronized void destroyActivityState(Activity activity) {
        HotKnotActivityState activityState = findActivityState(activity);
        if (activityState != null) {
            activityState.destroy();
            mActivities.remove(activityState);
        }
    }

    public HotKnotActivityManager(HotKnotAdapter adapter) {
        mAdapter = adapter;
        mActivities = new LinkedList<HotKnotActivityState>();
        mApps = new ArrayList<HotKnotApplicationState>(1);  // Android VM usually has 1 app        
    }

    public void setHotKnotMessage(Activity activity, HotKnotMessage message, int flags) {
        boolean isResumed, isRequest;

        if(message != null) {
            if(!checkMomPermission()) {
                Log.e(TAG, "MOM permission is denied");
                return;
            }
        }

        synchronized (HotKnotActivityManager.this) {
            HotKnotActivityState state = getActivityState(activity);
            state.hotKnotMessage = message;
            state.flags = flags;
            isResumed = state.resumed;
            isRequest = isHotKnotRequired(state);
        }

        Log.d(TAG, "setHotKnotMessage:" + (message != null));

        if (isResumed) {
            requestHotKnotServiceCallback(isRequest);
        }
    }

    public void setHotKnotMessageCallback(Activity activity,
            HotKnotAdapter.CreateHotKnotMessageCallback callback, int flags) {
        boolean isResumed, isRequest;

        if(callback != null) {
            if(!checkMomPermission()) {
                Log.e(TAG, "MOM permission is denied");
                return;
            }
        }

        synchronized (HotKnotActivityManager.this) {
            HotKnotActivityState state = getActivityState(activity);
            state.hotKnotMessageCallback = callback;
            state.flags = flags;
            isResumed = state.resumed;
            isRequest = isHotKnotRequired(state);
        }
        Log.d(TAG, "setHotKnotMessageCallback:" + (callback != null));
        if (isResumed) {
            requestHotKnotServiceCallback(isRequest);
        }
    }
 
     public void setOnHotKnotCompleteCallback(Activity activity,
            HotKnotAdapter.OnHotKnotCompleteCallback callback) {
        boolean isResumed, isRequest;

        synchronized (HotKnotActivityManager.this) {
            HotKnotActivityState state = getActivityState(activity);
            state.onHotKnotCompleteCallback = callback;
            isResumed = state.resumed;
            isRequest = isHotKnotRequired(state);
        }
        Log.d(TAG, "setOnHotKnotCompleteCallback:" + (callback != null));

        if (isResumed) {
            requestHotKnotServiceCallback(isRequest);
        }

    }

    public void setHotKnotContent(Activity activity, Uri[] uris) {
        boolean isResumed, isRequest;

        if(uris != null) {
            if(!checkMomPermission()) {
                Log.e(TAG, "MOM permission is denied");
                return;
            }
        }

        synchronized (HotKnotActivityManager.this) {
            HotKnotActivityState state = getActivityState(activity);
            state.uris = uris;
            isResumed = state.resumed;
            isRequest = isHotKnotRequired(state);
        }
        Log.d(TAG, "setHotKnotContent:" + (uris != null));
        if (isResumed) {
            requestHotKnotServiceCallback(isRequest);
        }
    }

    public void setHotKnotContentCallback(Activity activity,
                                          HotKnotAdapter.CreateHotKnotBeamUrisCallback callback) {
        boolean isResumed, isRequest;

        if(callback != null) {
            if(!checkMomPermission()) {
                Log.e(TAG, "MOM permission is denied");
                return;
            }
        }

        synchronized (HotKnotActivityManager.this) {
            HotKnotActivityState state = getActivityState(activity);
            state.uriCallback = callback;
            isResumed = state.resumed;
            isRequest = isHotKnotRequired(state);
        }
        Log.d(TAG, "setHotKnotContentCallback:" + (callback != null));
        if (isResumed) {
            requestHotKnotServiceCallback(isRequest);
        }
    }

    /**
     * Request or unrequest HotKnot service callbacks.
     * Makes IPC call - do not hold lock.
     */
    void requestHotKnotServiceCallback(boolean request) {
        try {
            HotKnotAdapter.mService.setHotKnotCallback(request ? this : null);
        } catch (RemoteException e) {
            
        }
    }

    boolean isHotKnotRequired(HotKnotActivityState state) {
        return (state.hotKnotMessage != null || state.hotKnotMessageCallback != null || state.uriCallback != null || state.uris != null);
    }

    boolean checkMomPermission() {
        
        if (FeatureOption.MTK_MOBILE_MANAGEMENT) {
            if (mMobileManagerService == null) {
                mMobileManagerService = IMobileManagerService.Stub.asInterface(
                                            ServiceManager.getService(Context.MOBILE_SERVICE));
            }

            try {
                int result = mMobileManagerService.checkPermission(SubPermissions.HOTKNOT_BIND, Binder.getCallingUid());
                if (result != PackageManager.PERMISSION_GRANTED) {
                    return false;
                }
            } catch (RemoteException e) {
                Log.e(TAG, e.getMessage());
            }

        }
        return true;
    }

    /** Callback from HotKnot service, usually on binder thread */
    @Override
    public HotKnotMessage createMessage() {
        HotKnotAdapter.CreateHotKnotMessageCallback hotKnotCallback;        
        HotKnotMessage message;

        synchronized (HotKnotActivityManager.this) {
            HotKnotActivityState state = findResumedActivityState();
            if (state == null) return null;

            hotKnotCallback = state.hotKnotMessageCallback;            
            message = state.hotKnotMessage;
        }        
        // Make callbacks without lock
        if (hotKnotCallback != null) {
            message  = hotKnotCallback.createHotKnotMessage();
        }

        return message;
    }
    

    /** Callback from HotKnot service, usually on binder thread */
    @Override
    public void onHotKnotComplete(int clientId, int reason) {
        HotKnotAdapter.OnHotKnotCompleteCallback callback;
        synchronized (HotKnotActivityManager.this) {
            HotKnotActivityState state = findActivityStateByClientId(clientId);
            if (state == null) return;

            callback = state.onHotKnotCompleteCallback;
        }

        Log.d(TAG, "onHotKnotComplete:" + reason);
        // Make callback without lock
        if (callback != null) {
            callback.onHotKnotComplete(reason);
        }
    }

    /** Callback from HotKnot service, usually on binder thread */
    @Override
    public Uri[] getUris() {
        Uri[] uris;
        HotKnotAdapter.CreateHotKnotBeamUrisCallback callback;
        synchronized (HotKnotActivityManager.this) {
            HotKnotActivityState state = findResumedActivityState();
            if (state == null) return null;
            uris = state.uris;
            callback = state.uriCallback;
        }


        if(uris != null) {
            for (Uri uri : uris) {
                Log.d(TAG, "  uri.toString: " + uri.toString());
            }
        }
        Log.d(TAG, "state.uriCallback:" + callback);

        if (callback != null) {
            uris = callback.createHotKnotBeamUris();
            if (uris != null) {
                for (Uri uri : uris) {
                    if (uri == null) {
                        Log.e(TAG, "Uri not allowed to be null.");
                        return null;
                    }
                    String scheme = uri.getScheme();
                    if (scheme == null || (!scheme.equalsIgnoreCase("file") &&
                                           !scheme.equalsIgnoreCase("content"))) {
                        Log.e(TAG, "Uri needs to have " +
                              "either scheme file or scheme content");
                        return null;
                    }
                }
            }
            return uris;
        } else {

            return uris;
        }
    }

    /** Callback from HotKnot service, usually on binder thread */
    @Override
    public int getClientId() {
        int clientId = -1;
        synchronized (HotKnotActivityManager.this) {
            HotKnotActivityState state = findResumedActivityState();
            if (state != null) {
                clientId = state.clientId;
            }
        }
        return clientId;
    }


    /** Callback from Activity life-cycle, on main thread */
    @Override
    public void onActivityCreated(Activity activity, Bundle savedInstanceState) {
        /* NO-OP */
    }

    /** Callback from Activity life-cycle, on main thread */
    @Override
    public void onActivityStarted(Activity activity) {
        /* NO-OP */
    }

    /** Callback from Activity life-cycle, on main thread */
    @Override
    public void onActivityResumed(Activity activity) {                
        Binder token;
        boolean isRequest;
        synchronized (HotKnotActivityManager.this) {
            HotKnotActivityState state = findActivityState(activity);
            if (DBG) Log.d(TAG, "onResume() for " + activity + " " + state);
            if (state == null) return;
            state.resumed = true;
            token = state.token;            
            isRequest = isHotKnotRequired(state);
        }
        requestHotKnotServiceCallback(isRequest);
    }

/** Callback from Activity life-cycle, on main thread */
    @Override
    public void onActivityPaused(Activity activity) {        
        Binder token;
        synchronized (HotKnotActivityManager.this) {
            HotKnotActivityState state = findActivityState(activity);
            if (DBG) Log.d(TAG, "onPause() for " + activity + " " + state);
            if (state == null) return;
            state.resumed = false;
            token = state.token;            
            requestHotKnotServiceCallback(false);      
        }        
    }

    /** Callback from Activity life-cycle, on main thread */
    @Override
    public void onActivityStopped(Activity activity) {
        /* NO-OP */
    }

    /** Callback from Activity life-cycle, on main thread */
    @Override
    public void onActivitySaveInstanceState(Activity activity, Bundle outState) {
        /* NO-OP */
    }

    /** Callback from Activity life-cycle, on main thread */
    @Override
    public void onActivityDestroyed(Activity activity) {
        synchronized (HotKnotActivityManager.this) {
            HotKnotActivityState state = findActivityState(activity);
            if (DBG) Log.d(TAG, "onDestroy() for " + activity + " " + state);
            if (state != null) {
                // release all associated references
                destroyActivityState(activity);
            }
        }
    }
}