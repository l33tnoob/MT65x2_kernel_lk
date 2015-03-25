/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.telephony;

import android.annotation.SdkConstant;
import android.annotation.SdkConstant.SdkConstantType;
import android.content.Context;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;

import com.android.internal.telephony.IPhoneSubInfo;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.ITelephonyRegistry;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.TelephonyProperties;

import com.mediatek.common.telephony.ITelephonyEx;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ITelephonyEx;

import android.os.Message;
import android.telephony.TelephonyManager;
import android.telephony.CellInfo;
import android.telephony.CellLocation;
import android.telephony.NeighboringCellInfo;
import android.telephony.PhoneStateListener;
import android.telephony.Rlog;
import java.util.List;

/// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.telephony.IOnlyOwnerSimSupport;
/// @}


/**
 * Provides access to information about the telephony services on
 * the device, especially for multiple SIM cards device.
 *
 * Applications can use the methods in this class to
 * determine telephony services and states, as well as to access some
 * types of subscriber information. Applications can also register
 * a listener to receive notification of telephony state changes.
 * 
 * Note that access to some telephony information is
 * permission-protected. Your application cannot access the protected
 * information unless it has the appropriate permissions declared in
 * its manifest file. Where permissions apply, they are noted in the
 * the methods through which you access the protected information.
 */
public class TelephonyManagerEx {
    private static final String TAG = "TelephonyManagerEx";
    
    private Context mContext = null;
    private ITelephonyRegistry mRegistry;

    
    /* Add for  Phone2 */
    private ITelephonyRegistry mRegistry2; 
    private ITelephonyRegistry mRegistry3; 
    private ITelephonyRegistry mRegistry4; 	
    private static int defaultSimId = PhoneConstants.GEMINI_SIM_1;
    /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT 
    private IOnlyOwnerSimSupport mOnlyOwnerSimSupport = null;

	/**
	 * Construction function for TelephonyManager
	 * @param context a context
	 */
    public TelephonyManagerEx(Context context) {
    	
    	Rlog.d( TAG,"getSubscriberInfo");
        mContext = context;
        mRegistry = ITelephonyRegistry.Stub.asInterface(ServiceManager.getService(
                    "telephony.registry"));

        /* Add for Gemini Phone2 */
        mRegistry2 = ITelephonyRegistry.Stub.asInterface(ServiceManager.getService(
                    "telephony.registry2"));
		
        if(PhoneConstants.GEMINI_SIM_NUM >=3){
            mRegistry3 = ITelephonyRegistry.Stub.asInterface(ServiceManager.getService(
                    "telephony.registry3"));
        }
        if(PhoneConstants.GEMINI_SIM_NUM >=4){		
            mRegistry4 = ITelephonyRegistry.Stub.asInterface(ServiceManager.getService(
                    "telephony.registry4"));		
        }			
        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        mOnlyOwnerSimSupport = MediatekClassFactory.createInstance(IOnlyOwnerSimSupport.class);
        if (mOnlyOwnerSimSupport != null) {
            String actualClassName = mOnlyOwnerSimSupport.getClass().getName();
            Rlog.d(TAG, "initial mOnlyOwnerSimSupport done, actual class name is " + actualClassName);
        } else {
            Rlog.e(TAG, "FAIL! intial mOnlyOwnerSimSupport");
        }
        /// @}
    }

    /*  Construction function for TelephonyManager */
    private TelephonyManagerEx() {

        mRegistry = ITelephonyRegistry.Stub.asInterface(ServiceManager.getService(
                   "telephony.registry"));

        /* Add for Gemini Phone2 */
        mRegistry2 = ITelephonyRegistry.Stub.asInterface(ServiceManager.getService(
                    "telephony.registry2"));
		
        if(PhoneConstants.GEMINI_SIM_NUM >=3){
            mRegistry3 = ITelephonyRegistry.Stub.asInterface(ServiceManager.getService(
                    "telephony.registry3"));
        }
        if(PhoneConstants.GEMINI_SIM_NUM >=4){		
            mRegistry4 = ITelephonyRegistry.Stub.asInterface(ServiceManager.getService(
                    "telephony.registry4"));		
        }			
        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        mOnlyOwnerSimSupport = MediatekClassFactory.createInstance(IOnlyOwnerSimSupport.class);
        if (mOnlyOwnerSimSupport != null) {
            String actualClassName = mOnlyOwnerSimSupport.getClass().getName();
            Rlog.d(TAG, "initial mOnlyOwnerSimSupport done, actual class name is " + actualClassName);
        } else {
            Rlog.e(TAG, "FAIL! intial mOnlyOwnerSimSupport");
        }
        /// @}
    }

    private  static TelephonyManagerEx sInstance = new TelephonyManagerEx();
    
    /** @hide
     *  @return return the static instance of TelephonyManagerEx
     */
    public static TelephonyManagerEx getDefault() {
        return sInstance;
    }
   
