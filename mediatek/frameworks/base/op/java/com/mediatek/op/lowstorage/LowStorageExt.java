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

package com.mediatek.op.lowstorage;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.UserHandle;
import android.util.Slog;
import com.mediatek.common.lowstorage.ILowStorageExt;


public class LowStorageExt implements ILowStorageExt 
{
	private static final String TAG = "LowStorageExt";
	private static final int DEFAULT_THRESHOLD_PERCENTAGE = 5;
    private static final int DEFAULT_THRESHOLD_MAX_BYTES = 0;
    private Context mContext;
    private long mTotalMemory = 0;
    private long mMemLowThreshold;
    private boolean mLowMemFlag = false;
    private Intent mStorageLowIntent;
    private Intent mStorageNotLowIntent;
    
	public void init(Context context, long total_size) {
/*        
        mContext = context;
        mTotalMemory = total_size;
        
        mStorageLowIntent = new Intent("android.intent.action.OP_DEVICES_STORAGE_LOW");
        mStorageLowIntent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
        mStorageNotLowIntent = new Intent("android.intent.action.OP_DEVICES_STORAGE_NOT_LOW");
        mStorageNotLowIntent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
        
        mMemLowThreshold = getMemThreshold();
        Slog.v(TAG, "OP threshold: " + mMemLowThreshold);
*/        
    }    
    
    private long getMemThreshold() {        
        long value = (DEFAULT_THRESHOLD_PERCENTAGE * mTotalMemory)/100;
        long maxValue = DEFAULT_THRESHOLD_MAX_BYTES;
        //evaluate threshold value
        return value < maxValue ? value : maxValue;
    }    
    
    public void checkStorage(long size) {
/*		
        Slog.v(TAG, "OP threshold: " + mMemLowThreshold);
        if (size < mMemLowThreshold) {
                Slog.v(TAG, "Operator storage low, storage left: " + size);
                if (!mLowMemFlag) {
                    sendLowNotification();
                    mLowMemFlag = true;
                }
        } else {
            if (mLowMemFlag) {
            	Slog.v(TAG, "Cancel operator storage low, storage left: " + size);
                cancelLowNotification();
                mLowMemFlag = false;
            }
        }
*/        
    }
    
    /**
     * Send a OP low storage intent.
     */
    private final void sendLowNotification() {
        Slog.i(TAG, "Sending OP low memory intent.");
        mContext.sendStickyBroadcastAsUser(mStorageLowIntent, UserHandle.ALL);
    }

    /**
     * Send a OP not low storage intent.
     */
    private final void cancelLowNotification() {
        Slog.i(TAG, "Sending OP not low memory intent.");
        mContext.sendBroadcastAsUser(mStorageNotLowIntent, UserHandle.ALL);
    }
}
