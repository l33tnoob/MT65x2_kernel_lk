# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2011. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.

# Software Name : RCS IMS Stack
#
# Copyright (C) 2010 France Telecom S.A.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

##########################################################################
# Build the application : Rcse.apk
##########################################################################

include $(CLEAR_VARS)

# This is the target being built. (Name of APK)
LOCAL_PACKAGE_NAME := Rcse
LOCAL_MODULE_TAGS := optional

LOCAL_JAVA_LIBRARIES += \
  com.mediatek.contacts.ext \
	com.mediatek.incallui.ext \
	com.mediatek.phone.ext \
	bouncycastle \
	com.mediatek.mms.ext \
	com.mediatek.settings.ext \
	mms-common \
        mediatek-framework

LOCAL_JAVA_LIBRARIES += telephony-common

# Add jar for Xdm Manager. {T-Mobile}
LOCAL_STATIC_JAVA_LIBRARIES := dom4j-1.6.1 \
    jaxen-1.1.1


# Only compile source java files in this apk.
LOCAL_SRC_FILES := \
    $(call all-java-files-under, src)

# Add AIDL files (the parcelable must not be added in SRC_FILES, but included in LOCAL_AIDL_INCLUDES)
LOCAL_SRC_FILES += \
    src/com/orangelabs/rcs/service/api/client/IImsApi.aidl \
    src/com/orangelabs/rcs/service/api/client/terms/ITermsApi.aidl \
    src/com/orangelabs/rcs/service/api/client/capability/ICapabilityApi.aidl \
    src/com/orangelabs/rcs/service/api/client/ipcall/IIPCallApi.aidl \
    src/com/orangelabs/rcs/service/api/client/ipcall/IIPCallEventListener.aidl \
    src/com/orangelabs/rcs/service/api/client/ipcall/IIPCallSession.aidl \
    src/com/orangelabs/rcs/service/api/client/media/IMediaEventListener.aidl \
    src/com/orangelabs/rcs/service/api/client/media/IMediaPlayer.aidl \
    src/com/orangelabs/rcs/service/api/client/media/IMediaRenderer.aidl \
    src/com/orangelabs/rcs/service/api/client/messaging/IChatEventListener.aidl \
    src/com/orangelabs/rcs/service/api/client/messaging/IChatSession.aidl \
    src/com/orangelabs/rcs/service/api/client/messaging/IMessageDeliveryListener.aidl \
    src/com/orangelabs/rcs/service/api/client/messaging/IFileTransferEventListener.aidl \
    src/com/orangelabs/rcs/service/api/client/messaging/IFileTransferSession.aidl \
    src/com/orangelabs/rcs/service/api/client/messaging/IMessagingApi.aidl \
    src/com/orangelabs/rcs/service/api/client/presence/IPresenceApi.aidl \
    src/com/orangelabs/rcs/service/api/client/richcall/IGeolocSharingEventListener.aidl \
    src/com/orangelabs/rcs/service/api/client/richcall/IGeolocSharingSession.aidl \
    src/com/orangelabs/rcs/service/api/client/richcall/IImageSharingEventListener.aidl \
    src/com/orangelabs/rcs/service/api/client/richcall/IImageSharingSession.aidl \
    src/com/orangelabs/rcs/service/api/client/richcall/IRichCallApi.aidl \
    src/com/orangelabs/rcs/service/api/client/richcall/IVideoSharingEventListener.aidl \
    src/com/orangelabs/rcs/service/api/client/richcall/IVideoSharingSession.aidl \
    src/com/orangelabs/rcs/service/api/client/sip/ISipApi.aidl \
    src/com/orangelabs/rcs/service/api/client/sip/ISipSession.aidl \
    src/com/orangelabs/rcs/service/api/client/sip/ISipSessionEventListener.aidl \
    src/com/mediatek/rcse/api/IRegistrationStatusRemoteListener.aidl \
    src/com/mediatek/rcse/service/IRegistrationStatus.aidl \
    src/com/mediatek/rcse/api/ICapabilityRemoteListener.aidl \
    src/com/mediatek/rcse/service/ICapabilities.aidl \
    src/com/orangelabs/rcs/service/api/client/media/IAudioEventListener.aidl \
    src/com/orangelabs/rcs/service/api/client/media/IVideoEventListener.aidl \
    src/com/orangelabs/rcs/service/api/client/media/IAudioRenderer.aidl \
    src/com/orangelabs/rcs/service/api/client/media/IVideoRenderer.aidl \
    src/com/orangelabs/rcs/service/api/client/media/IVideoPlayer.aidl \
    src/com/orangelabs/rcs/service/api/client/media/IAudioPlayer.aidl \
    src/com/mediatek/rcse/service/IFlightMode.aidl \
    src/com/android/server/power/IPreShutdown.aidl \
    src/com/mediatek/rcse/plugin/apn/IRcseOnlyApnStatus.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteChatWindow.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteChatWindowManager.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteFileTransfer.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteGroupChatWindow.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteOne2OneChatWindow.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteReceivedChatMessage.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteSentChatMessage.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteWindowBinder.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteChatWindowMessage.aidl \
    src/com/orangelabs/rcs/core/ims/network/INetworkConnectivity.aidl \
    src/com/orangelabs/rcs/core/ims/network/INetworkConnectivityApi.aidl \

