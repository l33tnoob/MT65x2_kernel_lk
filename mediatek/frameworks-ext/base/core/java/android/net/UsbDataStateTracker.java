/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package android.net;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbManager;
import android.net.NetworkInfo.DetailedState;
import android.net.wifi.WifiManager;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.INetworkManagementService;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Slog;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.util.AsyncChannel;

import java.io.CharArrayWriter;
import java.io.PrintWriter;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Collection;
import java.util.Iterator;

import android.net.BaseNetworkStateTracker;
/**
 * Track the state of usb data connectivity. This is done by
 * receiving broadcast intents from the Phone process whenever
 * the state of data connectivity changes.
 *
 * {@hide}
 */
public class UsbDataStateTracker extends BaseNetworkStateTracker {

    private static final String TAG = "UsbDataStateTracker";
    private static final boolean DBG = true;
    private static final boolean VDBG = true;

    private PhoneConstants.DataState mUsbDataState;
    private ITelephony mPhoneService;

    private String mApnType;
    private NetworkInfo mNetworkInfo;
    private boolean mTeardownRequested = false;
    private Handler mTarget;
    private Context mContext;
    private LinkProperties mLinkProperties;
    private LinkCapabilities mLinkCapabilities;
    private boolean mPrivateDnsRouteSet = false;
    private boolean mDefaultRouteSet = false;
    private INetworkManagementService mNetd;
    private ConnectivityManager mCm;
    private boolean mUsbConnected;
    private boolean mMassStorageActive;
    private String[] mTetherableUsbRegexs;
    //private boolean mUsbTethered;
    //private Object mPublicSync;

    private static final int USB_INTERNET_SYSTEM_DEFAULT  = ConnectivityManager.USB_INTERNET_SYSTEM_WINXP;
    private int    mUsbInternetSystemType;
    private static final String USB_IFACE_ADDR[] = {"192.168.0.100","192.168.137.100"};
    private static final int USB_PREFIX_LENGTH        = 24;
    private static final String DEFAULT_DNS1[] = {"192.168.0.1","192.168.137.1"};
    private static final String DEFAULT_DNS2        = "208.67.222.222"; //OpenDns server

    // NOTE: these are only kept for debugging output; actual values are
    // maintained in DataConnectionTracker.
    protected boolean mUserDataEnabled = true;
    protected boolean mPolicyDataEnabled = true;

    private Handler mHandler;
    private AsyncChannel mDataConnectionTrackerAc;
    private Messenger mMessenger;

    private static UsbDataStateReceiver mUsbDataStateReceiver;

    /**
     * Create a new UsbDataStateTracker
     * @param netType the ConnectivityManager network type
     * @param tag the name of this network
     */
    public UsbDataStateTracker(int netType, String tag, INetworkManagementService netd) {
        mNetworkInfo = new NetworkInfo(netType, 0, tag, "internet");
        mNetd = netd;
        mApnType = "usbinternet";
        mUsbInternetSystemType = USB_INTERNET_SYSTEM_DEFAULT;
        //mPublicSync = new Object();
    }

    /**
     * Begin monitoring data connectivity.
     *
     * @param context is the current Android context
     * @param target is the Hander to which to return the events.
     */
    public void startMonitoring(Context context, Handler target) {
        mTarget = target;
        mContext = context;

        mHandler = new UdstHandler(target.getLooper(), this);
        if (mUsbDataStateReceiver == null) {
            mUsbDataStateReceiver = new UsbDataStateReceiver();
            IntentFilter filter = new IntentFilter();
            filter.addAction("mediatek.intent.action.USB_DATA_STATE");
            filter.addAction(TelephonyIntents.ACTION_DATA_CONNECTION_FAILED);
            filter.addAction(UsbManager.ACTION_USB_STATE);
            filter.addAction(Intent.ACTION_MEDIA_SHARED);
            filter.addAction(ConnectivityManager.READY_FOR_USBINTERNET);

            mContext.registerReceiver(mUsbDataStateReceiver, filter);
        }

        mUsbDataState = PhoneConstants.DataState.DISCONNECTED;
    }

    static class UdstHandler extends Handler {
        private UsbDataStateTracker mUdst;

