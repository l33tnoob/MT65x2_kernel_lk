# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2010. All rights reserved.
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

#Add other MTK created AIDL files (Common to MR0 & MR1 architecture)
LOCAL_SRC_FILES += \
				../../mediatek/frameworks-ext/base/core/java/android/bluetooth/IBluetoothBipi.aidl \
				../../mediatek/frameworks-ext/base/core/java/android/bluetooth/IBluetoothBipr.aidl \
				../../mediatek/frameworks-ext/base/core/java/android/bluetooth/IBluetoothBpp.aidl \
				../../mediatek/frameworks-ext/base/core/java/android/bluetooth/IBluetoothDun.aidl \
				../../mediatek/frameworks-ext/base/core/java/android/bluetooth/IBluetoothFtpCtrl.aidl \
				../../mediatek/frameworks-ext/base/core/java/android/bluetooth/IBluetoothFtpServer.aidl \
				../../mediatek/frameworks-ext/base/core/java/android/bluetooth/IBluetoothFtpServerCallback.aidl \
				../../mediatek/frameworks-ext/base/core/java/android/bluetooth/IBluetoothProfileManager.aidl \
				../../mediatek/frameworks-ext/base/core/java/android/bluetooth/IBluetoothSimap.aidl \
				../../mediatek/frameworks-ext/base/core/java/android/bluetooth/IBluetoothSimapCallback.aidl \
				../../mediatek/frameworks-ext/base/core/java/com/mediatek/bluetooth/service/IBluetoothPrxm.aidl \
				../../mediatek/frameworks-ext/base/core/java/com/mediatek/bluetooth/service/IBluetoothPrxr.aidl \
                                ../../mediatek/frameworks-ext/base/core/java/android/net/INetworkManagementIpv6EventObserver.aidl

#Use MR1 architecture. Filter out MR0 SDK files

#Do not filter out for Atci Service
#LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/bluetooth/AtCommandHandler.java, $(LOCAL_SRC_FILES))
#LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/bluetooth/AtCommandResult.java, $(LOCAL_SRC_FILES))
#LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/bluetooth/AtParser.java, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/bluetooth/BluetoothA2dp.java, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/bluetooth/BluetoothHealth.java, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/bluetooth/BluetoothHealthAppConfiguration.java, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/bluetooth/BluetoothHealthCallback.java, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/bluetooth/BluetoothHid.java, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/bluetooth/BluetoothInputDevice.java, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/bluetooth/BluetoothPan.java, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/bluetooth/BluetoothOpp.java, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/bluetooth/BluetoothMap.java, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/bluetooth/BluetoothPbap.java, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/bluetooth/BluetoothTetheringDataTracker.java, $(LOCAL_SRC_FILES))

#Use MR1 architecture. Filter out MR0 files
LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/server/BluetoothA2dpService.java, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/server/BluetoothHealthProfileHandler.java, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/server/BluetoothInputProfileHandler.java, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/server/BluetoothPanProfileHandler.java, $(LOCAL_SRC_FILES))
#MTK Turnkey files
#LOCAL_SRC_FILES := $(filter-out ../../mediatek/frameworks-ext/base/core/java/android/server/BluetoothProfileManagerService.java, $(LOCAL_SRC_FILES))

LOCAL_SRC_FILES += \
	../../mediatek/frameworks-ext/base/core/java/com/mediatek/hotknot/IHotKnotCallback.aidl \
	../../mediatek/frameworks-ext/base/core/java/com/mediatek/hotknot/IHotKnotAdapter.aidl
