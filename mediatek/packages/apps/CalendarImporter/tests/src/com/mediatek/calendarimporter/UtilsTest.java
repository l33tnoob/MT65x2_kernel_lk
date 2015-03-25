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

package com.mediatek.calendarimporter;

import java.io.File;

import com.mediatek.calendarimporter.utils.Utils;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.content.Context;
import android.test.AndroidTestCase;

import com.mediatek.calendarimporter.utils.Utils;

public class UtilsTest extends AndroidTestCase {
    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test1_hasAccount() {
        Account accounts[] = AccountManager.get(getContext().getApplicationContext()).getAccounts();
        // for (Account account : accounts) {
        // AccountManager.get(getContext()).removeAccount(account, null, null);
        // }
        if (accounts.length == 0) {
            assertFalse(Utils.hasExchangeOrGoogleAccount(getContext()));
        } else {
            int i = 0;
            for (Account account : accounts) {
                i++;
                if (account.type.equals(Utils.MAIL_TYPE_EXCHANGE) || account.type.equals(Utils.MAIL_TYPE_GOOGLE)) {

                    assertTrue(Utils.hasExchangeOrGoogleAccount(getContext()));
                    assertTrue(Utils.isExchangeOrGoogleAccount(account));
                    break;
                }
                if (i == accounts.length) {
                    assertFalse(Utils.hasExchangeOrGoogleAccount(getContext()));
                }
            }
        }
    }

    public void test2_MockAccountSytemService() {
        Context context = getContext().getApplicationContext();
        TestUtils.addMockAccount(context, "test");
        Account[] accounts = AccountManager.get(context).getAccounts();
        assertFalse(accounts.length == 0);
        accounts = AccountManager.get(context).getAccountsByType(null);
        assertFalse(accounts.length == 0);
        TestUtils.removeTestAccounts(context);
        accounts = AccountManager.get(context).getAccounts();
        assertTrue(accounts.length == 0);
    }

    /**
     * add to test the add file function in TestUtils
     */
    public void test3_addFile() {
        File file = TestUtils.addFile("test.vcs", "test");
        assertTrue(file.length() > 0);
        TestUtils.removeFile(file);
    }

}
