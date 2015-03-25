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

package com.mediatek.FMTransmitter;

import android.app.Activity;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.view.View;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.custom.CustomProperties;

import java.util.Arrays;
import java.util.Locale;

public class FMTransmitterActivity extends Activity {
    public static final String TAG = "FMTx/FMTransmitterActivity";
    protected static FMTransmitterActivity sRefThis = null;
    private boolean mIsPlaying = false;
    private boolean mIsDoingPlayStop = false;
    private boolean mIsSeeking = false;
    private boolean mIsServiceStarted = false;
    private boolean mIsServiceBinded = false;
    private boolean mIsDestroying = false;
    private ServiceConnection mServiceConnection = null;
    private IFMTransmitterService mService = null;
    // private Handler mHandler = null;

    private Toast mToast = null;

    // private static final String TYPE_MSGID = "MSGID";
    // private static final int MSGID_PLAYSTOP_FINISH = 1;
    // private static final int MSGID_SEEK_FINISH = 2;

     private static FragmentManager sFragmentManager = null;

//    private static final String FMTRANSMITTERACTIVITY_FRAGMENT_TAG = "FMTransmitterTag";

    // Seek parameters.
    private static final int SEEK_CHANNEL_COUNT = 1;
    private static final int SEEK_CHANNEL_DIRECTION = 0;
    private static final int SEEK_CHANNEL_GAP = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 5 : 1;
    private static final int RDS_LENGTH_MAX = 8;
    private static final String CURRENTSTATION = "currentstation";
    public int mCurrentStation = FMTransmitterStation.FIXED_STATION_FREQ; // 100.0
                                                                         // MHz
    private static final int REQUEST_CODE_ADVANCED = 1;
    private static final int BASE_NUMBER = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 100 : 10;

    private ImageButton mButtonPlayStop = null;
    private ImageButton mButtonSeek = null;
    private ImageButton mButtonAdvanced = null;
    private TextView mTextStationValue = null;
    private Context mContext;
    PlayFMTxAsyncTask mPlayFMTxAsyncTask = null;
    SeekAsyncTask mSeekAsyncTask = null;
    private static final String FMTANSMITTERACTIVITY_FRAGMENT_TAG = "dialog";

    private enum TxDeviceStateEnum {
        TXOPENED, TXPOWERUP, TXCLOSED
    };

    private class FMTxAppBroadcastReceiver extends BroadcastReceiver {
        public void onReceive(Context context, Intent intent) {
            FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.onReceive");
            String action = intent.getAction();
            FMTxLogUtils.v(TAG, "Context: " + context);
            FMTxLogUtils.v(TAG, "Action: " + action);
            if (action.equals(FMTransmitterService.ACTION_STATE_CHANGED)) {
                mIsPlaying = intent.getBooleanExtra(
                        FMTransmitterService.EXTRA_FMTX_ISPOWERUP, false);
                enableAllButtons(true);
                refreshAllButtonsImages();
                refreshAllButtonsStatus();
            }

            FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.onReceive");
        }
    }

    private FMTxAppBroadcastReceiver mBroadcastReceiver = null;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.onCreate");
        