    /**
     * Retruns subscriber information.
     * @param simId sim card id
     * @return Get IPhoneSubInfo service
     */
    private IPhoneSubInfo getSubscriberInfo(int simId) {
    	Rlog.d( TAG,"getSubscriberInfo simId="+simId);
        // get it each time because that process crashes a lot
        if (PhoneConstants.GEMINI_SIM_4 == simId) {
            return IPhoneSubInfo.Stub.asInterface(ServiceManager.getService("iphonesubinfo4"));
        } else if (PhoneConstants.GEMINI_SIM_3 == simId){
            return IPhoneSubInfo.Stub.asInterface(ServiceManager.getService("iphonesubinfo3"));
        } else if (PhoneConstants.GEMINI_SIM_2 == simId){
            return IPhoneSubInfo.Stub.asInterface(ServiceManager.getService("iphonesubinfo2"));
        } else {
            return IPhoneSubInfo.Stub.asInterface(ServiceManager.getService("iphonesubinfo"));
        }
    }

    /**
     * @param simId Indicates which SIM(slot) to query
     * @return The software version number for the device
     * @hide
     * @internal
     */
    public String getDeviceSoftwareVersion(int simId) {
        try {
            return getSubscriberInfo(simId).getDeviceSvn();
        } catch (RemoteException ex) {
            return null;
        } catch (NullPointerException ex) {
            return null;
        }
    }

    //
    //
    // Device Info
    //
    //

    /**
     * Returns the unique device identifier e.g. IMEI for GSM phones. MEID or ESN for CDMA phones.
     * For GSM phone with multiple SIM support , there is IMEI for each SIM.
     * Required Permission:
     *  android.Manifest.permission READ_PHONE_STATE READ_PHONE_STATE
     *   
     * @param simId Indicates which SIM(slot) to query      
     * @return Unique device ID. For GSM phones,a string of IMEI
     * returns null if the device ID is not available.
     * @internal
     */
    public String getDeviceId(int simId) {
    	Rlog.d( TAG,"getDeviceId simId="+simId);
        try {
            return getSubscriberInfo(simId).getDeviceId();
        } catch (RemoteException ex) {
            ex.printStackTrace();        
            return null;
        } catch (NullPointerException ex) {
            ex.printStackTrace();        
            return null;
        }
    }
  
    /**
     * Returns the current cell location of the device.
     * <p>
     * Required Permission:
     *  android.Manifest.permission#ACCESS_COARSE_LOCATION ACCESS_COARSE_LOCATION or
     *  android.Manifest.permission#ACCESS_COARSE_LOCATION ACCESS_FINE_LOCATION.
     *
     * @param simId  Indicates which SIM to query. 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2        
     * @return current cell location of the device. A CellLocation object
     * returns null if the current location is not available.
     *
     */
    public CellLocation getCellLocation(int simId) {
    	Rlog.d( TAG,"getCellLocation simId="+simId);
        try {
            Bundle bundle = getITelephonyEx().getCellLocation(simId);
            return CellLocation.newFromBundle(bundle);
        } catch (RemoteException ex) {
            ex.printStackTrace();        
            return null;
        } catch (NullPointerException ex) {
            ex.printStackTrace();        
            return null;
        }
    }
    
    /**
     * Returns the neighboring cell information of the device. The getAllCellInfo is preferred
     * and use this only if getAllCellInfo return nulls or an empty list.
     *<p>
     * In the future this call will be deprecated.
     *<p>
     * @return List of NeighboringCellInfo or null if info unavailable.
     * 
     * Required Permission:
     *   "android.Manifest.permission#ACCESS_COARSE_UPDATES"
     * 
     * @param simId  Indicates which SIM to query. 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2     
     * @return list of NeighboringCellInfo, java.util.List<android.telephony.NeighboringCellInfo>, or null if info is unavailable
     *
     */
    public List<NeighboringCellInfo> getNeighboringCellInfo(int simId) {
    	Rlog.d( TAG,"getNeighboringCellInfo simId="+simId);
        try {
            return getITelephonyEx().getNeighboringCellInfo(mContext.getBasePackageName(), simId);
        } catch (RemoteException ex) {
            ex.printStackTrace();        
            return null;
        } catch (NullPointerException ex) {
            ex.printStackTrace();        
            return null;
        }
    }

    /**
     * Returns a constant indicating the device phone type.  This
     * indicates the type of radio used to transmit voice calls.
     * 
     * @param simId Indicates which SIM(slot) to query          
     * @return  a constant indicating the device phone type
     *
     * @see #PHONE_TYPE_NONE
     * @see #PHONE_TYPE_GSM
     * @see #PHONE_TYPE_CDMA    
     */
    public int getPhoneType(int simId) {
    	Rlog.d( TAG,"getPhoneType simId="+simId);
        try{
            ITelephony telephony = getITelephony();
            if (telephony != null) {
                return telephony.getActivePhoneTypeGemini(simId);
            } else {
                // This can happen when the ITelephony interface is not up yet.
                return getPhoneTypeFromProperty();
            }
        } catch (RemoteException ex) {
            // This shouldn't happen in the normal case, as a backup we
            // read from the system property.
            return getPhoneTypeFromProperty();
        } catch (NullPointerException ex) {
            // This shouldn't happen in the normal case, as a backup we
            // read from the system property.
            return getPhoneTypeFromProperty();
        }
    }