        UdstHandler(Looper looper, UsbDataStateTracker udst) {
            super(looper);
            mUdst = udst;
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case AsyncChannel.CMD_CHANNEL_HALF_CONNECTED:
                    if (msg.arg1 == AsyncChannel.STATUS_SUCCESSFUL) {
                        if (VDBG) {
                            Slog.v(TAG, "UdstHandler connected");
                        }
                        mUdst.mDataConnectionTrackerAc = (AsyncChannel) msg.obj;
                    } else {
                        if (VDBG) {
                            Slog.v(TAG, "UdstHandler %s NOT connected error=" + msg.arg1);
                        }
                    }
                    break;
                case AsyncChannel.CMD_CHANNEL_DISCONNECTED:
                    if (VDBG) Slog.v(TAG, "Disconnected from DataStateTracker");
                    mUdst.mDataConnectionTrackerAc = null;
                    break;
                default: {
                    if (VDBG) Slog.v(TAG, "Ignorning unknown message=" + msg);
                    break;
                }
            }
        }
    }

    private PhoneConstants.DataState valueOf(String s) {

        if ("1".equals(s))
            return PhoneConstants.DataState.CONNECTING;
        if ("2".equals(s))
            return PhoneConstants.DataState.CONNECTED;
        if ("3".equals(s))
            return PhoneConstants.DataState.SUSPENDED;

        return PhoneConstants.DataState.DISCONNECTED;
    }

    private class UsbDataStateReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals("mediatek.intent.action.USB_DATA_STATE")) {
                String apnType = intent.getStringExtra(PhoneConstants.DATA_APN_TYPE_KEY);

                Slog.d(TAG, "Broadcast received: mediatek.intent.action.USB_DATA_STATE");

                if (!TextUtils.equals(apnType, mApnType)) {
                    return;
                }
                mNetworkInfo.setSubtype(TelephonyManager.getDefault().getNetworkType(),
                        TelephonyManager.getDefault().getNetworkTypeName());
                PhoneConstants.DataState state = Enum.valueOf(PhoneConstants.DataState.class,
                        intent.getStringExtra(PhoneConstants.STATE_KEY));
                String reason = intent.getStringExtra(PhoneConstants.STATE_CHANGE_REASON_KEY);
                String apnName = intent.getStringExtra(PhoneConstants.DATA_APN_KEY);
                mNetworkInfo.setRoaming(intent.getBooleanExtra(PhoneConstants.DATA_NETWORK_ROAMING_KEY,
                        false));
                if (VDBG) {
                    Slog.d(TAG, mApnType + " setting isAvailable to " +
                            intent.getBooleanExtra(PhoneConstants.NETWORK_UNAVAILABLE_KEY,false));
                }
                mNetworkInfo.setIsAvailable(!intent.getBooleanExtra(PhoneConstants.NETWORK_UNAVAILABLE_KEY,
                        false));

                if (DBG) {
                    Slog.d(TAG, "Received state=" + state + ", old=" + mUsbDataState +
                        ", reason=" + (reason == null ? "(unspecified)" : reason));
                }

                mUsbInternetSystemType = intent.getIntExtra(ConnectivityManager.USB_INTERNET_SYSTEM_KEY, USB_INTERNET_SYSTEM_DEFAULT);

                if (mUsbDataState != state) {
                    mUsbDataState = state;
                    switch (state) {
                        case DISCONNECTED:
                            if(isTeardownRequested()) {
                                setTeardownRequested(false);
                            }

                            setDetailedState(DetailedState.DISCONNECTED, reason, apnName);
                            // can't do this here - ConnectivityService needs it to clear stuff
                            // it's ok though - just leave it to be refreshed next time
                            // we connect.
                            //if (DBG) log("clearing mInterfaceName for "+ mApnType +
                            //        " as it DISCONNECTED");
                            //mInterfaceName = null;
                            break;
                        case CONNECTING:
                            setDetailedState(DetailedState.CONNECTING, reason, apnName);
                            break;
                        case SUSPENDED:
                            setDetailedState(DetailedState.SUSPENDED, reason, apnName);
                            break;
                        case CONNECTED:
                            mLinkProperties = intent.getParcelableExtra(
                                    PhoneConstants.DATA_LINK_PROPERTIES_KEY);
                            if (mLinkProperties == null) {
                                Slog.e(TAG, "CONNECTED event did not supply link properties.");
                                mLinkProperties = new LinkProperties();
                            }
                            mLinkProperties.addDns(NetworkUtils.numericToInetAddress(DEFAULT_DNS1[mUsbInternetSystemType]));
                            mLinkProperties.addDns(NetworkUtils.numericToInetAddress(DEFAULT_DNS2));

                            InetAddress addr = NetworkUtils.numericToInetAddress(USB_IFACE_ADDR[mUsbInternetSystemType]);
                            mLinkProperties.addLinkAddress(new LinkAddress(addr, USB_PREFIX_LENGTH));

                            mLinkCapabilities = intent.getParcelableExtra(
                                    PhoneConstants.DATA_LINK_CAPABILITIES_KEY);
                            if (mLinkCapabilities == null) {
                                Slog.e(TAG, "CONNECTED event did not supply link capabilities.");
                                mLinkCapabilities = new LinkCapabilities();
                            }
                            setDetailedState(DetailedState.CONNECTED, reason, apnName);
                            break;
                    }
                } else {
                    // There was no state change. Check if LinkProperties has been updated.
                    if (TextUtils.equals(reason, PhoneConstants.REASON_LINK_PROPERTIES_CHANGED)) {
                        mLinkProperties = intent.getParcelableExtra(PhoneConstants.DATA_LINK_PROPERTIES_KEY);
                        if (mLinkProperties == null) {
                            Slog.e(TAG, "No link property in LINK_PROPERTIES change event.");
                            mLinkProperties = new LinkProperties();
                        }
                        // Just update reason field in this NetworkInfo
                        mNetworkInfo.setDetailedState(mNetworkInfo.getDetailedState(), reason,
                                                      mNetworkInfo.getExtraInfo());
                        Message msg = mTarget.obtainMessage(EVENT_CONFIGURATION_CHANGED,
                                                            mNetworkInfo);
                        msg.sendToTarget();
                    }
                }
            } else if (intent.getAction().
                    equals(ConnectivityManager.READY_FOR_USBINTERNET)){
                    onHandleReadyForUsbInternet();

            } else {
                if (DBG) Slog.d(TAG, "Broadcast received: ignore " + intent.getAction());
            }
        }
    }

    private void checkNullmCm() {
        if ( mCm == null )
            mCm = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
    }
    /**
     * Record the detailed state of a network, and if it is a
     * change from the previous state, send a notification to
     * any listeners.
     * @param state the new @{code DetailedState}
     * @param reason a {@code String} indicating a reason for the state change,
     * if one was supplied. May be {@code null}.
     * @param extraInfo optional {@code String} providing extra information about the state change
     */
    private void setDetailedState(NetworkInfo.DetailedState state, String reason,
            String extraInfo) {
        if (DBG) Slog.d(TAG, "setDetailed state, old ="
                + mNetworkInfo.getDetailedState() + " and new state=" + state);
        if (state != mNetworkInfo.getDetailedState()) {
            boolean wasConnecting = (mNetworkInfo.getState() == NetworkInfo.State.CONNECTING);
            String lastReason = mNetworkInfo.getReason();
            /*
             * If a reason was supplied when the CONNECTING state was entered, and no
             * reason was supplied for entering the CONNECTED state, then retain the
             * reason that was supplied when going to CONNECTING.
             */
            if (wasConnecting && state == NetworkInfo.DetailedState.CONNECTED && reason == null
                    && lastReason != null)
                reason = lastReason;
            mNetworkInfo.setDetailedState(state, reason, extraInfo);
            Message msg = mTarget.obtainMessage(EVENT_STATE_CHANGED, new NetworkInfo(mNetworkInfo));
            msg.sendToTarget();
        }
    }

    /**
     * carrier dependency is met/unmet
     * @param met
     */
    public void setDependencyMet(boolean met) {
        // not supported on this network
    }

    public boolean isTeardownRequested() {
        return mTeardownRequested;
    }

    public void setTeardownRequested(boolean isRequested) {
        mTeardownRequested = isRequested;
    }

    public void defaultRouteSet(boolean enabled) {
        mDefaultRouteSet = enabled;
    }

    public boolean isDefaultRouteSet() {
        return mDefaultRouteSet;
    }

    public boolean isPrivateDnsRouteSet() {
        return mPrivateDnsRouteSet;
    }

    public void privateDnsRouteSet(boolean enabled) {
        mPrivateDnsRouteSet = enabled;
    }



    private boolean configureUsbIface(boolean enabled) {
            if (VDBG) Slog.d(TAG, "configureUsbIface(" + enabled + ")");

            // toggle the USB interfaces
            mTetherableUsbRegexs = mContext.getResources().getStringArray(
                com.android.internal.R.array.config_tether_usb_regexs);

            String[] ifaces = new String[0];
            try {
                ifaces = mNetd.listInterfaces();
            } catch (Exception e) {
                Slog.d(TAG, "Error listing Interfaces", e);
                return false;
            }
            for (String iface : ifaces) {
                if (isUsb(iface)) {
                    InterfaceConfiguration ifcg = null;
                    try {
                        ifcg = mNetd.getInterfaceConfig(iface);
                        if (ifcg != null) {
                            InetAddress addr = NetworkUtils.numericToInetAddress(USB_IFACE_ADDR[mUsbInternetSystemType]);
                            ifcg.setLinkAddress(new LinkAddress(addr, USB_PREFIX_LENGTH));
                            if (enabled) {
                                ifcg.setInterfaceUp();
                            } else {
                                ifcg.setInterfaceDown();
                            }
                            ifcg.clearFlag("running");
                            mNetd.setInterfaceConfig(iface, ifcg);
                            mNetd.cfgUsbInternetAddress(DEFAULT_DNS1[mUsbInternetSystemType]);
                        }
                    } catch (RemoteException e) {
                        Slog.d(TAG, "Error configuring interface " + iface, e);
                    } catch (Exception e) {
                        Slog.d(TAG, "Error configuring interface " + iface, e);
                        return false;
                    }

                }
             }

            return true;
        }

    private boolean isUsb(String iface) {
            for (String regex : mTetherableUsbRegexs) {
                if (iface.matches(regex)) return true;
            }
            return false;
    }

    private void onHandleReadyForUsbInternet() {
        Slog.d(TAG, "onHandleReadyForUsbInternet");
        if (NetworkInfo.DetailedState.CONNECTED == mNetworkInfo.getDetailedState()) {
            configureUsbIface(true);
        }
    }

    /*private void setUsbTethered(boolean v) {
        synchronized (mPublicSync) {
            mUsbTethered = v;
        }
    }

    private boolean getUsbTethered() {
        synchronized (mPublicSync) {
            return mUsbTethered;
        }
    }*/

    /**
     * Re-enable usb data connectivity after a {@link #teardown()}.
     * TODO - make async and always get a notification?
     */
    public boolean reconnect() {
        Slog.d(TAG, "reconnect");
        boolean retValue = false; //connected or expect to be?
        setTeardownRequested(false);
        WifiManager wifimgr = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
        wifimgr.setWifiEnabled(false);

        checkNullmCm();
        mCm.setMobileDataEnabled(false);
        mCm.setUsbTethering(true);

        return retValue;
    }


    /**
     * Tear down mobile data connectivity, i.e., disable the ability to create
     * mobile data connections.
     * TODO - make async and return nothing?
     */
    public boolean teardown() {
        setTeardownRequested(true);
        Slog.d(TAG, "teardown");

        checkNullmCm();
        mCm.setUsbTethering(false);
        /*try {
            mNetd.stopUsbInternet();
        } catch (RemoteException e) {
            Slog.e(TAG, "stopUsbInternet err:" + e);
        }*/

        return true;
    }

    /**
     * Turn on or off the mobile radio. No connectivity will be possible while the
     * radio is off. The operation is a no-op if the radio is already in the desired state.
     * @param turnOn {@code true} if the radio should be turned on, {@code false} if
     */
    public boolean setRadio(boolean turnOn) {
        return true;
    }

    @Override
    public void setUserDataEnable(boolean enabled) {
        Slog.w(TAG, "ignoring setUserDataEnable(" + enabled + ")");
    }

    @Override
    public void setPolicyDataEnable(boolean enabled) {
        Slog.w(TAG, "ignoring setPolicyDataEnable(" + enabled + ")");
    }

    private void getPhoneService(boolean forceRefresh) {
        if ((mPhoneService == null) || forceRefresh) {
            mPhoneService = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
        }
    }

    @Override
    public void captivePortalCheckComplete() {
        // not implemented
    }

    /**
     * Report whether data connectivity is possible.
     */
    public boolean isAvailable() {
        return mNetworkInfo.isAvailable();
    }

    public String getTcpBufferSizesPropName() {
        return "net.tcp.buffersize.wifi";
    }

    /**
     * @see android.net.NetworkStateTracker#getLinkProperties()
     */
    public LinkProperties getLinkProperties() {
        return new LinkProperties(mLinkProperties);
    }

    /**
     * @see android.net.NetworkStateTracker#getLinkCapabilities()
     */
    public LinkCapabilities getLinkCapabilities() {
        return new LinkCapabilities(mLinkCapabilities);
    }

    public NetworkInfo getNetworkInfo() {
        return mNetworkInfo;
    }

    @Override
    public void addStackedLink(LinkProperties link) {
        mLinkProperties.addStackedLink(link);
    }

    @Override
    public void removeStackedLink(LinkProperties link) {
        mLinkProperties.removeStackedLink(link);
    }
}
