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

package com.mediatek.apst.target.tests;

import android.content.ContentProviderResult;
import android.content.Context;
import android.content.OperationApplicationException;
import android.os.RemoteException;
import android.test.AndroidTestCase;

import com.mediatek.android.content.ContentProviderOperationBatch;
import com.mediatek.android.content.DefaultUpdateBatchHelper;
import com.mediatek.apst.target.data.proxy.ObservedContentResolver;
import com.mediatek.apst.target.data.proxy.contacts.ContactsOperationBatch;

public class DefaultUpdateBatchHelperTest extends AndroidTestCase {
    private BatcherHelper mBatcherHelper;
    private Context mContext;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getContext();
        mBatcherHelper = new BatcherHelper(new ContactsOperationBatch(
                new ObservedContentResolver(mContext.getContentResolver())));
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01_getName() {
        assertEquals("DefaultUpdateBatchHelper", mBatcherHelper.getName());
    }

    public void test02_run() {
        mBatcherHelper.run(10000);
        assertNotNull(mBatcherHelper.getResults());
    }
    
    public void test03_onOperationResult() {
        ContentProviderResult[] result = null;
        try {
            result = mBatcherHelper.onApply(new ContactsOperationBatch(
                    new ObservedContentResolver(mContext.getContentResolver())));
        } catch (RemoteException e) {
            fail(e.getMessage());
        } catch (OperationApplicationException e) {
            fail(e.getMessage());
        }
        ContentProviderResult providerResult;
        for(int i = 0; i < result.length; i++) {
            providerResult = result[i];
            mBatcherHelper.onOperationResult(providerResult, i);
        }
    }

    class BatcherHelper extends DefaultUpdateBatchHelper {

        public BatcherHelper(ContentProviderOperationBatch opBatch) {
            super(opBatch);
        }

        @Override
        public void onAppend(ContentProviderOperationBatch opBatch,
                int appendPosition) {
            long[] ids = new long[10000];
            for (int i = 0; i < 10000; i++) {
                ids[i] = i;
            }
            ((ContactsOperationBatch) opBatch).appendRawContactDelete(
                    ids[appendPosition], true);

        }

        @Override
        public ContentProviderResult[] onApply(
                ContentProviderOperationBatch opBatch) throws RemoteException,
                OperationApplicationException {
            return ((ContactsOperationBatch) opBatch).apply();
        }

    }

}