    //
    // Current Network
    //

    /**
     * Returns the alphabetic name of current registered operator.
     * <p>
     * Availability: Only when the user is registered to a network. Result may be
     * unreliable on CDMA networks (use getPhoneType(int simId)) to determine if
     * it is on a CDMA network).
     * 
     * @param simId  Indicates which SIM to query. 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2         
     * @return alphabetic name of the current registered operator, e.g. "Vodafone"
     */
    public String getNetworkOperatorName(int simId) {
        try{
            ITelephony telephony = getITelephony();
            if (telephony != null) {
                return telephony.getNetworkOperatorNameGemini(simId);
            } else {
                // This can happen when the ITelephony interface is not up yet.
                return "";
            }
        } catch(RemoteException ex) {
            ex.printStackTrace();        
            return "";
        } catch (NullPointerException ex) {
            ex.printStackTrace();        
            return "";
        }				
    }

    /**
     * Returns the numeric name (MCC+MNC) of the current registered operator.
     * <p>
     * Availability: Only when the user is registered to a network. Result may be
     * unreliable on CDMA networks (use getPhoneType(int simId)) to determine if
     * it is on a CDMA network).
     * 
     * @param simId  Indicates which SIM to query. 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2         
     * @return numeric name (MCC+MNC) of current registered operator, e.g. "46000".
     */
    public String getNetworkOperator(int simId) {
        try{
            ITelephony telephony = getITelephony();
            if (telephony != null) {
                return telephony.getNetworkOperatorGemini(simId);
            } else {
                // This can happen when the ITelephony interface is not up yet.
                return "";
            }
        } catch(RemoteException ex) {
            ex.printStackTrace();        
            return "";
        } catch (NullPointerException ex) {
            ex.printStackTrace();        
            return "";
        }				    
    }

    /**
     * Indicates whether the device is considered roaming on the current  network, for GSM purposes.
     * <p>
     * Availability: Only when the user is registered to a network.
     *
     * @param simId  Indicates which SIM to query. 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2                  
     * @return Returns True if the device is considered roaming on the current network; otherwise false.
     * 
     */
    public boolean isNetworkRoaming(int simId) {
        try{
            ITelephony telephony = getITelephony();
            if (telephony != null) {
                return telephony.isNetworkRoamingGemini(simId);
            } else {
                // This can happen when the ITelephony interface is not up yet.
                return false;
            }
        } catch(RemoteException ex) {
            ex.printStackTrace();        
            return false;
        } catch (NullPointerException ex) {
            ex.printStackTrace();        
            return false;
        }		
    }

    /**
     * Returns the ISO country code of the current registered operator's MCC(Mobile Country Code).
     * <p>
     * Availability: Only when the user is registered to a network. Result may be
     * unreliable on CDMA networks (use getPhoneType(int simId)) to determine if
     * it is on a CDMA network).
     * 
     * @param simId  Indicates which SIM to query. 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2       
     * @return ISO country code equivilent of the current registered
     * operator's MCC (Mobile Country Code), e.g. "en","fr"
     */
    public String getNetworkCountryIso(int simId) {
        try{
            ITelephony telephony = getITelephony();
            if (telephony != null) {
                return telephony.getNetworkCountryIsoGemini(simId);
            } else {
                // This can happen when the ITelephony interface is not up yet.
                return "";
            }
        } catch(RemoteException ex) {
            ex.printStackTrace();        
            return "";
        } catch (NullPointerException ex) {
            ex.printStackTrace();        
            return "";
        }			    		
    }

    /**
     * Returns a constant indicating the radio technology (network type)
     * currently used on the device for data transmission.
     * <p>
     * @param simId  Indicates which SIM to query. 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2               
     * @return constant indicating the radio technology (network type)
     * currently used on the device. Constant may be one of the following items.
     * <p>
     * android.telephony.TelephonyManager.NETWORK_TYPE_UNKNOWN
     * <p>
     * android.telephony.TelephonyManager.NETWORK_TYPE_GPRS
     * <p>
     * android.telephony.TelephonyManager.NETWORK_TYPE_EDGE
     * <p>
     * android.telephony.TelephonyManager.NETWORK_TYPE_UMTS
     * <p>
     * android.telephony.TelephonyManager.NETWORK_TYPE_HSDPA
     * <p>
     * android.telephony.TelephonyManager.NETWORK_TYPE_HSUPA
     * <p>
     * android.telephony.TelephonyManager.NETWORK_TYPE_HSPA
     * <p>
     * android.telephony.TelephonyManager.NETWORK_TYPE_CDMA
     * <p>
     * android.telephony.TelephonyManager.NETWORK_TYPE_EVDO_0
     * <p>
     * android.telephony.TelephonyManager.NETWORK_TYPE_EVDO_A
     * <p>
     * android.telephony.TelephonyManager.NETWORK_TYPE_EVDO_B
     * <p>
     * android.telephony.TelephonyManager.NETWORK_TYPE_1xRTT
     * <p>
     * android.telephony.TelephonyManager.NETWORK_TYPE_IDEN
     * <p>
     * android.telephony.TelephonyManager.NETWORK_TYPE_LTE
     * <p>
     * android.telephony.TelephonyManager.NETWORK_TYPE_EHRPD
     * <p>
     * android.telephony.TelephonyManager.NETWORK_TYPE_HSPAP     
     */
    public int getNetworkType(int simId) {
    	Rlog.d( TAG,"getNetworkType simId="+simId); 
        try{
            ITelephony telephony = getITelephony();
            if (telephony != null) {
                return telephony.getNetworkTypeGemini(simId);
            } else {
                // This can happen when the ITelephony interface is not up yet.
                return android.telephony.TelephonyManager.NETWORK_TYPE_UNKNOWN;
            }
        } catch(RemoteException ex) {
            // This shouldn't happen in the normal case
            ex.printStackTrace();            
            return android.telephony.TelephonyManager.NETWORK_TYPE_UNKNOWN;
        } catch (NullPointerException ex) {
            // This could happen before phone restarts due to crashing
            ex.printStackTrace();            
            return android.telephony.TelephonyManager.NETWORK_TYPE_UNKNOWN;
        }
    }


