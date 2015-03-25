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

package com.mediatek.rcse.service;

import android.content.Context;
import android.os.Environment;

import com.mediatek.rcse.api.Logger;

/**
 * This class will help to manager file transfer capability for sdcard status
 * and network status
 */
public class FileTransferCapabilityManager {
    private static final String TAG = "FileTransferCapabilityManager";

    /**
     * Set the file transfer capability
     * 
     * @param context Current context instance
     * @param ftCapability file transfer capability value
     */
    public static void setFileTransferCapability(Context context, boolean ftCapability) {
        Logger.d(TAG, "setFileTransferCapability() ftCapability is " + ftCapability);
        final ExchangeMyCapability exchangeMyCapability = ExchangeMyCapability.getInstance(context);
        if (null == exchangeMyCapability) {
            Logger.e(TAG, "setFileTransferCapability() ExchangeMyCapability instance is null");
            return;
        }
        exchangeMyCapability.notifyCapabilityChanged(
                ExchangeMyCapability.FILE_TRANSFER_CAPABILITY_CHANGE, ftCapability);
        Logger.d(TAG, "setFileTransferCapability() exit");
    }

    /**
     * Get file transfer capbility from sdcard and network status
     * 
     * @return file transfer capbility
     */
    public static boolean isFileTransferCapabilitySupported() {
        Logger.d(TAG, "isFileTransferCapabilitySupported() entry");
        boolean ftCapability = SdcardHelper.isSdcardSupportedFileTransfer();
        Logger.d(TAG, "isFileTransferCapabilitySupported() exit, ftCapability is " + ftCapability);
        return ftCapability;
    }

    /**
     * This class will handle sdcard status for file transfer
     */
    private static class SdcardHelper {
        private static final String TAG = "SdcardHelper";

        /**
         * Check if sdcard support file transfer
         * 
         * @return true if support
         */
        public static boolean isSdcardSupportedFileTransfer() {
            Logger.d(TAG, "isSdcardSupportedFileTransfer()");
            boolean isSupported = isSdcardExist();
            Logger.d(TAG, "isSdcardSupportedFileTransfer() isSupported is " + isSupported);
            return isSupported;
        }

        private static boolean isSdcardExist() {
            Logger.d(TAG, "isSdcardExist() entry");
            boolean isExist = Environment.getExternalStorageState().equals(
                    Environment.MEDIA_MOUNTED);
            Logger.d(TAG, "isSdcardExist() exit isExist is " + isExist);
            return isExist;
        }
    }
}
