/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.op.telephony.cat;

import android.os.Handler;
import android.os.SystemProperties;

import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.cat.CatLog;
import com.android.internal.telephony.cat.CatService;

public class CatOpAppInterfaceImp {
    
    private static CatService mCatService = null;
    private static CatOpAppInterfaceImp mInstance = null;

    public CatOpAppInterfaceImp(CatService cat) {
        mCatService = cat;
        mInstance = this;
    }

    public void updateMenuTitleFromEf(String appName) {
        if (isOrangeSupport()) {
            String newAppName = appName;
            if (null == newAppName || newAppName.isEmpty() || 0 == newAppName.length()) {
                CatLog.d(this, "appName is invalid valule");
                // TODO: read SIM EF(Elementary File) SUME(Setup Menu
                // Entity) Address 6F54
                newAppName = getMenuTitleFromEf();
            }

            if (null != newAppName && !newAppName.isEmpty()) {
                CatLog.d(this, "update appName: " + newAppName);
                updateAppName(newAppName);
            }
        }
    }
    private String getMenuTitleFromEf() {
        String title = null;
        IccRecords ic = null;
        if (mCatService != null) {
            ic = mCatService.getIccRecords();
        }
        if (ic != null) {
            title = ic.getMenuTitleFromEf();
        }
        CatLog.d("[OP]", "mCatService: " + ((mCatService != null)? 1: 0) + ", ic: " + ((ic != null)? 1 : 0) + ", Title: " + ((title != null)? title : "title is null"));
        return title;
    }

    public static CatOpAppInterfaceImp getInstance() {
        return mInstance;
    }

    // MTK_OP03_PROTECT_START
    /* Orange customization begin */
    private boolean isOrangeSupport() {
        String optr = SystemProperties.get("ro.operator.optr");
        if (optr != null && "OP03".equals(optr)) {
            CatLog.d(this, "isOrangeSupport is true");
            return true;
        }
        CatLog.d(this, "isOrangeSupport is false");
        return false;
    }

    /*
     * Set system property to notify ContextImpl To update Stk app name, we need
     * to co-work with framework:
     * frameworks/base/core/java/android/app/ContextImpl.java
     */
    final static String STK_TITLE_KEY = "gsm.setupmenu.title";
    final static String STK_TITLE_KEY2 = "gsm.setupmenu.title2";

    private void updateAppName(String appName) {
        CatLog.d(this, "set menu title in SystemProperties to " + appName);
        SystemProperties.set(STK_TITLE_KEY, appName);
    }

    /* Orange customization end */
    // MTK_OP03_PROTECT_END
}