    //
    //
    // SIM Card
    //
    //

    /**
     * Gets true if a ICC card is present
     * <p>
     * @param simId  Indicates which SIM to query. 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2   
     * <p>
     * @return       Returns True if a ICC card is present.
     */
    public boolean hasIccCard(int simId) {
        Rlog.d( TAG,"hasIccCard simId="+simId);
        try {
            return getITelephonyEx().hasIccCard(simId);
        } catch (RemoteException ex) {
            // Assume no ICC card if remote exception which shouldn't happen
            ex.printStackTrace();            
            return false;
        } catch (NullPointerException ex) {
            // This could happen before phone restarts due to crashing
            ex.printStackTrace();            
            return false;
        }
    }

   /**
     * Get Icc Card Type
     * @param simId which sim(slot) to query
     * @return "SIM" for SIM card or "USIM" for USIM card.
     * @hide
     * @internal
     */
    public String getIccCardType(int simId) {
        Rlog.d( TAG,"getIccCardType simId="+simId);
        try {
            return getITelephonyEx().getIccCardType(simId);
        } catch (RemoteException ex) {
            ex.printStackTrace();        
            return null;
        } catch (NullPointerException ex) {
            // This could happen before phone restarts due to crashing
            ex.printStackTrace();        
            return null;
        }
    }

    /**
     * Get Icc Card is a test card or not.
     * @param simId Indicate which sim(slot) to query
     * @return ture if the ICC card is a test card.
     * @hide
     * @internal
     */
    public boolean isTestIccCard(int simId) {
        Rlog.d( TAG,"isTestIccCard simId="+simId);
        try {
            return getITelephonyEx().isTestIccCard(simId);
        } catch (RemoteException ex) {
            ex.printStackTrace();        
            return false;
        } catch (NullPointerException ex) {
            // This could happen before phone restarts due to crashing
            ex.printStackTrace();        
            return false;
        }
    }

    /**
     * Gets a constant indicating the state of the device SIM card.
     * <p>
     * @param simId  Indicates which SIM to query. 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2   
     * <p>
     * @return       Constant indicating the state of the device SIM card.
     * Constant may be one of the following items.
     * <p>
     * android.telephony.TelephonyManager.SIM_STATE_UNKNOWN
     * <p>
     * android.telephony.TelephonyManager.SIM_STATE_ABSENT
     * <p>
     * android.telephony.TelephonyManager.SIM_STATE_PIN_REQUIRED
     * <p>
     * android.telephony.TelephonyManager.SIM_STATE_PUK_REQUIRED
     * <p>
     * android.telephony.TelephonyManager.SIM_STATE_NETWORK_LOCKED
     * <p>
     * android.telephony.TelephonyManager.SIM_STATE_READY
     */
    public int getSimState(int simId) {
        Rlog.d( TAG,"getSimState simId="+simId);        
        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            Rlog.d(TAG, "getSimState return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return android.telephony.TelephonyManager.SIM_STATE_UNKNOWN; 
        }
        /// @}

        try {
            return getITelephony().getSimState(simId);
        } catch (RemoteException ex) {
            // This shouldn't happen in the normal case
            ex.printStackTrace();            
            return android.telephony.TelephonyManager.NETWORK_TYPE_UNKNOWN;
        } catch (NullPointerException ex) {
            // This could happen before phone restarts due to crashing
            ex.printStackTrace();            
            return android.telephony.TelephonyManager.NETWORK_TYPE_UNKNOWN;
        }
    }

