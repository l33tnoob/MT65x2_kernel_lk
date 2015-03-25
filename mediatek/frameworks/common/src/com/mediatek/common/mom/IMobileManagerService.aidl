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

/**
 * aidl file : src/com/mediatek/common/mom/IMobileManagerService.aidl
 * This file contains definitions of functions which are exposed by service
 */
package com.mediatek.common.mom;

import android.content.pm.PackageInfo;
import android.content.pm.ResolveInfo;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;

import com.mediatek.common.mom.IPermissionListener;
import com.mediatek.common.mom.IRequestedPermissionCallback;
import com.mediatek.common.mom.IPackageInstallCallback;
import com.mediatek.common.mom.INotificationListener;
import com.mediatek.common.mom.IMobileConnectionCallback;
import com.mediatek.common.mom.NotificationCacheRecord;
import com.mediatek.common.mom.Permission;
import com.mediatek.common.mom.PermissionRecord;
import com.mediatek.common.mom.ReceiverRecord;

interface IMobileManagerService {
    void systemReady();

    /**
     * Get the version of MobileManagerService
     *
     * @return Returns version name of MobileManagerService.
     *
     */
    String getVersionName();

    /**
     * Attach to MobileManagerService
     *
     * @param callback The callback will be triggered when the connection terminated or resumed.
     * @return Returns the result of attachment.
     */
    boolean attach(in IMobileConnectionCallback callback);

    /**
     * Detach from MobileManagerService
     *
     */
    void detach();

    /**
     * Clear all settings
     *
     */
    void clearAllSettings();

    /**
     * Clear setting for package
     *
     * @param packageName The specific package that setting will be erased.
     */
    void clearPackageSettings(String packageName);

    void registerManagerApListener(int controllerID, in IBinder listener);
    int triggerManagerApListener(int ControllerID, in Bundle params, int defaultResult);
    void triggerManagerApListenerAsync(int ControllerID, in Bundle params, int defaultResult, in IBinder callback);

    /**
     * [Permission Controller Functions]
     */
    void enablePermissionController(boolean enable);
    List<PackageInfo> getInstalledPackages();
    List<Permission> getPackageGrantedPermissions(String packageName);
    void setPermissionRecord(in PermissionRecord record);
    void setPermissionRecords(in List<PermissionRecord> records);
    void setPermissionCache(in List<PermissionRecord> cache);
    int checkPermission(String permissionName, int uid);
    void checkPermissionAsync(String permissionName, int uid, in IRequestedPermissionCallback callback);
    int checkPermissionWithData(String permissionName, int uid, in Bundle data);
    void checkPermissionAsyncWithData(String permissionName, int uid, in Bundle data, in IRequestedPermissionCallback callback);
    String getParentPermission(String subPermissionName);
    long getUserConfirmTime(int userId, long timeBound);


    /**
     * [Receiver Controller Functions]
     */
    List<ReceiverRecord> getBootReceiverList();
    void setBootReceiverEnabledSettings(in List<ReceiverRecord> list);
    void setBootReceiverEnabledSetting(String packageName, boolean enable);
    boolean getBootReceiverEnabledSetting(String packageName);
    void filterReceiver(in Intent intent, inout List<ResolveInfo> resolveList, int userId);
    void startMonitorBootReceiver(String cause);
    void stopMonitorBootReceiver(String cause);

    /**
     * [Package Controller Functions]
     */
    /**
     * Forcestop the specified package.
     * Protection Level: License
     *
     * @param packageName The name of the package to be forcestoped.
     */
    void forceStopPackage(String packageName);

    /**
     * Install a package. Since this may take a little while, the result will
     * be posted back to the given callback.
     * Protection Level: License
     *
     * @param packageURI The location of the package file to install.  This can be a 'file:' or a 'content:' URI.
     * @param callback An callback to get notified when the package installation is complete.
     */
    void installPackage(in Uri packageURI, in IPackageInstallCallback callback);

    /**
     * Attempts to delete a package.  Since this may take a little while, the result will
     * be posted back to the given callback.
     * Protection Level: License
     *
     * @param packageName The name of the package to delete
     */
    void deletePackage(String packageName);

    /**
     * [Notification Controller Functions]
     */
    void cancelNotification(String packageName);
    void setNotificationEnabledSetting(String packageName, boolean enable);
    boolean getNotificationEnabledSetting(String packageName);
    void setNotificationCache(in List<NotificationCacheRecord> cache);

    /**
     * [Interception Controller Functions]
     */
    void enableInterceptionController(boolean enable);
    boolean getInterceptionEnabledSetting();

    /**
     * Sets the firewall rule for application over mobile or Wi-Fi data connection.
     *
     * @param appUid The user id of application
     * @param networkType Specify over mobile or Wi-Fi data connection
     * @param enable Enable or disable firewall rule to restrict application data usage
     */
    void setFirewallPolicy(int appUid, int networkType, boolean enable);

    /**
     * [Radio Controller Functions]
     */

}

