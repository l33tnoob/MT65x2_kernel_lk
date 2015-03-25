/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.android.content;

import android.content.ContentProviderResult;

import com.mediatek.apst.target.util.Debugger;

public abstract class DefaultDeleteBatchHelper extends OperationBatchHelper {
    private boolean[] mDeletionResults;

    /**
     * @param opBatch
     *            The ContentProviderOperationBatch to used for bulk deleting.
     */
    public DefaultDeleteBatchHelper(final ContentProviderOperationBatch opBatch) {
        super(opBatch);
    }

    /**
     * @return a array of whether operations succeeded.
     */
    public boolean[] getResults() {
        return mDeletionResults;
    }

    /**
     * Override it. Provide a class name/tag for log.
     * 
     * @return a class name.
     */
    public String getName() {
        return "DefaultDeleteBatchHelper";
    };

    /**
     * @param batchSize
     *            the number of batch operations.
     * @see com.mediatek.android.content.OperationBatchHelper#run(int)
     */
    public void run(final int batchSize) {
        mDeletionResults = new boolean[batchSize];
        super.run(batchSize);
    }

    @Override
    /*
     * (non-Javadoc)
     * 
     * @see
     * com.mediatek.android.content.OperationBatchHelper#onOperationResult(android
     * .content.ContentProviderResult, int)
     */
    public void onOperationResult(final ContentProviderResult opResult,
            final int executedPosition) {
        if (null == opResult) {
            Debugger.logW(getName(), "onOperationResult", new Object[] {
                    opResult, executedPosition },
                    "ContentProviderResult is null!");
            mDeletionResults[executedPosition] = false;
        } else if (1 == opResult.count) {
            mDeletionResults[executedPosition] = true;
        } else {
            Debugger.logW(getName(), "onOperationResult", new Object[] {
                    opResult, executedPosition },
                    "Operation failed, affected rows count=" + opResult.count);
            mDeletionResults[executedPosition] = false;
        }
    }
}