    /**
     * Gets the MCC+MNC (mobile country code + mobile network code) of the provider of the SIM. 5 or 6 decimal digits. 
     * <p>
     * Availability: The result of calling getSimState() must be android.telephony.TelephonyManager.SIM_STATE_READY.
     * <p>
     * @param simId  Indicates which SIM to query. 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2  
     * <p>
     * @return       MCC+MNC (mobile country code + mobile network code) of the provider of the SIM. 5 or 6 decimal digits.
     */
    public String getSimOperator(int simId) {
        Rlog.d( TAG,"getSimOperator simId="+simId);        
        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            Rlog.d(TAG, "getSimOperator return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return "";
        }
        /// @}

        try {
            return getITelephony().getSimOperator(simId);
        } catch (RemoteException ex) {
            // This shouldn't happen in the normal case
            ex.printStackTrace();            
            return null;
        } catch (NullPointerException ex) {
            // This could happen before phone restarts due to crashing
            ex.printStackTrace();            
            return null;
        }
    }

    /**
     * Gets the Service Provider Name (SPN).
     * <p>
     * Availability: The result of calling getSimState() must be android.telephony.TelephonyManager.SIM_STATE_READY.
     * <p>
     * @param simId  Indicates which SIM to query. 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2 
     * <p>
     * @return       Service Provider Name (SPN)
     */
    public String getSimOperatorName(int simId) {
        Rlog.d( TAG,"getSimOperatorName simId="+simId);
        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            Rlog.d(TAG, "getSimOperatorName return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return "";
        }
        /// @}

        try {
            return getITelephony().getSimOperatorName(simId);
        } catch (RemoteException ex) {
            // This shouldn't happen in the normal case
            ex.printStackTrace();            
            return null;
        } catch (NullPointerException ex) {
            // This could happen before phone restarts due to crashing
            ex.printStackTrace();            
            return null;
        }
    }

    /**
     * Gets the ISO country code equivalent for the SIM provider's country code.
     * <p>
     * @param simId  Indicates which SIM to query. 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2     
     * <p>
     * @return       Gets the ISO country code equivalent for the SIM provider's country code.
     */
    public String getSimCountryIso(int simId) {
        Rlog.d( TAG,"getSimCountryIso simId="+simId);
        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            Rlog.d(TAG, "getSimCountryIso return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return "";
        }
        /// @}

        try {
            return getITelephony().getSimCountryIso(simId);
        } catch (RemoteException ex) {
            // This shouldn't happen in the normal case
            ex.printStackTrace();            
            return null;
        } catch (NullPointerException ex) {
            // This could happen before phone restarts due to crashing
            ex.printStackTrace();            
            return null;
        }
    }
    

    /**
     * Gets the serial number of the SIM, if applicable
     * <p>
     * Required Permission:
     *   "android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE"
     * <p>
     * @param simId  Indicates which SIM to query. 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2   
     * <p>
     * @return       serial number of the SIM, if applicable. Null is returned if it is unavailable.
     */
    public String getSimSerialNumber(int simId) {
    	Rlog.d( TAG,"getSimSerialNumber simId="+simId);
        /// M: For MTK multiuser in 3gdatasms:MTK_ONLY_OWNER_SIM_SUPPORT @{ 
        if(!mOnlyOwnerSimSupport.isCurrentUserOwner()){
            Rlog.d(TAG, "getSimSerialNumber return: 3gdatasms MTK_ONLY_OWNER_SIM_SUPPORT ");
            return "";
        }
        /// @}

        try {
            return getSubscriberInfo(simId).getIccSerialNumber();
        } catch (RemoteException ex) {
            ex.printStackTrace();        
            return null;
        } catch (NullPointerException ex) {
            // This could happen before phone restarts due to crashing
            ex.printStackTrace();            
            return null;
        }
    }

    //
    //
    // Subscriber Info
    //
    //

    /**
     * Gets the unique subscriber ID, for example, the IMSI for a GSM phone.
     * <p>
     * Required Permission:
     *   "android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE"
     * <p>
     * @param simId  Indicates which SIM to query. 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2  
     * <p>
     * @return       unique subscriber ID, for example, the IMSI for a GSM phone. Null is returned if it is unavailable.
     */
    public String getSubscriberId(int simId) {
    	Rlog.d( TAG,"getSubscriberId simId="+simId);
        try {
            return getSubscriberInfo(simId).getSubscriberId();
        } catch (RemoteException ex) {
            ex.printStackTrace();        
            return null;
        } catch (NullPointerException ex) {
            ex.printStackTrace();        
            // This could happen before phone restarts due to crashing
            return null;
        }
    }

    /**
     * Gets the phone number string for line 1, for example, the MSISDN for a GSM phone
     * <p>
     * Required Permission:
     *   "android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE"
     * <p>
     * @param simId  Indicates which SIM to quer 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2
     * <p>
     * @return       Phone number string for line 1, for example, the MSISDN for a GSM phone. Returns null if it is unavailable.
     */
    public String getLine1Number(int simId) {
    	Rlog.d( TAG,"getLine1Number simId="+simId);
        try {
            return getSubscriberInfo(simId).getLine1Number();
        } catch (RemoteException ex) {
            ex.printStackTrace();        
            return null;
        } catch (NullPointerException ex) {
            // This could happen before phone restarts due to crashing
            ex.printStackTrace();            
            return null;
        }
    }
  