        sFragmentManager = getFragmentManager();
        sRefThis = this;
        setContentView(R.layout.main);
        mContext = getApplicationContext();
        uiComponentInit();
        // registerButtonClickListenner();
        // registerHandler();
        // Register broadcast receiver.
        IntentFilter filter = new IntentFilter();
        filter.addAction(FMTransmitterService.ACTION_STATE_CHANGED);
        mBroadcastReceiver = new FMTxAppBroadcastReceiver();
        FMTxLogUtils.i(TAG, "Register Tx broadcast receiver.");
        registerReceiver(mBroadcastReceiver, filter);
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.onCreate");
    }

    private void uiComponentInit() {
        TextView textView = null;
        TextView textMhz = null;
        textView = (TextView) findViewById(R.id.text_fm);
        textView.setText("FM");
        textMhz = (TextView) findViewById(R.id.text_mhz);
        textMhz.setText("MHz");

        mCurrentStation = FMTransmitterStation.getCurrentStation(mContext);
        mTextStationValue = (TextView) findViewById(R.id.station_value);
        if (FeatureOption.MTK_FM_50KHZ_SUPPORT) {
            mTextStationValue.setTextSize(50);
        }
        mTextStationValue.setText(formatStation(mCurrentStation));

        mButtonPlayStop = (ImageButton) findViewById(R.id.button_play_stop);
        mButtonAdvanced = (ImageButton) findViewById(R.id.button_advanced);
        mButtonSeek = (ImageButton) findViewById(R.id.button_seek);
        mButtonPlayStop.setOnClickListener(mButtonClickListener);
        mButtonAdvanced.setOnClickListener(mButtonClickListener);
        mButtonSeek.setOnClickListener(mButtonClickListener);
    }

    /**
     * button click listeners on UI
     */
    private View.OnClickListener mButtonClickListener = new View.OnClickListener() {

        public void onClick(View v) {
            switch (v.getId()) {
            case R.id.button_play_stop:
                FMTxLogUtils.d(TAG, ">>> onClick PlayStop");
                handlePowerClick();
                FMTxLogUtils.d(TAG, "<<< onClick PlayStop");
                break;

            case R.id.button_seek:
                FMTxLogUtils.d(TAG, ">>> onClick Seek");
                handleSeekClick();
                FMTxLogUtils.d(TAG, "<<< onClick Seek");
                break;

            case R.id.button_advanced:
                FMTxLogUtils.d(TAG, ">>> onClick Advanced");
                // Show advanced activity.
                Bundle b = new Bundle();
                b.putInt(CURRENTSTATION, mCurrentStation);

                Intent intent = new Intent();
                intent.setClass(FMTransmitterActivity.this,
                        FMTransmitterAdvanced.class);
                intent.putExtras(b);
                startActivityForResult(intent, REQUEST_CODE_ADVANCED);
                FMTxLogUtils.d(TAG, "<<< onClick Advanced");
                break;

            default:
                break;
            }
        }
    };

    private void handleSeekClick() {
        if (mIsSeeking) {
            FMTxLogUtils.e(TAG, "Error: already seeking.");
        } else {
            if (null != mSeekAsyncTask && !mSeekAsyncTask.isCancelled()) {
                mSeekAsyncTask.cancel(true);
                mSeekAsyncTask = null;
            }
            mSeekAsyncTask = new SeekAsyncTask();
            mSeekAsyncTask.execute();
        }
    }

    private void handlePowerClick() {
        if (mIsDoingPlayStop) {
            FMTxLogUtils.e(TAG, "Error: already doing play/stop.");
        } else if (isEarphonePluged()) {
            // When earphone is plugged, should not power up FM Tx.
            FMTxLogUtils.w(TAG,
                    "Warning: do not power up Tx when earphone is plugged.");
            showToast(getString(R.string.toast_plugout_earphone));
        } else {
            // new FMTxAsyncTask to play FM.
            if (null != mPlayFMTxAsyncTask && !mPlayFMTxAsyncTask.isCancelled()) {
                mPlayFMTxAsyncTask.cancel(true);
                mPlayFMTxAsyncTask = null;
            }
            mPlayFMTxAsyncTask = new PlayFMTxAsyncTask();
            mPlayFMTxAsyncTask.execute();            
        }

    }

    public void onStart() {
        super.onStart();
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.onStart");
        // Should start FM Tx service first.
        ComponentName cn = startService(new Intent(FMTransmitterActivity.this,
                FMTransmitterService.class));
        if (null != cn) {
            FMTxLogUtils.d(TAG, "Start FM Tx service successfully.");
            mIsServiceStarted = true;
            mServiceConnection = new ServiceConnection() {
                public void onServiceConnected(ComponentName className,
                        IBinder service) {
                    FMTxLogUtils.d(TAG,
                            ">>> FMTransmitterActivity.onServiceConnected");
                    mService = IFMTransmitterService.Stub.asInterface(service);
                    if (null == mService) {
                        FMTxLogUtils.e(TAG, "Error: null interface");
                        finish();
                    } else {
                        if (!isServiceInit()) {
                            FMTxLogUtils.i(TAG, "FM Tx service is not init.");
                            initService((float) mCurrentStation / BASE_NUMBER);
                            mIsPlaying = isTxPowerUp();
                            refreshAllButtonsImages();
                            refreshAllButtonsStatus();
                        } else {
                            FMTxLogUtils.i(TAG,
                                    "FM Tx service is already init.");
                            // Get the current frequency in service and save it
                            // into database.
                            int iFreq = (int) (getCurFrequency() * BASE_NUMBER);
                            if ((iFreq >= FMTransmitterStation.LOWEST_STATION)
                                    && (iFreq <= FMTransmitterStation.HIGHEST_STATION)) {
                                if (mCurrentStation != iFreq) {
                                    FMTxLogUtils
                                            .i(TAG,
                                                    "The frequency in FM Tx service is not same as in database.");
                                    // FMTxEM change the data.?? Need to set the
                                    // same as User mode.
                                    // mCurrentStation = iFreq;
                                    // Save the current station frequency into
                                    // data base.
                                    FMTransmitterStation.setCurrentStation(
                                            mContext, mCurrentStation);
                                    // Change the station frequency displayed.
                                    mTextStationValue.setText(formatStation(mCurrentStation));
                                }
                            }

                            if (!isSearching()) {
                                mIsPlaying = isTxPowerUp();
                                refreshAllButtonsImages();
                                refreshAllButtonsStatus();
                                try {
                                    if (!mService.isSearching() && sFragmentManager != null) {
                                        DialogFragment prefragdialog = (DialogFragment) sFragmentManager.findFragmentByTag(
                                                FMTANSMITTERACTIVITY_FRAGMENT_TAG);
                                        if (prefragdialog != null) {
                                            prefragdialog.dismissAllowingStateLoss();
                                        }
                                    }
                                } catch (RemoteException e) {
                                    FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
                                }
                            }
                        }
                    }
                    FMTxLogUtils.d(TAG,
                            "<<< FMTransmitterActivity.onServiceConnected");
                }

                public void onServiceDisconnected(ComponentName className) {
                    FMTxLogUtils.d(TAG,
                            ">>> FMTransmitterActivity.onServiceDisconnected");
                    mService = null;
                    FMTxLogUtils.d(TAG,
                            "<<< FMTransmitterActivity.onServiceDisconnected");
                }
            };
            mIsServiceBinded = bindService(new Intent(
                    FMTransmitterActivity.this, FMTransmitterService.class),
                    mServiceConnection, Context.BIND_AUTO_CREATE);
        }
        if (!mIsServiceBinded) {
            FMTxLogUtils.e(TAG, "Error: Cannot bind FM Tx service");
            finish();
            FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.onCreat");
            return;
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.onStart");
    }


    public void onResume() {
        super.onResume();
        FMTxLogUtils.d(TAG, "FMTransmitterActivity.onResume");
    }

    public void onPause() {
        super.onPause();
        FMTxLogUtils.d(TAG, "FMTransmitterActivity.onPause");
    }

    public void onStop() {
        super.onStop();
        // Unbind the FM service.
        if (mIsServiceBinded) {
            unbindService(mServiceConnection);
            mIsServiceBinded = false;
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.onStop");
    }

    public void onDestroy() {
        super.onDestroy();
        mIsDestroying = true;
        // Unregister the broadcast receiver.
        if (null != mBroadcastReceiver) {
            FMTxLogUtils.v(TAG, "Unregister Tx broadcast receiver.");
            unregisterReceiver(mBroadcastReceiver);
            mBroadcastReceiver = null;
        }
        switch (getTxStatus()) {
        case TXOPENED:
            if (!closeTxDevice()) {
                FMTxLogUtils.e(TAG, "Error: FM Tx device is not closed");
            }
            break;
        case TXPOWERUP:
            FMTxLogUtils.v(TAG, "FM Tx device also can work");
            break;
        case TXCLOSED:
            if (mIsServiceStarted) {
                boolean bRes = stopService(new Intent(
                        FMTransmitterActivity.this, FMTransmitterService.class));
                if (!bRes) {
                    FMTxLogUtils.e(TAG, "Error: Cannot stop the FM service.");
                }
                mIsServiceStarted = false;
            }
            FMTxLogUtils.v(TAG, "FM Tx device is closed");
            break;
        default:
            break;
        }
        sRefThis = null;
        sFragmentManager = null;
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.onDestroy");
    }

    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        FMTxLogUtils.d(TAG, "FMTransmitterActivity.onConfigurationChanged");
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (RESULT_OK == resultCode) {
            if (REQUEST_CODE_ADVANCED == requestCode) {
                int iStation = data.getIntExtra(
                        FMTransmitterAdvanced.ACTIVITY_RESULT, mCurrentStation);
                mCurrentStation = iStation;
                // Save the current station frequency into data base.
                FMTransmitterStation.setCurrentStation(mContext,
                        mCurrentStation);
                // Update UI.
                mTextStationValue.setText(formatStation(mCurrentStation));
                // Tune to this channel.
                if (!isTxPowerUp() && !isEarphonePluged()) {
                    powerUpTx((float) mCurrentStation / BASE_NUMBER);
                }
                if (isTxPowerUp()) {
                    FMTxLogUtils.v(TAG, "Tx is power up");
                    if (turnToFrequency((float) iStation / BASE_NUMBER)) {
                        FMTxLogUtils.v(TAG, "turnToFrequency succed.");
                    }
                }
            }
        } else {
            // Do not handle other result.
            FMTxLogUtils.v(TAG, "The activity for requestcode " + requestCode
                    + " does not return any data.");
        }
        long endTuneTime = 0;
        endTuneTime = System.currentTimeMillis();
        FMTxLogUtils.i(TAG,
                "[Performance test][FMTransmitter] Test FM Tx Tune end ["
                        + endTuneTime + "]");
    }

    private void refreshAllButtonsImages() {
        FMTxLogUtils
                .v(TAG, ">>> FMTransmitterActivity.refreshAllButtonsImages");
        // Refresh button images.
        if (mIsPlaying) {
            mButtonPlayStop.setImageResource(R.drawable.fmtx_started);
            mButtonSeek.setImageResource(R.drawable.fmtx_seek);
            mButtonAdvanced.setImageResource(R.drawable.fmtx_setting);
        } else {
            mButtonPlayStop.setImageResource(R.drawable.fmtx_stop);
            mButtonSeek.setImageResource(R.drawable.fmtx_seek_dis);
            mButtonAdvanced.setImageResource(R.drawable.fmtx_setting_dis);
        }
        FMTxLogUtils
                .v(TAG, "<<< FMTransmitterActivity.refreshAllButtonsImages");
    }

    private void refreshAllButtonsStatus() {
        FMTxLogUtils
                .v(TAG, ">>> FMTransmitterActivity.refreshAllButtonsStatus");
        // Refresh button enable/disable status.
        if (mIsPlaying) {
            mButtonSeek.setEnabled(true);
            mButtonAdvanced.setEnabled(true);
        } else {
            mButtonSeek.setEnabled(false);
            mButtonAdvanced.setEnabled(false);
        }
        FMTxLogUtils
                .v(TAG, "<<< FMTransmitterActivity.refreshAllButtonsStatus");
    }

    private void enableAllButtons(boolean enable) {
        FMTxLogUtils.v(TAG, ">>> FMTransmitterActivity.enableAllButtons: "
                + enable);
        mButtonPlayStop.setEnabled(enable);
        mButtonSeek.setEnabled(enable);
        mButtonAdvanced.setEnabled(enable);
        FMTxLogUtils.v(TAG, "<<< FMTransmitterActivity.enableAllButtons");
    }

    private void showToast(CharSequence text) {
        FMTxLogUtils.v(TAG, ">>> FMTransmitterActivity.showToast: " + text);
        if (null == mToast) {
            mToast = Toast.makeText(FMTransmitterActivity.this, text,
                    Toast.LENGTH_SHORT);
        }
        mToast.show();
        FMTxLogUtils.v(TAG, "<<< FMTransmitterActivity.showToast");
    }

    private void enableRDS() {
        FMTxLogUtils.v(TAG, ">>> FMTransmitterActivity.enableRDS");
        if (isRDSTxSupport()) {
            if (isRDSOn() || setRDSTxEnabled(true)) {
                if (isRDSOn() || setRDSTxEnabled(true)) {
                    if (!FeatureOption.MTK_BSP_PACKAGE) {
                        String defaultValue = "Mediatek";
                        String value = CustomProperties.getString(
                                CustomProperties.MODULE_FMTRANSMITTER,
                                CustomProperties.RDS_VALUE,defaultValue);
                        int rdsLength = (value.length() > RDS_LENGTH_MAX ? RDS_LENGTH_MAX : value.length());             
                        char[] ps = new char[rdsLength];
                        for (int i = 0;i < rdsLength;i++) {
                            ps[i] = value.charAt(i);
                        }                
                        setRDSText(mCurrentStation, ps, null, 0);
                        defaultValue = null;
                        value = null;
                        rdsLength = 0;
                        ps = null;
                    } else {
                        char[] ps = { 'M', 'e', 'd', 'i', 'a', 't', 'e', 'k' };
                        setRDSText(mCurrentStation, ps, null, 0);
                    }
                }
            }
        }
        FMTxLogUtils.v(TAG, "<<< FMTransmitterActivity.enableRDS");
    }

    private void disableRDS() {
        FMTxLogUtils.v(TAG, ">>> FMTransmitterActivity.disableRDS");
        if (isRDSOn()) {
            setRDSTxEnabled(false);
        }
        FMTxLogUtils.v(TAG, "<<< FMTransmitterActivity.disableRDS");
    }

    /**
     * Get FMTx device status
     * 
     * @return FMTx device status which can be open,powerup,closed
     */
    private TxDeviceStateEnum getTxStatus() {
        FMTxLogUtils.d(TAG, "getTxStatus");
        if (isTxPowerUp()) {
            FMTxLogUtils.v(TAG, "tx state -> power up.");
            return TxDeviceStateEnum.TXPOWERUP;

        } else if (isTxDeviceOpen()) {
            FMTxLogUtils.v(TAG, "tx state -> open.");
            return TxDeviceStateEnum.TXOPENED;

        }
        return TxDeviceStateEnum.TXCLOSED;
    }

    private String formatStation(int station) {
        String result = null;
        if (FeatureOption.MTK_FM_50KHZ_SUPPORT) {
            result = String.format(Locale.ENGLISH, "%.2f",  (float)station / BASE_NUMBER);
        } else {
            result = String.format(Locale.ENGLISH, "%.1f",  (float)station / BASE_NUMBER);
        }
        return result;
    }
    
    // //////////////////////////////////////////////////////////////////////////////
    // Wrap service interfaces.
    public boolean openTxDevice() {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.openTxDevice");
        boolean bRet = false;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mService.openTxDevice();
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.openTxDevice: " + bRet);
        return bRet;
    }

    public boolean closeTxDevice() {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.closeTxDevice");
        boolean bRet = false;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mService.closeTxDevice();
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.closeTxDevice: " + bRet);
        return bRet;
    }

    public boolean isTxDeviceOpen() {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.isTxDeviceOpen");
        boolean bRet = false;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mService.isTxDeviceOpen();
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils
                .d(TAG, "<<< FMTransmitterActivity.isTxDeviceOpen: " + bRet);
        return bRet;
    }

    public boolean powerUpTx(float frequency) {
        FMTxLogUtils
                .d(TAG, ">>> FMTransmitterActivity.powerUpTx: " + frequency);
        boolean bRet = false;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mService.powerUpTx(frequency);
                if (bRet) {
                    enableRDS();
                }
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.powerUpTx: " + bRet);
        return bRet;
    }

    public boolean powerDownTx() {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.powerDownTx");
        boolean bRet = false;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                disableRDS();
                bRet = mService.powerDownTx();
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.powerDownTx: " + bRet);
        return bRet;
    }

    public boolean isTxPowerUp() {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.isTxPowerUp");
        boolean bRet = false;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mService.isTxPowerUp();
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.isTxPowerUp: " + bRet);
        return bRet;
    }

    public boolean isSearching() {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.isSearching");
        boolean bRet = false;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mService.isSearching();
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.isSearching: " + bRet);
        return bRet;
    }

    public boolean turnToFrequency(float frequency) {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.turnToFrequency: "
                + frequency);
        boolean bRet = false;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mService.turnToFrequency(frequency);
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.turnToFrequency: "
                + bRet);
        return bRet;
    }

    public boolean initService(float frequency) {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.initService: "
                + frequency);
        boolean bRet = false;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mService.initService(frequency);
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.initService: " + bRet);
        return bRet;
    }

    public boolean isServiceInit() {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.isServiceInit");
        boolean bRet = false;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mService.isServiceInit();
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.isServiceInit: " + bRet);
        return bRet;
    }

    public float getCurFrequency() {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.getCurFrequency");
        // ...
        float ret = (float) FMTransmitterStation.FIXED_STATION_FREQ
                / BASE_NUMBER;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                ret = mService.getCurFrequency();
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils
                .d(TAG, "<<< FMTransmitterActivity.getCurFrequency: " + ret);
        return ret;
    }

    public float[] searchChannelsForTx(float frequency, int direction,
            int number) {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.searchChannelsForTx: "
                + frequency + direction + number);
        float[] ret = null;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                ret = mService
                        .searchChannelsForTx(frequency, direction, number);
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.searchChannelsForTx: "
                + Arrays.toString(ret));
        return ret;
    }

    public boolean isRDSTxSupport() {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.isRDSTxSupport");
        boolean bRet = false;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mService.isRDSTxSupport();
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils
                .d(TAG, "<<< FMTransmitterActivity.isRDSTxSupport: " + bRet);
        return bRet;
    }

    public boolean isRDSOn() {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.isRDSOn");
        boolean bRet = false;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mService.isRDSOn();
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.isRDSOn: " + bRet);
        return bRet;
    }

    public boolean setRDSTxEnabled(boolean state) {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.setRDSTxEnabled: "
                + state);
        boolean bRet = false;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mService.setRDSTxEnabled(state);
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.setRDSTxEnabled: "
                + bRet);
        return bRet;
    }

    public boolean setRDSText(int pi, char[] ps, int[] rdsText, int rdsCnt) {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterActivity.setRDSText.");
        boolean bRet = false;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mService.setRDSText(pi, ps, rdsText, rdsCnt);
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils.d(TAG, "<<< FMTransmitterActivity.setRDSText: " + bRet);
        return bRet;
    }

    public boolean isEarphonePluged() {
        FMTxLogUtils.d(TAG, ">>> isEarphonePluged()");
        boolean bRet = false;
        if (null == mService) {
            FMTxLogUtils.e(TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mService.isEarphonePluged();
            } catch (RemoteException e) {
                FMTxLogUtils.e(TAG, "Exception: Cannot call service function.");
            }
        }
        FMTxLogUtils.d(TAG, "<<< isEarphonePluged(): " + bRet);
        return bRet;
    }

    private class PlayFMTxAsyncTask extends AsyncTask<Void, Void, Void> {

        @Override
        protected void onPreExecute() {
            mIsDoingPlayStop = true;
            enableAllButtons(false);
        }

        @Override
        protected Void doInBackground(Void... arg0) {
            switch (getTxStatus()) {
            case TXOPENED:
                FMTxLogUtils.v(TAG, "Device is open,then power up tx");
                // power up Tx
                if (powerUpTx((float) mCurrentStation / BASE_NUMBER)) {
                    mIsPlaying = true;
                    FMTxLogUtils.i(TAG, "power up successfully");
                }
                break;
            case TXPOWERUP:
                FMTxLogUtils
                        .v(TAG,
                                "Device is power up,then power down tx and close device");
                // power down tx.
                if (powerDownTx() && closeTxDevice()) {
                    mIsPlaying = false;
                    FMTxLogUtils.i(TAG, "power down successfully");
                }
                break;
            case TXCLOSED:
                FMTxLogUtils.v(TAG,
                        "Device is closed,then open device and power up tx");
                // first open device, if succeed, then power up Tx
                if (openTxDevice()) {
                    FMTxLogUtils.i(TAG, "Error: FM Tx device is open");
                    if (powerUpTx((float) mCurrentStation / BASE_NUMBER)) {
                        mIsPlaying = true;
                        FMTxLogUtils.i(TAG, "power up successfully");
                    }
                }

                // the earphone is inserted during fmtx power up,in this case,we
                // should powerdownTx
                if (isEarphonePluged()) {
                    powerDownTx();
                    closeTxDevice();
                    mIsPlaying = false;
                    FMTxLogUtils.i(TAG,
                            "earphone inserted,and power down successfully");
                }

                break;
            default:
                break;
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            mIsDoingPlayStop = false;
            if (mIsDestroying) {
                FMTxLogUtils.w(TAG, "Warning: app is being destroyed.");
                return;
            }
            long endPowerTime = 0;
            enableAllButtons(true);
            refreshAllButtonsImages();
            refreshAllButtonsStatus();
            endPowerTime = System.currentTimeMillis();
            FMTxLogUtils.i(TAG,
                    "[Performance test][FMTransmitter] Test FM Tx Power on end ["
                            + endPowerTime + "]");
            FMTxLogUtils.i(TAG,
                    "[Performance test][FMTransmitter] Test FM Tx Power down end ["
                            + endPowerTime + "]");
        }

    }

    private class SeekAsyncTask extends AsyncTask<Void, Void, Void> {

        @Override
        protected void onPreExecute() {
            FMTxLogUtils.d(TAG, "onPreExecute() called!");
            mIsSeeking = true;    
            if (sFragmentManager != null) {
                FMTxLogUtils.d(TAG, "sFragmentManager != null in onPreExecute()");                
                DialogFragment newFragment = ProgressDialogFragment.newInstance();
                newFragment.setCancelable(false);
                newFragment.show(sFragmentManager, FMTANSMITTERACTIVITY_FRAGMENT_TAG);
                sFragmentManager.executePendingTransactions();
            } else {
                FMTxLogUtils.w(TAG, "sFragmentManager == null in onPreExecute()");
            }        

        }

        @Override
        protected Void doInBackground(Void... arg0) {
            int iStation = mCurrentStation + SEEK_CHANNEL_GAP;
            if (iStation > FMTransmitterStation.HIGHEST_STATION) {
                iStation = FMTransmitterStation.LOWEST_STATION;
            }
            float[] channels = searchChannelsForTx((float) iStation
                    / BASE_NUMBER, SEEK_CHANNEL_DIRECTION, SEEK_CHANNEL_COUNT);
            if (null == channels) {
                FMTxLogUtils.v(TAG, "Seek again from the lowest frequency");
                channels = searchChannelsForTx(
                        (float) FMTransmitterStation.LOWEST_STATION
                                / BASE_NUMBER, SEEK_CHANNEL_DIRECTION,
                        SEEK_CHANNEL_COUNT);
            }

            if (null != channels) {
                FMTxLogUtils.v(TAG, "Seek out channel number: "
                        + channels.length);
                int iFrq = (int) (channels[0] * BASE_NUMBER);
                FMTxLogUtils.v(TAG, "Seek out channel: " + iFrq);
                if (turnToFrequency((float) iFrq / BASE_NUMBER)) {
                    mCurrentStation = iFrq;
                    // Save the current station frequency into data base.
                    FMTransmitterStation.setCurrentStation(mContext,
                            mCurrentStation);
                }
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            // Close progress dialog.            
            FMTxLogUtils.d(TAG, "onPostExecute() called!");
            if (sFragmentManager != null) {
                FMTxLogUtils.d(TAG, "sFragmentManager != null in onPostExecute()");
                DialogFragment prefragdialog = (DialogFragment) sFragmentManager.findFragmentByTag(
                        FMTANSMITTERACTIVITY_FRAGMENT_TAG);
                if (prefragdialog != null) {
                    prefragdialog.dismissAllowingStateLoss();
                    
                }
            } else {
                FMTxLogUtils.w(TAG, "sFragmentManager == null in onPostExecute()");
            }        
            
            if (mIsDestroying) {
                FMTxLogUtils.w(TAG, "Warning: app is being destroyed.");
                return;
            }
            long endSeekTime = 0;
            mTextStationValue.setText(formatStation(mCurrentStation));
            endSeekTime = System.currentTimeMillis();
            FMTxLogUtils.i(TAG, "[Performance test][FMTransmitter] Test FM Tx total seek time end [" + endSeekTime + "]");
            mIsSeeking = false;
        }

    }

}