# FRAMEWORKS_BASE_JAVA_SRC_DIRS comes from build/core/pathmap.mk
LOCAL_AIDL_INCLUDES += $(FRAMEWORKS_BASE_JAVA_SRC_DIRS)
LOCAL_AIDL_INCLUDES += \
    src/com/orangelabs/rcs/service/api/client/capability/Capabilities.aidl \
    src/com/orangelabs/rcs/service/api/client/messaging/InstantMessage.aidl \
    src/com/orangelabs/rcs/service/api/client/media/MediaCodec.aidl \
    src/com/orangelabs/rcs/service/api/client/presence/FavoriteLink.aidl \
    src/com/orangelabs/rcs/service/api/client/presence/Geoloc.aidl \
    src/com/orangelabs/rcs/service/api/client/presence/PhotoIcon.aidl \
    src/com/orangelabs/rcs/service/api/client/presence/PresenceInfo.aidl

#Added for JPE begin
ifeq ($(strip $(MTK_AUTO_TEST)), no)
LOCAL_JAVASSIST_ENABLED := true
LOCAL_JAVASSIST_OPTIONS := $(LOCAL_PATH)/jpe.config
endif
#Added for JPE end

# Add classes used by reflection
LOCAL_PROGUARD_FLAG_FILES := proguard.cfg

# Add *.so files to apk for video sharing
LOCAL_JNI_SHARED_LIBRARIES := libH264Decoder \
                              libH264Encoder

LOCAL_EMMA_COVERAGE_FILTER := +com.mediatek.*
# Exclude the java files generaged by aidl files
# Notice that the filter exclude method, so use * to cover all methods
LOCAL_EMMA_COVERAGE_FILTER += -com.mediatek.rcse.service.binder.IRemoteChatWindow* \
                              -com.mediatek.rcse.service.binder.IRemoteChatWindowManager* \
                              -com.mediatek.rcse.service.binder.IRemoteChatWindowMessage* \
                              -com.mediatek.rcse.service.binder.IRemoteFileTransfer* \
                              -com.mediatek.rcse.service.binder.IRemoteGroupChatWindow* \
                              -com.mediatek.rcse.service.binder.IRemoteOne2OneChatWindow* \
                              -com.mediatek.rcse.service.binder.IRemoteReceivedChatMessage* \
                              -com.mediatek.rcse.service.binder.IRemoteSentChatMessage* \
                              -com.mediatek.rcse.service.binder.IRemoteWindowBinder* \
                              -com.mediatek.rcse.api.IRegistrationStatusRemoteListener* \
                              -com.mediatek.rcse.api.ICapabilityRemoteListener* \
                              -com.mediatek.rcse.service.IRegistrationStatus* \
                              -com.mediatek.rcse.service.ICapabilities* \
                              -com.mediatek.rcse.service.IFlightMode* \
                              -com.mediatek.rcse.plugin.apn.IRcseOnlyApnStatus*
# Exclude the debug java files
LOCAL_EMMA_COVERAGE_FILTER += -com.mediatek.rcse.settings.PhoneNumberToAccountSettings* \
                              -com.mediatek.rcse.settings.ProvisionProfileSettings* \
                              -com.mediatek.rcse.activities.ConfigMessageActicity* \
                              -com.mediatek.rcse.activities.RoamingActivity*

#Specify install path for MTK CIP solution
ifeq ($(strip $(MTK_CIP_SUPPORT)),yes)
LOCAL_MODULE_PATH := $(TARGET_CUSTOM_OUT)/app

endif


# Tell it to build an APK
include $(BUILD_PACKAGE)

include $(CLEAR_VARS)

# Add jar for Xdm Manager. {T-Mobile}
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := dom4j-1.6.1:libs/dom4j-1.6.1.jar \
    jaxen-1.1.1:libs/jaxen-1.1.1.jar

include $(BUILD_MULTI_PREBUILT)

#For auto test
include $(CLEAR_VARS)

include $(call all-makefiles-under,$(LOCAL_PATH))
