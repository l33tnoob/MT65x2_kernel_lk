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

package com.mediatek.rcse.test.receiver;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.test.InstrumentationTestCase;
import android.view.View;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.receiver.PackageReceiver;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;

/**
 * The class is used to test PackageReceiver
 */
public class PackageReceiverTest extends InstrumentationTestCase {
    private static final String TAG = "PackageReceiverTest";
    private Context mContext = null;
    private PackageReceiver mReceiver = null;
    
    private static final String INVALID_ACTION = "com.invalid.action";
    private static final String INVALID_PACKAGE_NAME = "com.mediatek.invalid";
    private static final int THREAD_SLEEP_PERIOD = 20;
    private static final long TIME_OUT = 5000;

    @Override
    protected void setUp() throws Exception {
        Logger.d(TAG, "setUp() entry");
        super.setUp();
        mContext = getInstrumentation().getTargetContext();
        mReceiver = new PackageReceiver();
        RcsSettings.createInstance(mContext);
        Logger.d(TAG, "setUp() entry");
    }

    /**
     * Test case for onReceive()
     */
    public void testCase1_OnReceive() {
        Logger.d(TAG, "testCase1_OnReceive() entry");
        // Test invalid action
        RcsSettings.getInstance().writeParameter(RcsSettingsData.BLOCK_XCAP_OPERATION,RcsSettingsData.TRUE);
        Intent intent = new Intent();
        intent.setAction(INVALID_ACTION);
        intent.setData(Uri.parse(INVALID_PACKAGE_NAME));
        mReceiver.onReceive(mContext, intent);
        waitWriteParameter(true);
        // Test valid action
        intent = null;
        intent = new Intent();
        RcsSettings.getInstance().writeParameter(RcsSettingsData.BLOCK_XCAP_OPERATION,RcsSettingsData.TRUE);
        intent.setAction(Intent.ACTION_PACKAGE_REPLACED);
        intent.setData(Uri.parse(INVALID_PACKAGE_NAME));
        mReceiver.onReceive(mContext, intent);
        waitWriteParameter(true);
        intent = null;
        intent = new Intent();
        RcsSettings.getInstance().writeParameter(RcsSettingsData.BLOCK_XCAP_OPERATION,RcsSettingsData.TRUE);
        intent.setAction(Intent.ACTION_PACKAGE_REPLACED);
        intent.setData(Uri.parse(mContext.getApplicationInfo().packageName));
        mReceiver.onReceive(mContext, intent);
        waitWriteParameter(false);
        Logger.d(TAG, "testCase1_OnReceive() exit");
    }
    
    private void waitWriteParameter(boolean expect) {
        long startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                String retVal = RcsSettings.getInstance().readParameter(
                        RcsSettingsData.BLOCK_XCAP_OPERATION);
                Logger.d(TAG, "retVal = " + retVal);
                try {
                    if (retVal == null) {
                        fail();
                    } else {
                        if (retVal.equals(String.valueOf(expect))) {
                            break;
                        } else {
                            Thread.sleep(THREAD_SLEEP_PERIOD);
                        }
                    }
                } catch (InterruptedException e) {
                    this.fail(e.toString());
                }
            }
        }
    }
}
