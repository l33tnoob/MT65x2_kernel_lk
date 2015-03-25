/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
package com.mediatek.apst.target.tests;

import android.net.Uri;
import android.test.AndroidTestCase;

import com.mediatek.apst.target.data.proxy.contacts.ContactsProxy;
import com.mediatek.apst.target.data.proxy.sysinfo.SystemInfoProxy;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.util.entity.contacts.Email;
import com.mediatek.apst.util.entity.contacts.RawContact;
import com.mediatek.apst.util.entity.contacts.StructuredName;

import java.util.ArrayList;

public class ContactsProxyTest extends AndroidTestCase {

    ContactsProxy mCP = null;
    SimContact mSim, mSim1, mSim2;

    public static final int SIM = 0;
    /** SIM card 1(for dual-SIM feature). */
    public static final int SIM1 = 1;
    /** SIM card 2(for dual-SIM feature). */
    public static final int SIM2 = 2;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mCP = ContactsProxy.getInstance(getContext());
        mSim = new SimContact("test", "123", SIM, "email");
        mSim1 = new SimContact("test1", "1231", SIM1, "email1");
        mSim2 = new SimContact("test2", "1232", SIM2, "email2");
    }

    @Override
    protected void tearDown() throws Exception {
        mSim = null;
        mSim1 = null;
        mSim2 = null;
        super.tearDown();
    }

    /**
     * 
     */
    public void test_GetMaxRawContactsId() {
        long result = mCP.getMaxRawContactsIdByQuery();
        assertTrue(result > -1);
    }

    /**
     * 
     */
    public void test01_GetSimUri() {
        Uri result = ContactsProxy.getSimUri(SIM);
        assertTrue(result != null);
    }

    /**
     * If SIM card is avaliable, insert a SIM contact.
     */
    public void test02_InsertSimContact() {
        long result;

        if (SystemInfoProxy.getInstance(getContext()).isSimAccessible()) {
            result = mCP.insertSimContact(mSim.mName, mSim.mNumber,
                    mSim.mSlotId);
            assertTrue(result >= 0);
            // int result2 = mCP.deleteSimContact(mSim.mName, mSim.mNumber,
            // mSim.mSlotId);
            // assertTrue(result2 >= 0);
        } else {
            Debugger.logD(new Object[] {}, "SIM isn't accessible");
        }

        if (SystemInfoProxy.isSim1Accessible()) {
            result = mCP.insertSimContact(mSim1.mName, mSim1.mNumber,
                    mSim1.mSlotId);
            assertTrue(result >= 0);
        } else {
            Debugger.logD(new Object[] {}, "SIM1 isn't accessible");
        }

        if (SystemInfoProxy.isSim2Accessible()) {
            result = mCP.insertSimContact(mSim2.mName, mSim2.mNumber,
                    mSim2.mSlotId);
            assertTrue(result >= 0);
        } else {
            Debugger.logD(new Object[] {}, "SIM2 isn't accessible");
        }
    }

    /**
     * Update SIM contact
     */
    public void test03_UpdateSimContact() {
        long insertResult;
        int updateResult;
        if (SystemInfoProxy.getInstance(getContext()).isSimAccessible()) {
            insertResult = mCP.insertSimContact(mSim.mName, mSim.mNumber,
                    mSim.mSlotId);
            // assertTrue(result >= 0);
            if (insertResult >= 0) {
                updateResult = mCP.updateSimContact(mSim.mName, mSim.mNumber,
                        mSim1.mName, mSim1.mNumber, SIM);
                assertTrue(updateResult > 0);
            } else {
                Debugger.logD(new Object[] {}, "Insert sim contact fail!");
            }
        }

        if (SystemInfoProxy.isSim1Accessible()) {
            insertResult = mCP.insertSimContact(mSim.mName, mSim.mNumber,
                    mSim.mSlotId);
            // assertTrue(result >= 0);
            if (insertResult >= 0) {
                updateResult = mCP.updateSimContact(mSim.mName, mSim.mNumber,
                        mSim1.mName, mSim1.mNumber, SIM1);
                assertTrue(updateResult > 0);
            } else {
                Debugger.logD(new Object[] {}, "Insert sim1 contact fail!");
            }
        }

        if (SystemInfoProxy.isSim2Accessible()) {
            insertResult = mCP.insertSimContact(mSim.mName, mSim.mNumber,
                    mSim.mSlotId);
            // assertTrue(result >= 0);
            if (insertResult >= 0) {
                updateResult = mCP.updateSimContact(mSim.mName, mSim.mNumber,
                        mSim1.mName, mSim1.mNumber, SIM2);
                assertTrue(updateResult > 0);
            } else {
                Debugger.logD(new Object[] {}, "Insert sim2 contact fail!");
            }
        }
    }

    /**
     * Delete a SIM contact
     */
    public void test04_DeleteSimContact() {

        long insertResult;
        int updateResult;
        if (SystemInfoProxy.getInstance(getContext()).isSimAccessible()) {
            insertResult = mCP.insertSimContact(mSim.mName, mSim.mNumber,
                    mSim.mSlotId);
            // assertTrue(result >= 0);
            if (insertResult >= 0) {
                updateResult = mCP.deleteSimContact(mSim.mName, mSim.mNumber,
                        SIM);
                assertTrue(updateResult > 0);
            } else {
                Debugger.logD(new Object[] {}, "Insert SIM contact fail!");
            }
        }

        if (SystemInfoProxy.isSim1Accessible()) {
            insertResult = mCP.insertSimContact(mSim.mName, mSim.mNumber,
                    mSim.mSlotId);
            // assertTrue(result >= 0);
            if (insertResult >= 0) {
                updateResult = mCP.deleteSimContact(mSim.mName, mSim.mNumber,
                        SIM1);
                assertTrue(updateResult > 0);
            } else {
                Debugger.logD(new Object[] {}, "Insert SIM1 contact fail!");
            }
        }

        if (SystemInfoProxy.isSim2Accessible()) {
            insertResult = mCP.insertSimContact(mSim.mName, mSim.mNumber,
                    mSim.mSlotId);
            // assertTrue(result >= 0);
            if (insertResult >= 0) {
                updateResult = mCP.deleteSimContact(mSim.mName, mSim.mNumber,
                        SIM2);
                assertTrue(updateResult > 0);
            } else {
                Debugger.logD(new Object[] {}, "Insert SIM2 contact fail!");
            }
        }
    }

    /**
     * Insert a contact without data.
     */

    public void test05_InsertContact() {
        RawContact rawContact = new RawContact();
        rawContact.setSourceLocation(RawContact.SOURCE_PHONE);
        rawContact.setDirty(false);
        rawContact.setDisplayName("DisplayName");
        rawContact.setEmails(new ArrayList<Email>());
        long result = mCP.insertContact(rawContact, false);
        assertTrue(result > 0);
    }

    /**
     * Insert a contact with data.
     */
    public void test06_InsertContactWithData() {
        RawContact rawContact = new RawContact();
        rawContact.setSourceLocation(RawContact.SOURCE_PHONE);
        rawContact.setDirty(false);
        rawContact.setDisplayName("DisplayName");
        Email email = new Email();
        email.setData("8888888");
        email.setLabel("email01");
        ArrayList<Email> emails = new ArrayList<Email>(1);
        emails.add(email);
        rawContact.setEmails(emails);
        ArrayList<StructuredName> structuredNames = new ArrayList<StructuredName>(
                1);
        StructuredName structuredName = new StructuredName();
        structuredName.setDisplayName("displayName");
        structuredName.setFamilyName("familyName");
        structuredName.setGivenName("givenName");
        rawContact.setNames(structuredNames);
        long result = mCP.insertContact(rawContact, true);
        assertTrue(result > 0);
    }

    class SimContact {
        String mName;
        String mNumber;
        int mSlotId;
        String mEmail;

        public SimContact(String name, String number, int slotId, String email) {
            mName = name;
            mNumber = number;
            mSlotId = slotId;
            mEmail = email;
        }
    }
}
