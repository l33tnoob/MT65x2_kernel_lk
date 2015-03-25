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
 

package com.mediatek.common.mom;

import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import java.util.List;


/**
  * All of the APIs in IMobileManager will be protected by license checking.
  * Total 2 protection level for each API:
  * - License checking:    Only specified authorized certificate in the application will be accepted.
  * - Attachment checking: Based on LICENSE checking, this kind of APIs can only be accessed
  *                        after attaching successfully.
  * Protection level of each APIs can be found in comment with Tag "Protection Level".
  */
public interface IMobileManager {

    // This version name should match the result returned by getVersionName().
    public static final String VERSION_NAME = "MoMS.3.0.0";

    /**
     * The status(mode) for each permissions.
     */
    // Ok to use the permission.
    public static final int PERMISSION_STATUS_GRANTED = 0;
    // Forbidden to use the permission. Notifing this event to user is recommand.
    public static final int PERMISSION_STATUS_DENIED = 1;
    // Ask user to grant the permission or not.
    public static final int PERMISSION_STATUS_CHECK = 2;

    /**
     * The attribute for each permissions.
     */
    public static final int PERMISSION_STATUS_NONE = 0;
    // For permissions defined by android
    public static final int PERMISSION_FLAG_NORMAL = 1;
    // For permissions defined by MoMS, and each 
    public static final int PERMISSION_FLAG_SUB = 1 << 1;
    // For permission must checked by user for each time.
    public static final int PERMISSION_FLAG_USERCONFIRM = 1 << 2;
    // For permission can be accessed with system authority.
    public static final int PERMISSION_FLAG_SYSTEM = 1 << 3;
    public static final int PERMISSION_FLAG_NONE = 0;
    public static final int PERMISSION_FLAG_ALL = PERMISSION_FLAG_NORMAL |
                                                  PERMISSION_FLAG_SUB |
                                                  PERMISSION_FLAG_USERCONFIRM |
                                                  PERMISSION_FLAG_SYSTEM;

    /**
     * The bundle extra definitions
     */
    // The bundle extra definition for Notification
    public static final String OPTION_NOTIFICATION_PKG = "notificationPkg";
    public static final String OPTION_NOTIFICATION_ID = "notificationID";
    public static final String OPTION_NOTIFICATION_TITLE = "notificationTitle";
    public static final String OPTION_NOTIFICATION_CONTENT = "notificationContent";

    //The bundle extra definition for Call Interception
    public static final String PARAMETER_PHONENUMBER = "phoneNumber";
    public static final String PARAMETER_CALLTYPE = "callType";
    public static final String PARAMETER_SLOTID = "slotId";

    // The bundle extra definition for Message Intercept
    public static final String SMS_MESSAGE_INTENT = "intent";
    public static final String SMS_MESSAGE_SIMID = "simId";
    public static final String SMS_MESSAGE_FORMAT = "format";
    public static final String SMS_MESSAGE_RECIPIENT = "recipient";
    public static final String SMS_MESSAGE_TEXT = "text";
    public static final String SMS_MESSAGE_MULTIPARTTEXT = "multiparttext";
    public static final String SMS_MESSAGE_DATA = "data";
    public static final String SMS_MESSAGE_MULTIPARTDATA = "multipartdata";

    /**
     * Broadcast of MoMS
     */
    public static final String ACTION_PACKAGE_CHANGE = "mom.action.PACKAGE_CHANGE";
    public static final String ACTION_USER_CHANGE = "mom.action.USER_CHANGE";
    public static final String ACTION_PERM_MGR_CHANGE = "mom.action.PERM_MGR_CHANGE";

    public static final String ACTION_EXTRA_STATUS = "mom.action.extra.status";
    public static final String ACTION_EXTRA_USER = "mom.action.extra.user";
    public static final String ACTION_EXTRA_PACKAGE = "mom.action.extra.package";
    public static final String ACTION_EXTRA_PACKAGE_LIST = "mom.action.extra.package.list";
    public static final String ACTION_EXTRA_UID = "mom.action.extra.uid";

    // Package Status
    public static final int PACKAGE_ADDED = 0;
    public static final int PACKAGE_REMOVED = 1;
    public static final int PACKAGE_UPDATED = 2;
    public static final int PACKAGE_EXT_AVALIABLE = 3;
    public static final int PACKAGE_EXT_UNAVALIABLE = 4;

    // User Status
    public static final int USER_ADDED = 0;
    public static final int USER_REMOVED = 1;
    public static final int USER_SWITCHED = 2;

    // Permission Manager Status
    public static final int MGR_ATTACHED = 0;
    public static final int MGR_DETACHED = 1;

    // Controller ID
    public static final int CONTROLLER_PERMISSION = 0;
    public static final int CONTROLLER_PACKAGE = 1;
    public static final int CONTROLLER_RECEIVER = 2;
    public static final int CONTROLLER_NOTIFICATION = 3;
    public static final int CONTROLLER_FIREWALL = 4;
    public static final int CONTROLLER_MESSAGE_INTERCEPT = 5;
    public static final int CONTROLLER_CALL = 6;
    public static final int CONTROLLER_LICENSE = 7;
    public static final int CONTROLLER_INTERCEPTION = 8;

    // Timeout for triggering a callback
    public static final int TRIGGER_LISTENER_TIMEOUT = 35*1000; // 35 sec

    // An extra information to start service by MoMS
    public static final String START_BY_MOMS = "start_service_by_moms";


    // =============[Utility Functions]==============
    /**
     * Get the version name of MoMS, the value should
     * match the value of IMobileManager.VERSION_NAME.
     * Protection Level: License
     *
     * @return Returns version name of MoMS.
     */
    public String getVersionName();

