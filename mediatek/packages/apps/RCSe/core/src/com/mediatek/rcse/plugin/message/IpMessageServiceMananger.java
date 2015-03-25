/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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
package com.mediatek.rcse.plugin.message;

import android.content.Context;

import com.mediatek.mms.ipmessage.IpMessageConsts;
import com.mediatek.mms.ipmessage.IpMessageConsts.IntegrationMode;
import com.mediatek.mms.ipmessage.ServiceManager;
import com.mediatek.mms.ipmessage.IpMessageConsts.FeatureId;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.PluginApiManager.ContactInformation;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.LauncherUtils;

/**
 * This class manages the rcse service and monitor the status of Registration or RcsCoreServiceStatus.
 */
public class IpMessageServiceMananger extends ServiceManager implements PluginApiManager.CapabilitiesChangeListener {
    protected static final String TAG = "IpMessageServiceManger";
    private static final String PROVISION_INFO_VERSION_ONE = "-1";
    private static final String PROVISION_INFO_VERSION_ZERO = "0";
    private static final String PERMENTLY_DISABLE_VALUE = "-3";
    private static final long PROVISION_INFO_VILIDITY_ONE = -1;
    private static final long PROVISION_INFO_VILIDITY_ZERO = 0;
    public static int INTEGRATION_MODE = 0;

    public IpMessageServiceMananger(Context context) {
        super(context);
        PluginApiManager.getInstance().addCapabilitiesChangeListener(this);
    }

    @Override
    public boolean isActivated() {
        Logger.d(TAG, "isActivated() entry");
        return isRcseActivated();
    }

    @Override
    public boolean isActivated(int simId) {
        Logger.d(TAG, "isActivated(int simId) entry with simId is " + simId);
        return isRcseActivated();
    }

    @Override
    public int getIpMessageServiceId() {
        Logger.d(TAG, "getIpMessageServiceId() entry");
        return IpMessageConsts.IpMessageServiceId.ISMS_SERVICE;
    }

    @Override
    public boolean serviceIsReady() {
        Logger.d(TAG, "serviceIsReady() entry mRegistrationStatus is "
                + PluginApiManager.getInstance().getRegistrationStatus());
        return PluginApiManager.getInstance().getRegistrationStatus();
    }

    @Override
    public void startIpService() {
        Logger.d(TAG, "startIpService() entry ");
        LauncherUtils.launchRcsService(mContext, false, false);
    }

    @Override
    public boolean isEnabled() {
        Logger.d(TAG, "isEnabled() entry ");
        return isRcseEnabled();
    }

    @Override
    public boolean isEnabled(int simId) {
        Logger.d(TAG, "isEnabled(int simId) entry with simId is " + simId);
        return isRcseEnabled();
    }
    
    /* If joyn messaging disabled, will only use XMS */    
	public boolean isAlwaysSendMessageByJoyn() {
		boolean imCaps = false;
		if (RcsSettings.getInstance() == null)
			RcsSettings.createInstance(mContext);
		imCaps = RcsSettings.getInstance().isImAlwaysOn();
		Logger.d(TAG, "isAlwaysSendMessageByJoyn imcapability is " + imCaps);
		return imCaps;
    }

    private boolean isRcseEnabled() {
        Logger.d(TAG, "isRcseEnabled() entry");
        RcsSettings.createInstance(mContext);
        RcsSettings rcsSettings = RcsSettings.getInstance();
        boolean isEnabled = false;
        if (rcsSettings != null) {
            isEnabled = rcsSettings.isServiceActivated();
        } else {
            Logger.w(TAG, "isRcseEnabled(), rcsSettings is null");
        }
        Logger.d(TAG, "isRcseEnabled() exit with isEnabled is " + isEnabled);
        return isEnabled;
    }

    public boolean isJoynPermanentlyDisabled()
    {
    	boolean joynStatus = false;
    	if(RcsSettings.getInstance().getProvisioningVersion().equals(PERMENTLY_DISABLE_VALUE))
    	{
    		Logger.d(TAG, "isJoynPermanentlyDisabled" + true);
    		return true;
    	}
    	return joynStatus;
    }