    /**
     * Gets the voice mail number.
     * <p>
     * Required Permission:
     *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
     * <p>
     * @param simId  Indicates which SIM (slot) to query
     * <p>
     * @return       Voice mail number. Null is returned if it is unavailable.
     */
    public String getVoiceMailNumber(int simId) {
    	Rlog.d( TAG,"getVoiceMailNumber simId="+simId);
        try {
            return getSubscriberInfo(simId).getVoiceMailNumber();
        } catch (RemoteException ex) {
            return null;
        } catch (NullPointerException ex) {
            // This could happen before phone restarts due to crashing
            return null;
        }
    }

    /**
     * Retrieves the alphabetic identifier associated with the voice mail number.
     * <p>
     * Required Permission:
     *   "android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE"
     * <p>
     * @param simId  Indicates which SIM to query. 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2   
     * <p>
     * @return       Alphabetic identifier associated with the voice mail number
     */
    public String getVoiceMailAlphaTag(int simId) {
    	Rlog.d( TAG,"getVoiceMailAlphaTag simId="+simId);
        try {
            return getSubscriberInfo(simId).getVoiceMailAlphaTag();
        } catch (RemoteException ex) {
            ex.printStackTrace();        
            return null;
        } catch (NullPointerException ex) {
            // This could happen before phone restarts due to crashing
            ex.printStackTrace();            
            return null;
        }
    }

      /**
     * Gets the current call state according to the specific SIM ID.
     * The call state can be one of the following states:
     * 1) android.telephony.TelephonyManager.CALL_STATE_IDLE;
     * 2) android.telephony.TelephonyManager.CALL_STATE_RINGING;
     * 3) android.telephony.TelephonyManager.CALL_STATE_OFFHOOK
     * @param  simId SIM ID for getting call state. 
     *         Value of simId:
     *         0 for SIM1;
     *         1 for SIM2
     * @return Constant indicating the call state (cellular) on the device.
     * @internal
     */
    public int getCallState(int simId) {
    	Rlog.d( TAG,"getCallState simId="+simId);
        try {
            return getITelephonyEx().getCallState(simId);
        } catch (RemoteException ex) {
            // the phone process is restarting.
            ex.printStackTrace();            
            return android.telephony.TelephonyManager.CALL_STATE_IDLE;
        } catch (NullPointerException ex) {
          // the phone process is restarting.
            ex.printStackTrace();          
          return android.telephony.TelephonyManager.CALL_STATE_IDLE;
      }
    }

    /**
     * Returns a constant indicating the type of activity on a data connection
     * (cellular).
     *
     * The data activity can be one of the following:
     * 1) DATA_ACTIVITY_NONE;
     * 2) DATA_ACTIVITY_IN;
     * 3) DATA_ACTIVITY_OUT;
     * 4) DATA_ACTIVITY_INOUT;
     * 5) DATA_ACTIVITY_DORMANT
     *
     * @param simId Indicates which SIM(slot) to query   
     * @return Constant indicating the type of activity on specific SIM's data connection
     * (cellular).
     * @internal
     */
    public int getDataActivity(int simId) {
    	Rlog.d( TAG,"getDataActivity simId="+simId);
        try {
            return getITelephonyEx().getDataActivity(simId);
        } catch (RemoteException ex) {
            // the phone process is restarting.
            ex.printStackTrace();            
            return android.telephony.TelephonyManager.DATA_ACTIVITY_NONE;
        } catch (NullPointerException ex) {
          // the phone process is restarting.
            ex.printStackTrace();          
          return android.telephony.TelephonyManager.DATA_ACTIVITY_NONE;
      }
    }

    /**
     * Returns a constant indicating the specific SIM's data connection state
     * (cellular).
     *
     * The data connection state can be one of the following states:
     * 1) DATA_DISCONNECTED;
     * 2) DATA_CONNECTING;
     * 3) DATA_CONNECTED;
     * 4) DATA_SUSPENDED
     *
     * @param simId Indicates which SIM(slot) to query
     * @return Constant indicating specific SIM's data connection state
     * (cellular).
     * @internal
     */    
    public int getDataState(int simId) {
    	Rlog.d( TAG,"getDataState simId="+simId);
        try {
            return getITelephonyEx().getDataState(simId);
        } catch (RemoteException ex) {
            // the phone process is restarting.
            ex.printStackTrace();            
            return android.telephony.TelephonyManager.DATA_DISCONNECTED;
        } catch (NullPointerException ex) {
            ex.printStackTrace();        
            return android.telephony.TelephonyManager.DATA_DISCONNECTED;
        }
    }


    //
    //
    // PhoneStateListener
    //
    //


