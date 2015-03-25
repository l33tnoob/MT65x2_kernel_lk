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

package com.mediatek.smsreg;

import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;

import com.mediatek.common.dm.DmAgent;
import com.mediatek.xlog.Xlog;

interface IInfoPersistentor {
    String getSavedIMSI();

    void setSavedIMSI(String regIMSI);
}

public class InfoPersistentor implements IInfoPersistentor {
    private String mTAG = "SmsReg/InfoPersistentor";
    private DmAgent mAgent = null;

    InfoPersistentor() {
        if (mAgent == null) {
            Xlog.i(mTAG, "get the agent...");
            IBinder binder = ServiceManager.getService("DmAgent");
            if (binder == null) {
                Xlog.e(mTAG, "get DmAgent fail! binder is null!");
                return;
            }
            mAgent = DmAgent.Stub.asInterface(binder);
        }
    }

    public String getSavedIMSI() {
        if (mAgent == null) {
            Xlog.e(mTAG, "get IMSI failed, agent is null!");
            return null;
        }

        String savedIMSI = null;
        try {
            byte[] imsiByte = mAgent.readImsi();
            if (imsiByte != null) {
                savedIMSI = new String(imsiByte);
            }
        } catch (RemoteException e) {
            Xlog.e(mTAG, "get IMSI failed, readImsi failed!");
            e.printStackTrace();
        }
        Xlog.i(mTAG, "Get savedIMSI = [" + savedIMSI + "]");
        return savedIMSI;
    }

    public void setSavedIMSI(String regIMSI) {
        if (mAgent == null) {
            Xlog.e(mTAG, "save IMSI failed, agent is null!");
            return;
        }
        try {
            mAgent.writeImsi(regIMSI.getBytes());
        } catch (RemoteException e) {
            Xlog.e(mTAG, "save IMSI failed, writeImsi failed!");
            e.printStackTrace();
        }
        Xlog.i(mTAG, "save IMSI [" + regIMSI + "]");
    }

    public int getSavedCTA() {
        if (mAgent == null) {
            Xlog.e(mTAG, "get CTA failed, agent is null!");
            return 0;
        }
        int savedCTA = 0;
        try {
            byte[] ctaBytes = mAgent.getRegisterSwitch();
            savedCTA = Integer.parseInt(ctaBytes == null ? null : new String(
                    ctaBytes));
        } catch (RemoteException e) {
            Xlog.e(mTAG, "get cta cmcc switch failed, readCTA failed!");
            e.printStackTrace();
        } catch (NumberFormatException e) {
            Xlog.e(mTAG, "number format exception. ", e);
        }
        Xlog.i(mTAG, "Get savedCTA = [" + savedCTA + "]");
        return savedCTA;
    }

    private void setSavedCTA(String cta) {
        if (mAgent == null) {
            Xlog.e(mTAG, "save CTA switch value failed, agent is null!");
            return;
        }
        try {
            mAgent.setRegisterSwitch(cta.getBytes());
        } catch (RemoteException e) {
            Xlog.e(mTAG, "save CTA switch failed, writeCTA failed!");
            e.printStackTrace();
        }
        Xlog.i(mTAG, "save CTA [" + cta + "]");
    }
}