    public int getDisableServiceStatus() {
    	int status = 0;
    	if(RcsSettings.getInstance() == null)
    		RcsSettings.createInstance(AndroidFactory.getApplicationContext());
    	status = RcsSettings.getInstance().getDisableServiceStatus();
    	Logger.d(TAG, "getDisableServiceStatus" + status);
        return status;
    }


	public boolean isAlwaysSendFileByJoyn() {
		int ftCaps = 0;
		if (RcsSettings.getInstance() == null)
			RcsSettings.createInstance(mContext);
		ftCaps = RcsSettings.getInstance().getFtHttpCapAlwaysOn();
		Logger.d(TAG, "isAlwaysSendFileByJoyn FTcapability is " + ftCaps);
		if (ftCaps == 1)
    	return true;
		else
			return false;
    }

    /* Need to be overridden from host app*/
    
    @Override
	public int getIntegrationMode() {
		int integrationMode;
		if (RcsSettings.getInstance() == null)
			RcsSettings.createInstance(mContext);
		integrationMode = RcsSettings.getInstance().getMessagingUx();

		if (integrationMode == 1) {
			Logger.d(TAG, "getIntegrationMode , mode =" + integrationMode);
    	return IpMessageConsts.IntegrationMode.FULLY_INTEGRATED;
		} else {
			Logger.d(TAG, "getIntegrationMode , mode =" + integrationMode);
			return IpMessageConsts.IntegrationMode.CONVERGED_INBOX;
		}
    }
    
    /**
     * Get configuration value to check whether RCSe can be used.
     * 
     * @return True if RCSe can be used, otherwise false.
     */
    private boolean isRcseActivated() {
        Logger.d(TAG, "isRcseActivated() entry");
        if (LauncherUtils.getDebugMode(mContext)) {
            Logger.d(TAG, "isRcseActivated() debug mode,do not care configuration");
            return true;
        }
        return checkIsRcseActivated();
    }

    private boolean checkIsRcseActivated() {
        RcsSettings.createInstance(mContext);
        RcsSettings rcsSettings = RcsSettings.getInstance();
        boolean isActivated = false;
        if (rcsSettings != null) {
            long validity = rcsSettings.getProvisionValidity();
            String version = rcsSettings.getProvisioningVersion();
            Logger.d(TAG, "isRcseActivated(),validity is " + validity + ", version is " + version);
            if ((version.equals(PROVISION_INFO_VERSION_ONE) && validity == PROVISION_INFO_VILIDITY_ONE)
                    || (version.equals(PROVISION_INFO_VERSION_ZERO) && validity == PROVISION_INFO_VILIDITY_ZERO)) {
                isActivated = false;
            } else {
                isActivated = true;
            }
        } else {
            Logger.w(TAG, "isRcseActivated(), rcsSettings is null");
        }
        return isActivated;
    }

    @Override
    public boolean isFeatureSupported(int featureId) {
        switch (featureId) {
            case FeatureId.CHAT_SETTINGS:
            case FeatureId.ACTIVITION:            
            case FeatureId.ACTIVITION_WIZARD:
            case FeatureId.MEDIA_DETAIL:
            case FeatureId.GROUP_MESSAGE:
            case FeatureId.SKETCH:
            case FeatureId.TERM:
            case FeatureId.FILE_TRANSACTION:
            case FeatureId.EXTEND_GROUP_CHAT:
            case FeatureId.EXPORT_CHAT:
            case FeatureId.PARSE_EMO_WITHOUT_ACTIVATE:
                return true;
            case FeatureId.APP_SETTINGS:
            {
            	if(RcsSettings.getInstance().getDisableServiceStatus() == 2)
            		return false;
            	else 
            		return true;
            }
            default:
                return super.isFeatureSupported(featureId);
        }
    }

    @Override
    public void onCapabilitiesChanged(String contact, ContactInformation contactInformation) {
        Logger.d(TAG, "onCapabilitiesChanged() entry capabilities is " + contact);
        if (contactInformation.isRcsContact == 1 && !ContactsListManager.getInstance().isLocalContact(contact)
                && !ContactsListManager.getInstance().isStranger(contact)) {
            ContactsListManager.getInstance().setStrangerList(contact, true);
        }
    }

    @Override
    public void onApiConnectedStatusChanged(boolean isConnected) {
    }
}
