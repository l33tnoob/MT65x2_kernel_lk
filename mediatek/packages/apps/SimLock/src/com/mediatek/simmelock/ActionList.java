/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.android.simmelock;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.telephony.TelephonyManager;//To find the SIM card Ready State
import android.util.Log;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneBase;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.ITelephony;
import com.mediatek.common.featureoption.FeatureOption;//To find whether the project support Gemini

public class ActionList extends PreferenceActivity {
    public static final int SMLLOCKED = 1;
    public static final int SMLUNLOCKED = 2;
    public static final String LOCKNAME = "LockName";
    public static final String SIMID = "SIMNo";
    public static final String LOCKCATEGORY = "LockCategory";
    public static final String ACTIONNAME = "ActionName";
    private static final int QUERY_ICC_SML_COMPLETE = 120;
    private static final int QUERY_ICC_SML_LOCK_STATE = 100;

    String lockName = null;
    int simNumber = 0;// Gemini: SIM number
    int lockCategory = -1;
    private boolean unlock_enable = true;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.addPreferencesFromResource(R.xml.actionlist);

        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        registerReceiver(mReceiver, intentFilter);

        // get the lock name
        Bundle bundle = this.getIntent().getExtras();
        if (bundle != null) {
            lockCategory = bundle.getInt(LOCKCATEGORY, -1);
        }
        if (lockCategory == -1) {
            finish();
            return;
        }

