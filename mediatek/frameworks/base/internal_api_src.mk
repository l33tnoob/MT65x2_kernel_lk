# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
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

internal_docs_FRAMEWORK_AIDL_JAVA_DIR := $(call intermediates-dir-for,JAVA_LIBRARIES,framework,,COMMON)/src
internal_docs_TEL_COMMON_AIDL_JAVA_DIR := $(call intermediates-dir-for,JAVA_LIBRARIES,telephony-common,,COMMON)/src
internal_docs_MTK_COMMON_AIDL_JAVA_DIR := $(call intermediates-dir-for,JAVA_LIBRARIES,mediatek-common,,COMMON)/src

INTERNAL_API_SRC_FILES := \
    ../../../external/apache-http/src/org/apache/http/params/HttpConnectionParams.java \
    ../../../external/libphonenumber/java/src/com/android/i18n/phonenumbers/Phonenumber.java \
    ../../../frameworks/base/core/java/android/app/AlarmManager.java \
    ../../../frameworks/base/core/java/android/app/DownloadManager.java \
    ../../../frameworks/base/core/java/android/app/StatusBarManager.java \
    ../../../frameworks/base/core/java/android/content/pm/PackageInfo.java \
    ../../../frameworks/base/core/java/android/content/res/Configuration.java \
    ../../../frameworks/base/core/java/android/content/res/Resources.java \
    ../../../frameworks/base/core/java/android/database/DatabaseUtils.java \
    ../../../frameworks/base/core/java/android/hardware/Camera.java \
    ../../../frameworks/base/core/java/android/net/ConnectivityManager.java \
    ../../../frameworks/base/core/java/android/net/http/AndroidHttpClient.java \
    ../../../frameworks/base/core/java/android/net/NetworkInfo.java \
    ../../../frameworks/base/core/java/android/net/NetworkPolicy.java \
    ../../../frameworks/base/core/java/android/net/NetworkUtils.java \
    ../../../frameworks/base/core/java/android/nfc/NfcAdapter.java \
    ../../../frameworks/base/core/java/android/provider/CallLog.java \
    ../../../frameworks/base/core/java/android/provider/MediaStore.java \
    ../../../frameworks/base/core/java/android/provider/Settings.java \
    ../../../frameworks/base/core/java/android/view/SurfaceView.java \
    ../../../frameworks/base/core/java/android/webkit/WebView.java \
    ../../../frameworks/base/core/java/android/widget/ListView.java \
    ../../../frameworks/base/core/java/android/widget/Spinner.java \
    ../../../frameworks/base/core/java/android/widget/TextView.java \
    ../../../frameworks/base/core/java/com/android/internal/widget/LockPatternUtils.java \
    ../../../frameworks/base/core/java/com/android/internal/widget/multiwaveview/GlowPadView.java \
    ../../../frameworks/base/media/java/android/media/AudioManager.java \
    ../../../frameworks/base/media/java/android/media/AudioSystem.java \
    ../../../frameworks/base/media/java/android/media/MediaPlayer.java \
    ../../../frameworks/base/media/java/android/media/MediaRecorder.java \
    ../../../frameworks/base/media/java/android/media/RingtoneManager.java \
    ../../../frameworks/base/media/java/android/media/videoeditor/VideoEditorImpl.java \
    ../../../frameworks/base/media/java/android/mtp/MtpServer.java \
    ../../../frameworks/base/services/java/com/android/server/NetworkManagementService.java \
    ../../../frameworks/base/telephony/java/android/telephony/PhoneNumberUtils.java \
    ../../../frameworks/base/telephony/java/android/telephony/PhoneStateListener.java \
    ../../../frameworks/base/telephony/java/android/telephony/ServiceState.java \
    ../../../frameworks/base/telephony/java/android/telephony/SignalStrength.java \
    ../../../frameworks/base/telephony/java/android/telephony/TelephonyManager.java \
    ../../../frameworks/base/telephony/java/com/android/internal/telephony/CallerInfo.java \
    ../../../frameworks/base/telephony/java/com/android/internal/telephony/CallerInfoAsyncQuery.java \
    ../../../frameworks/base/telephony/java/com/android/internal/telephony/PhoneConstants.java \
    ../../../frameworks/base/telephony/java/com/android/internal/telephony/TelephonyIntents.java \
    ../../../frameworks/base/wifi/java/android/net/wifi/WifiConfiguration.java \
    ../../../frameworks/base/wifi/java/android/net/wifi/WifiManager.java \
    ../../../frameworks/opt/telephony/src/java/android/provider/Telephony.java \
    ../../../frameworks/opt/telephony/src/java/android/telephony/SmsMessage.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/BaseCommands.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/Call.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/CallManager.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/cat/AppInterface.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/cat/bip/BearerDesc.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/cat/CatCmdMessage.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/cat/CatResponseMessage.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/cat/CatService.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/cat/Input.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/cat/Menu.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/CommandException.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/Connection.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/gsm/NetworkInfoWithAcT.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/IccCard.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/Phone.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/PhoneFactory.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/PhoneProxy.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/PhoneStateIntentReceiver.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/uicc/AdnRecord.java \
    ../../../frameworks/opt/telephony/src/java/com/android/internal/telephony/uicc/IccFileHandler.java \
    ../../../mediatek/frameworks/base/bluetooth/java/com/mediatek/bluetooth/BluetoothUuidEx.java \
    ../../../mediatek/frameworks/base/op/java/com/mediatek/op/telephony/cat/CatOpAppInterfaceImp.java \
    ../../../mediatek/frameworks/base/storage/java/com/mediatek/storage/StorageManagerEx.java \
    ../../../mediatek/frameworks/base/telephony/java/com/mediatek/telephony/PhoneNumberUtilsEx.java \
    ../../../mediatek/frameworks/base/telephony/java/com/mediatek/telephony/SimInfoManager.java \
    ../../../mediatek/frameworks/base/telephony/java/com/mediatek/telephony/TelephonyManagerEx.java \
    ../../../mediatek/frameworks/base/text/java/com/mediatek/text/style/BackgroundImageSpan.java \
    ../../../mediatek/frameworks/common/src/com/mediatek/common/audioprofile/AudioProfileListener.java \
    ../../../mediatek/frameworks/common/src/com/mediatek/common/search/ISearchEngineManager.java \
    ../../../mediatek/frameworks/common/src/com/mediatek/common/search/SearchEngineInfo.java \
    ../../../mediatek/frameworks/common/src/com/mediatek/common/telephony/IccSmsStorageStatus.java \
    ../../../mediatek/frameworks/common/src/com/mediatek/common/telephony/IServiceStateExt.java \
    ../../../mediatek/frameworks/common/src/com/mediatek/common/telephony/ITelephonyExt.java \
    ../../../mediatek/frameworks/common/src/com/mediatek/common/telephony/ITelephonyProviderExt.java \
    ../../../mediatek/frameworks-ext/base/core/java/android/widget/BounceCoverFlow.java \
    ../../../mediatek/frameworks-ext/base/core/java/android/widget/BounceGallery.java \
    ../../../mediatek/frameworks-ext/base/telephony/java/android/telephony/BtSimapOperResponse.java \
    ../../../mediatek/frameworks-ext/base/wifi/java/android/net/wifi/HotspotClient.java \
    ../../../mediatek/protect/frameworks/base/telephony/java/com/android/internal/telephony/gemini/GeminiPhone.java \
    ../../../mediatek/protect/frameworks/base/telephony/java/com/android/internal/telephony/gemini/MTKCallManager.java \
    ../../../mediatek/protect/frameworks/base/telephony/java/com/android/internal/telephony/gemini/MTKPhoneFactory.java \
    ../../../mediatek/protect/frameworks/base/telephony/java/com/android/internal/telephony/worldphone/ModemSwitchHandler.java \

# Specify directory of intermediate source files (e.g. AIDL) here.  
INTERNAL_API_ADDITIONAL_SRC_DIR := \
#    $(internal_docs_TEL_COMMON_AIDL_JAVA_DIR)/src/java/com/android/internal/telephony \
#    $(internal_docs_MTK_COMMON_AIDL_JAVA_DIR)/src/com/mediatek/common/dm