    /**
     * 
     * Registers a listener object to receive notification of changes
     * in specified telephony states.
     * 
     * To register a listener, pass a PhoneStateListener
     * and specify at least one telephony state of interest in
     * the events argument.
     *
     * At registration, and when a specified telephony state
     * changes, the telephony manager invokes the appropriate
     * callback method on the listener object and passes the
     * current (udpated) values.
     * 
     * To unregister a listener, pass the listener object and set the
     * events argument to PhoneStateListener LISTEN_NONE LISTEN_NONE.
     *
     * @param listener  the android.telephony.PhoneStateListener object
     *                  to register or unregister
     * @param events  the telephony state(s) of interest to the listener,
     *               as a bitwise-OR combination of PhoneStateListener
     *               LISTEN_ flags.
     * @param simId  Indicates which SIM to regisrer or unregister. 
     *               Value of simId:
     *                 0 for SIM1
     *                 1 for SIM2
     */
    public void listen(PhoneStateListener listener, int events, int simId) {
    	Rlog.d( TAG,"listen simId="+simId+",events="+events);
        String pkgForDebug = mContext != null ? mContext.getPackageName() : "<unknown>";
        try {
            Boolean notifyNow = (getITelephony() != null);
            if (PhoneConstants.GEMINI_SIM_4 == simId) {
                mRegistry4.listen(pkgForDebug, listener.getCallback(), events, notifyNow);
            } else if(PhoneConstants.GEMINI_SIM_3 == simId){
                mRegistry3.listen(pkgForDebug, listener.getCallback(), events, notifyNow);
            } else if(PhoneConstants.GEMINI_SIM_2 == simId){
                mRegistry2.listen(pkgForDebug, listener.getCallback(), events, notifyNow);
            } else {
                mRegistry.listen(pkgForDebug, listener.getCallback(), events, notifyNow);
            }
        } catch (RemoteException ex) {
            // system process dead
            ex.printStackTrace();            
        } catch (NullPointerException ex) {
            // system process dead
            ex.printStackTrace();            
        }
    }


    /**
     * Returns all observed cell information from all radios on the
     * device including the primary and neighboring cells. This does
     * not cause or change the rate of PhoneStateListner#onCellInfoChanged.
     *<p>
     * The list can include one or more of {@link android.telephony.CellInfoGsm CellInfoGsm},
     * {@link android.telephony.CellInfoCdma CellInfoCdma},
     * {@link android.telephony.CellInfoLte CellInfoLte} and
     * {@link android.telephony.CellInfoWcdma CellInfoCdma} in any combination.
     * Specifically on devices with multiple radios it is typical to see instances of
     * one or more of any these in the list. In addition 0, 1 or more CellInfo
     * objects may return isRegistered() true.
     *<p>
     * This is preferred over using getCellLocation although for older
     * devices this may return null in which case getCellLocation should
     * be called.
     *<p>
     * @return List of CellInfo or null if info unavailable.
     * @hide     
     *
     * <p>Requires Permission: {@link android.Manifest.permission#ACCESS_COARSE_LOCATION}
     */
    public List<CellInfo> getAllCellInfo(int simId) {
        try {
            return getITelephonyEx().getAllCellInfo(simId);
        } catch (RemoteException ex) {
            return null;
        } catch (NullPointerException ex) {
            return null;
        }
    }


    /**
     * Sets the minimum time in milli-seconds between {@link PhoneStateListener#onCellInfoChanged
     * PhoneStateListener.onCellInfoChanged} will be invoked.
     *<p>
     * The default, 0, means invoke onCellInfoChanged when any of the reported
     * information changes. Setting the value to INT_MAX(0x7fffffff) means never issue
     * A onCellInfoChanged.
     *<p>
     * @param rateInMillis the rate
     *
     * @hide
     */
    public void setCellInfoListRate(int rateInMillis,int simId) {
        try {
            getITelephonyEx().setCellInfoListRate(rateInMillis,simId);
        } catch (RemoteException ex) {
        } catch (NullPointerException ex) {
        }
    }

