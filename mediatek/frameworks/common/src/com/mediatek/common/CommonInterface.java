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

import com.mediatek.common.telephony.ISimInfoUpdate;
import com.mediatek.common.aee.IExceptionLog;
import com.mediatek.common.lowstorage.*;
import com.mediatek.common.agps.MtkAgpsManager;
import com.mediatek.common.agps.IMtkAgpsManager;
import com.mediatek.common.dcfdecoder.IDcfDecoder;
import com.mediatek.common.gifdecoder.IGifDecoder;

import com.mediatek.common.audioprofile.IAudioProfileService;
import com.mediatek.common.audioprofile.IAudioProfileManager;
import com.mediatek.common.hdmi.IHDMINative;
import com.mediatek.common.hdmi.IMtkHdmiManager;
import com.mediatek.common.voicecommand.IVoicePhoneDetection;
import com.mediatek.common.amsplus.IAmsPlus;
/// M: MSG Logger Manager @{
import com.mediatek.common.msgmonitorservice.IMessageLogger;
import com.mediatek.common.msgmonitorservice.IMessageLoggerWrapper;
/// MSG Logger Manager @}

import com.mediatek.common.search.ISearchEngineManager;
import com.mediatek.common.search.ISearchEngineManagerService;

/// M: IR feature start @{
import com.mediatek.common.telephony.internationalroaming.IInternationalRoamingController;
/// M: IR feature end @}

/// M: Mobile Manager Service @{
import com.mediatek.common.mom.IMobileManager;
import com.mediatek.common.mom.IMobileManagerService;
/// @}

/// M: IPerfService feature start @{
import com.mediatek.common.perfservice.IPerfService;
import com.mediatek.common.perfservice.IPerfServiceWrapper;
import com.mediatek.common.perfservice.IPerfServiceManager;
/// M: IPerfService feature end @}

/// M: Privacy Protection Lock SMS Filter @{
import com.mediatek.common.ppl.IPplSmsFilter;
/// @}

/// M: StorageManagerEx start @{
import com.mediatek.common.storage.IStorageManagerEx;
/// M: StorageManagerEx end @}
import com.mediatek.common.geocoding.IGeoCodingQuery;
import com.mediatek.common.stereo3d.IJpsParser;
import com.mediatek.common.telephony.IWorldPhone;
import com.mediatek.common.telephony.ISwitch3GPolicyWrapper;
import com.mediatek.common.mpodecoder.IMpoDecoder;
import com.mediatek.common.smsdbpermission.ISmsDbVisitor;

/// M: BG powerSaving feature start @{
import com.mediatek.common.amplus.IAlarmMangerPlus;
/// M: BG powerSaving feature end @}
public class CommonInterface {
	
		public static String getClass(Class clazz){
				return commonInterfaceMap.get(clazz);    	
		}
		
		public static boolean getContainsKey(Class clazz){
				return commonInterfaceMap.containsKey(clazz); 			
		}
	
    private static Map<Class, String> commonInterfaceMap = new HashMap<Class, String>();
    static {
        commonInterfaceMap.put(ISimInfoUpdate.class,
                "com.mediatek.telephony.SimInfoUpdateAdp");
        commonInterfaceMap.put(IExceptionLog.class,
                "com.mediatek.exceptionlog.ExceptionLog");
        commonInterfaceMap.put(ILowStorageHandle.class,
                "com.mediatek.lowstorage.LowStorageHandle");
        commonInterfaceMap.put(MtkAgpsManager.class,
                "com.mediatek.agps.MtkAgpsManagerImpl");
        commonInterfaceMap.put(IMtkAgpsManager.class,
                "com.mediatek.agps.MtkAgpsManagerService");
        commonInterfaceMap.put(IDcfDecoder.class,
                "com.mediatek.dcfdecoder.DcfDecoder");
        commonInterfaceMap.put(IGifDecoder.class,
                "com.mediatek.gifdecoder.GifDecoder");
        commonInterfaceMap.put(IAudioProfileService.class,
                "com.mediatek.audioprofile.AudioProfileService");
        commonInterfaceMap.put(IAudioProfileManager.class,
                "com.mediatek.audioprofile.AudioProfileManager");
        commonInterfaceMap.put(IMtkHdmiManager.class,
                "com.mediatek.hdmi.MtkHdmiManagerService");
        commonInterfaceMap.put(IHDMINative.class,
                "com.mediatek.hdmi.HDMINative");
        commonInterfaceMap.put(IVoicePhoneDetection.class,
                "com.mediatek.voicecommand.app.VoicePhoneDetection");
        commonInterfaceMap.put(IAmsPlus.class,
            	"com.mediatek.amsplus.ActivityStackPlus");
        commonInterfaceMap.put(IMessageLogger.class,
                "com.mediatek.msglogger.MessageMonitorService");
        commonInterfaceMap.put(IMessageLoggerWrapper.class,
                "com.mediatek.msglogger.MessageLoggerWrapper");
        commonInterfaceMap.put(ISearchEngineManager.class,
                "com.mediatek.search.SearchEngineManager");
        commonInterfaceMap.put(ISearchEngineManagerService.class,
                "com.mediatek.search.SearchEngineManagerService");
        commonInterfaceMap.put(IMobileManager.class,
                "com.mediatek.mom.MobileManager");
        commonInterfaceMap.put(IMobileManagerService.class,
                "com.mediatek.mom.MobileManagerService");
        commonInterfaceMap.put(IInternationalRoamingController.class,
                "com.mediatek.telephony.InternationalRoamingController");
        commonInterfaceMap.put(IPerfService.class,
                "com.mediatek.perfservice.PerfService");
        commonInterfaceMap.put(IPerfServiceWrapper.class,
                "com.mediatek.perfservice.PerfServiceWrapper");
        commonInterfaceMap.put(IPerfServiceManager.class,
                "com.mediatek.perfservice.PerfServiceManagerImpl");                
        commonInterfaceMap.put(IGeoCodingQuery.class,
                "com.mediatek.geocoding.GeoCodingQueryWrapper");
        commonInterfaceMap.put(IJpsParser.class,
                "com.mediatek.stereo3d.JpsParserWrapper");
        commonInterfaceMap.put(IWorldPhone.class,
                "com.mediatek.telephony.WorldPhoneWrapper");
        commonInterfaceMap.put(ISwitch3GPolicyWrapper.class,
                "com.mediatek.telephony.Switch3GPolicyWrapper");
        commonInterfaceMap.put(IMpoDecoder.class,
                "com.mediatek.mpo.MpoDecoderWrapper");
        commonInterfaceMap.put(IStorageManagerEx.class,
                "com.mediatek.storage.StorageManagerExWrapper");
        commonInterfaceMap.put(IAlarmMangerPlus.class,
                "com.mediatek.amplus.AlarmManagerPlus");
        commonInterfaceMap.put(IPplSmsFilter.class,
                "com.mediatek.telephony.PplSmsFilterExtension");
        commonInterfaceMap.put(ISmsDbVisitor.class,
                "com.mediatek.smsdbpermission.SmsDbVisitor");

    }
}