        // set the title
        lockName = getLockName(lockCategory);
        this.setTitle(lockName);

        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            simNumber = bundle.getInt(SIMID);
        }

        getUnlockEnableState(lockCategory);
        enablePreference(false, false, false, false, false);
    }

    private String getLockName(final int locktype) {
        switch (locktype) {
        case 0:
            return getString(R.string.strLockNameNetwork);
        case 1:
            return getString(R.string.strLockNameNetworkSub);
        case 2:
            return getString(R.string.strLockNameService);
        case 3:
            return getString(R.string.strLockNameCorporate);
        case 4:
            return getString(R.string.strLockNameSIM);
        default:
            return getString(R.string.simmelock_name);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        if (lockCategory == -1) {
            finish();
            return;
        }

        Message callback = Message.obtain(mHandler, QUERY_ICC_SML_COMPLETE);
        queryIccNetworkLock(lockCategory, callback);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mReceiver);
    }

    @Override
    protected void onRestoreInstanceState(Bundle state) {
        super.onRestoreInstanceState(state);
        lockName = state.getString(LOCKNAME);
        lockCategory = state.getInt(LOCKCATEGORY, -1);
        simNumber = state.getInt(SIMID, 0);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        outState.putString(LOCKNAME, lockName);
        outState.putInt(LOCKCATEGORY, lockCategory);
        outState.putInt(SIMID, simNumber);
        super.onSaveInstanceState(outState);
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferencescreen, Preference preference) {
        Bundle bundle = new Bundle();
        bundle.putInt(LOCKCATEGORY, lockCategory);
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            bundle.putInt(SIMID, simNumber);
        }

        if (this.getPreferenceScreen().findPreference("lock") == preference) {
            // To lock
            Log.i("SIMMELOCK", "Action lock");
            Intent intent = new Intent(ActionList.this, LockSetting.class);
            intent.putExtras(bundle);
            startActivity(intent);
            return true;
        } else if (preference.getKey().equals("unlock")) {
            // to unlock
            Log.i("SIMMELOCK", "Action unlock");
            Configuration conf = getResources().getConfiguration();
            String locale = conf.locale.getDisplayName(conf.locale);
            Intent intent = new Intent(ActionList.this, UnlockSetting.class);
            bundle.putString("LOCALNAME", locale);
            intent.putExtras(bundle);
            startActivity(intent);
            return true;
        } else if (this.getPreferenceScreen().findPreference("addlock") == preference) {
            // To add a lock
            Log.i("SIMMELOCK", "Action addlock");
            Intent intent = null;
            switch (lockCategory) {
            case 0:
                intent = new Intent(ActionList.this, NPAddLockSetting.class);
                break;
            case 1:
                intent = new Intent(ActionList.this, NSPAddLockSetting.class);
                break;
            case 2:
                intent = new Intent(ActionList.this, SPAddLockSetting.class);
                break;
            case 3:
                intent = new Intent(ActionList.this, CPAddLockSetting.class);
                break;
            case 4:
                intent = new Intent(ActionList.this, SIMPAddLockSetting.class);
                break;
            default:
                return false;
            }
            intent.putExtras(bundle);
            startActivity(intent);
            return true;
        } else if (this.getPreferenceScreen().findPreference("removelock") == preference) {
            // To remove a lock
            Log.i("SIMMELOCK", "Action removelock");
            Intent intent = new Intent(ActionList.this, RemoveSetting.class);
            intent.putExtras(bundle);
            startActivity(intent);
            return true;
        } else if (this.getPreferenceScreen().findPreference("permanentlyunlock") == preference) {
            // To permanently unlock
            Log.i("SIMMELOCK", "Action permanentlyunlock");
            Intent intent = new Intent(ActionList.this, PermanUnlockSetting.class);
            intent.putExtras(bundle);
            startActivity(intent);
            return true;
        }
        Log.i("SIMMELOCK", "Action null || preference" + preference);
        return false;
    }

    /**
     * get the enable status of every Action
     * 
     */
    // notes: wait for framework's interface

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {

            switch (msg.what) {
            case QUERY_ICC_SML_LOCK_STATE: {
                if (!unlock_enable) {
                    return;
                }
                AsyncResult ar = (AsyncResult) msg.obj;
                Log.i("SIMMELOCK", "ActionList handler QUERY_ICC_SML_LOCK_STATE");
                int[] LockState = (int[]) ar.result;
                if (LockState == null) {
                    ActionList.this.finish();
                } else if (LockState[1] == SMLLOCKED) {
                    Log.i("SIMMELOCK", "unlock_enable = false");
                    unlock_enable = false;
                }
                break;
            }
            case QUERY_ICC_SML_COMPLETE: {
                Log.i("SIMMELOCK", "ActionList handler");
                AsyncResult ar1 = (AsyncResult) msg.obj;

                int[] LockState = (int[]) ar1.result;
                if (LockState == null) {
                    ActionList.this.finish();
                } else if (LockState[2] == 0) {
                    ActionList.this.finish();
                } else if (LockState[1] == 4) {  // Disable
                    enablePreference(false, false, false, false, false);
                } else if (LockState[1] == 2) {  // Not locked
                    if (LockState[4] == 0) {  // Lock number == 0
                        enablePreference(false, false, true, false, true);
                    } else {
                        if (LockState[4] < LockState[5]) {  // Lock number < Max number
                            enablePreference(true, false, true, true, true);
                        } else {  // Lock number == Max number
                            enablePreference(true, false, false, true, true);
                        }
                    }
                } else {  // Locked
                    enablePreference(false, true, false, false, false);
                }
                if (!unlock_enable) {
                    ((PreferenceActivity) (ActionList.this)).getPreferenceScreen().getPreference(1).setEnabled(false);
                }
                break;
            }
            default:
                break;
            }
        }
    };

    private void enablePreference(boolean lock, boolean unlock, boolean add, boolean remove, boolean permanRemove) {
        ((PreferenceActivity) (ActionList.this)).getPreferenceScreen().getPreference(0).setEnabled(lock);
        ((PreferenceActivity) (ActionList.this)).getPreferenceScreen().getPreference(1).setEnabled(unlock);
        ((PreferenceActivity) (ActionList.this)).getPreferenceScreen().getPreference(2).setEnabled(add);
        ((PreferenceActivity) (ActionList.this)).getPreferenceScreen().getPreference(3).setEnabled(remove);
        ((PreferenceActivity) (ActionList.this)).getPreferenceScreen().getPreference(4).setEnabled(permanRemove);
    }

    private void getUnlockEnableState(int category) {
        Log.i("SIMMELOCK", "[getUnlockEnableState] Current lock category is " + category);
        if (category < 0 || category > 4) {
            return;
        }

        for (int tempCate = 0; tempCate < category; tempCate++) {
            Log.i("SIMMELOCK", "[getUnlockEnableState] Queried lock category is " + tempCate + " || unlock_enable is "
                    + unlock_enable);
            if (!unlock_enable) {
                break;
            }
            Message unlockCallBack = Message.obtain(mHandler, QUERY_ICC_SML_LOCK_STATE);
            queryIccNetworkLock(tempCate, unlockCallBack);
        }
    }

    private void queryIccNetworkLock(int lockCategory, Message callback) {
        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
            // if it single SIM, no need to pass the card number
            Phone phone = PhoneFactory.getDefaultPhone();
            if (phone != null) {
                IccCard iccCard = phone.getIccCard();
                if (iccCard != null) {
                    iccCard.QueryIccNetworkLock(lockCategory, 4, null, null, null, null, callback);
                }
            }
        } else {
            GeminiPhone geminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
            if (geminiPhone != null) {
                IccCard iccCard = geminiPhone.getPhonebyId(simNumber).getIccCard();
                if (iccCard != null) {
                    iccCard.QueryIccNetworkLock(lockCategory, 4, null, null, null, null, callback);
                }
            }
        }
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                finish();
            }
        }
    };
}