    /**
     * Clear all settings to MoMS except permission controller.
     * Protection Level: License
     */
    public void clearAllSettings();

    /**
     * Clear settings to MoMS for a specified package
     * except Permission Controller.
     * Protection Level: License
     *
     * @param packageName The package to be cleared.
     */
    public void clearPackageSettings(String packageName);


    // =============[Permission Controller Functions]==============
    /**
     * For each app wants to use the APIs in permission controller must
     * attach to MoMS at first, otherwise, SecurityException will be thrown.
     * Protection Level: License
     *
     * @param callback The callback functions handle the connection events.
     *                 Please reference to IMobileConnectionCallback.aidl.
     * @return         Returns true when attached successfully.
     */
    public boolean attach(IMobileConnectionCallback callback);

    /**
     * Invoke this function if there is no need to use APIs in permission controller.
     * Protection Level: Attachment
     */
    public void detach();

    /**
     * Register a listener to monitor permission checking.
     * Protection Level: Attachment
     *
     * @param listener The callback function will be triggered during checking.
     */
    public void registerPermissionListener(IPermissionListener listener);

    /**
     * To enable/disable the permission controller.
     * Protection Level: Attachment
     *
     * @param enable To enable it or not.
     */
    public void enablePermissionController(boolean enable);

    /**
     * To get all the packages installed except "system" applications.
     * Protection Level: License
     *
     * @return Returns a list of PackageInfo.
     */
    public List<PackageInfo> getInstalledPackages();

    /**
     * To get all "granted" permissions of a given package.
     * Protection Level: License
     *
     * @param packageName The name of the package to be checked.
     * @return            Returns a list of PackageInfo.
     */
    public List<Permission> getPackageGrantedPermissions(String packageName);

    /**
     * Set the status to the permission with a given record.
     * Protection Level: Attachment
     *
     * @param record The setting data, please refers to PermissionRecord.java.
     */
    public void setPermissionRecord(PermissionRecord record);

    /**
     * Set the status to the permissions with a list of given records.
     * Protection Level: Attachment
     *
     * @param record The setting list.
     */
    public void setPermissionRecords(List<PermissionRecord> records);

    /**
     * Set the status to the permissions with a list of given records
     * with reseting all data to GRANTED at first.
     * Protection Level: Attachment
     *
     * @param cache The setting list.
     */
    public void setPermissionCache(List<PermissionRecord> cache);

    /**
     * [Receiver Controller Functions]
     * Protection Level: License
     */
    /**
     * Set the enabled setting for a package to receive BOOT_COMPLETED
     * Protection Level: License
     *
     * @param packageName The package to enable
     * @param enable The new enabled state for the package.
     */
    public void setBootReceiverEnabledSetting(String packageName, boolean enable);

    public void setBootReceiverEnabledSettings(List<ReceiverRecord> list);

    public List<ReceiverRecord> getBootReceiverList();

    /**
     * Return the the enabled setting for a package that receives BOOT_COMPLETED
     * Protection Level: License
     *
     * @param packageName The package to retrieve.
     * @return enable Returns the current enabled state for the package.
     */
    public boolean getBootReceiverEnabledSetting(String packageName);

    /**
     * [Package Controller Functions]
     * Protection Level: License
     */
    /**
     * Forcestop the specified package.
     * Protection Level: License
     *
     * @param packageName The name of the package to be forcestoped.
     */
    public void forceStopPackage(String packageName);

    /**
     * Install a package. Since this may take a little while, the result will
     * be posted back to the given callback.
     * Protection Level: License
     *
     * @param packageURI The location of the package file to install.  This can be a 'file:' or a 'content:' URI.
     * @param callback An callback to get notified when the package installation is complete.
     */
    public void installPackage(Uri packageURI, IPackageInstallCallback callback);

    /**
     * Attempts to delete a package.  Since this may take a little while, the result will
     * be posted back to the given callback.
     * Protection Level: License
     *
     * @param packageName The name of the package to delete
     */
    public void deletePackage(String packageName);

    // =============[Notification Controller Functions]==============
    /**
     * Clear notifications for a specified package.
     * Protection Level: License
     *
     * @param packageName The name of the package to be cleared.
     */
    public void cancelNotification(String packageName);

    /**
     * To enable/disable the notification for a specified package.
     * Protection Level: License
     *
     * @param packageName The name of the package to be set.
     * @param enable To enable it or not.
     */
    public void setNotificationEnabledSetting(String packageName, boolean enable);

    /**
     * Get the notification enabled status for a specified package.
     * Protection Level: License
     *
     * @param packageName The package to be cleared.
     * @return         Return true when notification is enabled, otherwise false.
     */
    public boolean getNotificationEnabledSetting(String packageName);

    /**
     * Register a listener to monitor notification checking.
     * Protection Level: License
     *
     * @param listener The callback function will be triggered during checking.
     */
    public void registerNotificationListener(INotificationListener listener);

    /**
     * Set the notification enable/disable cache status with a list of given records.
     * Protection Level: License
     *
     * @param cache The setting list.
     */
    public void setNotificationCache(List<NotificationCacheRecord> cache);

    /**
     * [Interception Controller Functions]
     */
    public void enableInterceptionController(boolean enable);
    public void registerCallInterceptionListener(ICallInterceptionListener listener);



    /**
     * Sets the firewall rule for application over mobile or Wi-Fi data connection.
     * Protection Level: License
     *
     * @param appUid Specify one application by UID
     * @param networkType Specify over mobile or Wi-Fi data connection
     * @param enable Enable or disable firewall rule to restrict application data usage
     */
    public void setFirewallPolicy(int appUid, int networkType, boolean enable);

    /**
     * [Message Intercept Controller Functions]
     */
    public void registerMessageInterceptListener(IMessageInterceptListener listener);
}
