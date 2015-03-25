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

package com.mediatek.common;

import java.io.File;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

//import com.mediatek.common.IFwkExt;
import com.mediatek.common.wifi.IWifiFwkExt;
import com.mediatek.common.net.IConnectivityServiceExt;
import com.mediatek.common.util.IPatterns;
import com.mediatek.common.bootanim.IBootAnimExt;
import com.mediatek.common.media.IRCSePriorityExt;
import com.mediatek.common.telephony.IServiceStateExt;
import com.mediatek.common.telephony.ITetheringExt;
import com.mediatek.common.telephony.IPhoneNumberExt;
import com.mediatek.common.telephony.IGsmConnectionExt;
import com.mediatek.common.telephony.ITelephonyExt;
import com.mediatek.common.telephony.ICallerInfoExt;
import com.mediatek.common.media.IOmaSettingHelper;
import com.mediatek.common.media.IAudioServiceExt;
import com.mediatek.common.audioprofile.IAudioProfileExtension;
import com.mediatek.common.sms.IWapPushFwkExt;
import com.mediatek.common.sms.IConcatenatedSmsFwkExt;
import com.mediatek.common.sms.IDupSmsFilterExt;
import com.mediatek.common.telephony.ITelephonyProviderExt;
import com.mediatek.common.telephony.IGsmDCTExt;
import com.mediatek.common.sms.IDefaultSmsSimSettingsExt;
import com.mediatek.common.util.IWebProtocolNames;
import com.mediatek.common.telephony.IUiccControllerExt;
import com.mediatek.common.search.IRegionalPhoneSearchEngineExt;
import com.mediatek.common.lowstorage.ILowStorageExt;

/// M: IR feature start @{
import com.mediatek.common.telephony.internationalroaming.strategy.ICardStrategy;
import com.mediatek.common.telephony.internationalroaming.strategy.IDataStrategy;
import com.mediatek.common.telephony.internationalroaming.strategy.IGeneralStrategy;
import com.mediatek.common.telephony.internationalroaming.strategy.INetworkSelectionStrategy;
/// M: IR feature end @}

/// M: Account sync start
import com.mediatek.common.accountsync.ISyncManagerExt;
/// M: Account sync feature end @}


public class OperatorInterface {
	
		public static String getClass(Class clazz){
				return opInterfaceMap.get(clazz);    	
		}
		
		public static boolean getContainsKey(Class clazz){
				return opInterfaceMap.containsKey(clazz); 			
		}
	
    private static Map<Class, String> opInterfaceMap = new HashMap<Class, String>();
    static {
        opInterfaceMap.put(IWifiFwkExt.class,
                "com.mediatek.op.wifi.DefaultWifiFwkExt");
        opInterfaceMap.put(IConnectivityServiceExt.class,
                "com.mediatek.op.net.DefaultConnectivityServiceExt");
        opInterfaceMap.put(IPatterns.class,
                "com.mediatek.op.util.DefaultPatterns");
        opInterfaceMap.put(IBootAnimExt.class,
                "com.mediatek.op.bootanim.DefaultBootAnimExt");
        opInterfaceMap.put(IServiceStateExt.class,
                "com.mediatek.op.telephony.ServiceStateExt");
        opInterfaceMap.put(ITetheringExt.class,
                "com.mediatek.op.telephony.TetheringExt");
        opInterfaceMap.put(IPhoneNumberExt.class,
                "com.mediatek.op.telephony.PhoneNumberExt");
        opInterfaceMap.put(IGsmConnectionExt.class,
                "com.mediatek.op.telephony.GsmConnectionExt");
        opInterfaceMap.put(ITelephonyExt.class,
                "com.mediatek.op.telephony.TelephonyExt");
        opInterfaceMap.put(ICallerInfoExt.class,
                "com.mediatek.op.telephony.CallerInfoExt");
        opInterfaceMap.put(IOmaSettingHelper.class,
                "com.mediatek.op.media.DefaultOmaSettingHelper");
        opInterfaceMap.put(IAudioServiceExt.class,
                "com.mediatek.common.media.IAudioServiceExt");
        opInterfaceMap.put(IRCSePriorityExt.class,
                "com.mediatek.op.media.DefaultRCSePriority");
        opInterfaceMap.put(IAudioProfileExtension.class,
                "com.mediatek.op.audioprofile.DefaultAudioProfileExtension");
        opInterfaceMap.put(
                IAudioProfileExtension.IDefaultProfileStatesGetter.class,
                "com.mediatek.op.audioprofile.DefaultProfileStatesGetter");
        opInterfaceMap.put(IWapPushFwkExt.class,
                "com.mediatek.op.sms.WapPushFwkExt");
        opInterfaceMap.put(IConcatenatedSmsFwkExt.class,
                "com.mediatek.op.sms.ConcatenatedSmsFwkExt");
        opInterfaceMap.put(IDupSmsFilterExt.class, 
                "com.mediatek.op.sms.DupSmsFilterExt");
        opInterfaceMap.put(ITelephonyProviderExt.class,
                "com.mediatek.op.telephony.TelephonyProviderExt");
        opInterfaceMap.put(IGsmDCTExt.class,
                "com.mediatek.op.telephony.GsmDCTExt");
        opInterfaceMap.put(IDefaultSmsSimSettingsExt.class,
                "com.mediatek.telephony.DefaultSmsSimSettings");
        opInterfaceMap.put(IWebProtocolNames.class, 
                "com.mediatek.op.util.DefaultWebProtocolNames");
        opInterfaceMap.put(IUiccControllerExt.class, 
                "com.mediatek.op.telephony.UiccControllerExt");
        opInterfaceMap.put(IRegionalPhoneSearchEngineExt.class,
        "com.mediatek.op.search.DefaultRegionalPhoneSearchEngineExt");
        opInterfaceMap.put(ICardStrategy.class, 
                "com.mediatek.op.telephony.internationalroaming.strategy.DefaultCardStrategy");
        opInterfaceMap.put(IDataStrategy.class, 
                "com.mediatek.op.telephony.internationalroaming.strategy.DefaultDataStrategy");
        opInterfaceMap.put(IGeneralStrategy.class, 
                "com.mediatek.op.telephony.internationalroaming.strategy.DefaultGeneralStrategy");
        opInterfaceMap.put(INetworkSelectionStrategy.class, 
                "com.mediatek.op.telephony.internationalroaming.strategy.DefaultNetworkSelectionStrategy");
        opInterfaceMap.put(ISyncManagerExt.class,
                "com.mediatek.op.accountsync.SyncManagerExt");
        opInterfaceMap.put(ILowStorageExt.class,
                "com.mediatek.op.lowstorage.LowStorageExt");

    }
}