    private ITelephony getITelephony() {
        return ITelephony.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICE));
    }
    
    private ITelephonyEx getITelephonyEx() {
        return ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
    }
    
    /* Get current active phone type by system property, GsmPhone or CdmaPhone */
    private int getPhoneTypeFromProperty() {
        int type =
            SystemProperties.getInt(TelephonyProperties.CURRENT_ACTIVE_PHONE,
                    getPhoneTypeFromNetworkType());
        return type;
    }
    
    /* Get phone type by network type, GsmPhone or CdmaPhone */
    private int getPhoneTypeFromNetworkType() {
        // When the system property CURRENT_ACTIVE_PHONE, has not been set,
        // use the system property for default network type.
        // This is a fail safe, and can only happen at first boot.
        int mode = SystemProperties.getInt("ro.telephony.default_network", -1);
        if (mode == -1)
            return android.telephony.TelephonyManager.PHONE_TYPE_NONE;
        return PhoneFactory.getPhoneType(mode);
    }


    /**
     * Get service center address
     * 
     * @param simId SIM ID
     * @return Current service center address
     * @hide
     * @internal
     */
    public String getScAddress(int slotId) {
        try {
            return getITelephonyEx().getScAddressGemini(slotId);
        } catch (RemoteException e1) {
            e1.printStackTrace();	   
            return null;
        } catch (NullPointerException e2) {
            e2.printStackTrace();	   
            return null;
        }
    }

    /**
     * Set service center address
     * 
     * @param address Address to be set
     * @param simId SIM ID
     * @return True for success, false for failure
     * @hide
     * @internal     
     */
   public boolean setScAddress(String address, int slotId) {
	   try {
		   getITelephonyEx().setScAddressGemini(address, slotId);
		   return true;
	   } catch(RemoteException e1) {
            e1.printStackTrace();	   
		   return false;
	   } catch(NullPointerException e2) {
            e2.printStackTrace();	   
		   return false;
	   }
   }

   /**
    * Returns the network service state. 
    * <p>     
     * @return service state.    
    * @hide
     * @deprecated -use function in Itelephony(PhoneInterfaceManager) instead 
    */ 
    @Deprecated	public Bundle getServiceState(){
       try {
            ITelephony telephony = getITelephony();
            if (telephony != null) {
                return telephony.getServiceState();
            } else {
                // This can happen when the ITelephony interface is not up yet.
                return null;
            }          
       } catch (RemoteException ex) {
           // the phone process is restarting.
            ex.printStackTrace();           
           return null;
       } catch (NullPointerException ex) {
            ex.printStackTrace();       
           return null;
       }
   }

   /**
    * Returns the network service state.
    * <p>     
     * @param simId Indicate which sim(slot) to query
    * @return service state
    * @hide
     * @deprecated - use function in Itelephony(PhoneInterfaceManager) instead     
    */ 
    @Deprecated	public Bundle getServiceState(int simId){
       try {
            ITelephonyEx telephonyEx = getITelephonyEx();
            if (telephonyEx != null) {
                return telephonyEx.getServiceState(simId);
            } else {
                // This can happen when the ITelephony interface is not up yet.
                return null;
            }          
       } catch (RemoteException ex) {
           // the phone process is restarting.
            ex.printStackTrace();           
           return null;
       } catch (NullPointerException ex) {
           ex.printStackTrace();       
           return null;
       }   
   }

    /**
     * Gemini
     * Returns the voice mail count. Return 0 if unavailable.
     * <p>
     * Requires Permission:
     *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
     * @hide
     */
    public int getVoiceMessageCount(int simId) {
        try {
            return getITelephony().getVoiceMessageCountGemini(simId);
        } catch (RemoteException ex) {
           ex.printStackTrace();       
            return 0;
        } catch (NullPointerException ex) {
            // This could happen before phone restarts due to crashing
           ex.printStackTrace();       
            return 0;
        }
    }

    /**
     * Gemini
     * Returns the alphabetic identifier associated with the line 1 number.
     * Return null if it is unavailable.
     * <p>
     * Requires Permission:
     *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
     * @hide
     * @internal
     */
    public String getLine1AlphaTag(int simId) {
        try {
            return getSubscriberInfo(simId).getLine1AlphaTag();
        } catch (RemoteException ex) {
           ex.printStackTrace();       
            return null;
        } catch (NullPointerException ex) {
            // This could happen before phone restarts due to crashing
           ex.printStackTrace();       
            return null;
        }
    }

   // TODO: need to be removed by Edward, Data LEGO API [start]

   /**
     * Gemini (ToDo)
     * Returns a constant indicating the current data connection state
     * (cellular).
     *
     * @see #DATA_DISCONNECTED
     * @see #DATA_CONNECTING
     * @see #DATA_CONNECTED
     * @see #DATA_SUSPENDED
     * @hide
     * @internal
     */    
    public int getDataStateGemini(int simId) {
        try {
            return getITelephonyEx().getDataState(simId);
        } catch (RemoteException ex) {
            // the phone process is restarting.
            ex.printStackTrace();            
            return TelephonyManager.DATA_DISCONNECTED;
        } catch (NullPointerException ex) {
            ex.printStackTrace();        
            return TelephonyManager.DATA_DISCONNECTED;
        }
    }

       
    /**
        * Returns void
        * (cellular).  
        *
        * @param enable boolean (true/false)
        * @param simId sim slot
        * @hide
        * @internal
        */
    public void setDataRoamingEnabledGemini(boolean enable,int simId) throws RemoteException{
        try {
            getITelephonyEx().setDataRoamingEnabled(enable, simId);
        } catch (RemoteException ex) {
            ex.printStackTrace();
            throw new RemoteException();
        } catch (NullPointerException ex) {
            ex.printStackTrace();
        }
    } 
    // TODO: need to be removed by Edward, Data LEGO API [end]

    /**
        * Returns void
        * (cellular).  
        *
        * @param enable boolean (true/false)
        * @param simId sim slot
        * @hide
        * @internal
        */
    public void setDataRoamingEnabled(boolean enable,int simId) throws RemoteException{
        try {
            getITelephonyEx().setDataRoamingEnabled(enable, simId);
        } catch (RemoteException ex) {
            ex.printStackTrace();
            throw new RemoteException();
        } catch (NullPointerException ex) {
            ex.printStackTrace();
        }
    } 
   
}
