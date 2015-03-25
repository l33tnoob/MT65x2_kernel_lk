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

import android.os.Bundle;
import com.mediatek.common.mom.PermissionRecord;

/**
 * The interface is designed for listening permission related events,
 * and can register callback function through
 * registerPermissionListener(IPermissionListener listener) in MobileManagerService.
 */
interface IPermissionListener {
    /**
     * The callback will be triggered when monitored API is invoked.
     * 
     * @param record The permission record for this checking.
     *               mPackageName: The package invokes the API.
     *               mPermissionName: The permission binding to the API.
     *               mStatus: The status of permission when checking.
     *                        MobileManager.PERMISSION_STATUS_CHECK: The checking requires user's confirmation.
     *                        MobileManager.PERMISSION_STATUS_DENIED: The API is revoked for this package.
     *                        MobileManager.PERMISSION_STATUS_GRANTED won't trigger this function.
     * @param flag The attributes of the permission.
     *             MobileManager.PERMISSION_FLAG_USERCONFIRM: This permission always requires user's confirmation.
     * 
     * @param uid The user id of the package.
     * @param data The addition information for checking
     * @return Returns true when user allows this operation,
     *         otherwise, false should be returned.
     */
    boolean onPermissionCheck(in PermissionRecord record, int flag, int uid, in Bundle data);
    
    /**
     * The callback will be triggered when a permission had been revoked/granted
     * to a package. This will be triggered only by "system" authority user.
     * 
     * @param record The permission record for this checking.
     *               mPackageName: The package invokes the API.
     *               mPermissionName: The permission binding to the API.
     */
    void onPermissionChange(in PermissionRecord record);
}

