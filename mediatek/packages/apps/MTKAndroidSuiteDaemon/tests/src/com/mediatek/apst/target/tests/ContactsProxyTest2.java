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

import android.content.Context;
import android.test.AndroidTestCase;
import android.util.Log;

import com.mediatek.apst.target.data.proxy.IRawBlockConsumer;
import com.mediatek.apst.target.data.proxy.contacts.ContactsProxy;
import com.mediatek.apst.target.data.proxy.contacts.USIMUtils;
import com.mediatek.apst.target.data.proxy.sysinfo.SystemInfoProxy;
import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.util.entity.DataStoreLocations;
import com.mediatek.apst.util.entity.DatabaseRecordEntity;
import com.mediatek.apst.util.entity.contacts.BaseContact;
import com.mediatek.apst.util.entity.contacts.Email;
import com.mediatek.apst.util.entity.contacts.Group;
import com.mediatek.apst.util.entity.contacts.GroupMembership;
import com.mediatek.apst.util.entity.contacts.Im;
import com.mediatek.apst.util.entity.contacts.Nickname;
import com.mediatek.apst.util.entity.contacts.Note;
import com.mediatek.apst.util.entity.contacts.Organization;
import com.mediatek.apst.util.entity.contacts.Phone;
import com.mediatek.apst.util.entity.contacts.Photo;
import com.mediatek.apst.util.entity.contacts.RawContact;
import com.mediatek.apst.util.entity.contacts.StructuredName;
import com.mediatek.apst.util.entity.contacts.StructuredPostal;
import com.mediatek.apst.util.entity.contacts.Website;

import java.nio.ByteBuffer;
import java.util.ArrayList;

public class ContactsProxyTest2 extends AndroidTestCase {
    private static final String TAG = "ContactsProxyTest2";

    private Context mContext;
    private ContactsProxy mContactsProxy;
    private SimContact mSimContact = new SimContact("test", "13565985685",
            DataStoreLocations.SIM, "test@126.com");
    private SimContact mSimContact1 = new SimContact("test1", "13662415658",
            DataStoreLocations.SIM1, "test@126.com");
    private SimContact mSimContact2 = new SimContact("test2", "13845985647",
            DataStoreLocations.SIM2, "test@163.com");
    private SimContact mSimNewContact = new SimContact("newTest",
            "15832659858", DataStoreLocations.SIM, "newTest@126.com");

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getContext();
        mContactsProxy = ContactsProxy.getInstance(mContext);
    }

    @Override
    protected void tearDown() throws Exception {

        super.tearDown();
    }

    public void test01_getInstance() {
        assertNotNull(mContactsProxy);
    }

    /**
     * int slotId SIM.
     */
    public void test02_getSimUri() {
        assertNotNull(ContactsProxy.getSimUri(DataStoreLocations.SIM));
    }

    /**
     * int slotId SIM1.
     */
    public void test03_getSimUri() {
        assertNotNull(ContactsProxy.getSimUri(DataStoreLocations.SIM1));
    }

    /**
     * int slotId SIM2.
     */
    public void test04_getSimUri() {
        assertNotNull(ContactsProxy.getSimUri(DataStoreLocations.SIM2));
    }

    /**
     * int slotId else.
     */
    public void test05_getSimUri() {
        assertNotNull(ContactsProxy.getSimUri(10));
    }

    /**
     * sourceLocation is SIM.
     */
    public void test06_getSlotId() {
        assertEquals(DataStoreLocations.SIM, ContactsProxy
                .getSlotId(RawContact.SOURCE_SIM));
    }

    /**
     * sourceLocation is SIM1.
     */
    public void test07_getSlotId() {
        assertEquals(DataStoreLocations.SIM1, ContactsProxy
                .getSlotId(RawContact.SOURCE_SIM1));
    }

    /**
     * sourceLocation is SIM2.
     */
    public void test08_getSlotId() {
        assertEquals(DataStoreLocations.SIM2, ContactsProxy
                .getSlotId(RawContact.SOURCE_SIM2));
    }

    /**
     * sourceLocation is others.
     */
    public void test09_getSlotId() {
        assertEquals(DataStoreLocations.NONE, ContactsProxy.getSlotId(-1));
    }

    /**
     * The sourceLocation < 0 or >4.
     */
    public void test10_getRealSlotId() {
        assertEquals(-1, ContactsProxy.getRealSlotId(5));
        assertEquals(-1, ContactsProxy.getRealSlotId(-1));
    }

    public void test11_getRealSlotId() {
        if (Config.MTK_GEMINI_SUPPORT) {
            assertEquals(0, ContactsProxy.getRealSlotId(1));
        } else {
            assertEquals(1, ContactsProxy.getRealSlotId(1));
        }
    }

    /**
     * Test "long insertSimContact(final String name, final String num, final
     * int slotId)".
     */
    public void test12_insertSimContact() {
        long result;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                result = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, mSimContact1.mSlotId);
                assertTrue("result: " + result, result >= 0);
                // delete the inserted rows in sim card.
                int rows = mContactsProxy.deleteSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, mSimContact1.mSlotId);
                assertTrue(rows >= 0);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                result = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, mSimContact2.mSlotId);
                assertTrue(result >= 0);
                // delete the inserted rows in sim card.
                int rows = mContactsProxy.deleteSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, mSimContact2.mSlotId);
                assertTrue(rows >= 0);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                result = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, mSimContact.mSlotId);
                assertTrue(result >= 0);
                // int result2 = mCP.deleteSimContact(mSim.mName, mSim.mNumber,
                // mSim.mSlotId);
                // assertTrue(result2 >= 0);
                // delete the inserted rows in sim card.
                int rows = mContactsProxy.deleteSimContact(mSimContact.mName,
                        mSimContact.mNumber, mSimContact.mSlotId);
                assertTrue(rows >= 0);
            } else {
                Debugger.logD(new Object[] {}, "SIM isn't accessible");
            }
        }
    }

    /**
     * Test "long insertSimContact(final String name, final String num, final
     * int slotId)". while the name is null or the number is null.
     */
    public void test13_insertSimContact() {
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                assertEquals(DatabaseRecordEntity.ID_NULL, mContactsProxy
                        .insertSimContact(null, mSimContact1.mNumber,
                                mSimContact1.mSlotId));
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                assertEquals(DatabaseRecordEntity.ID_NULL, mContactsProxy
                        .insertSimContact(null, mSimContact2.mNumber,
                                mSimContact2.mSlotId));
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                assertEquals(DatabaseRecordEntity.ID_NULL, mContactsProxy
                        .insertSimContact(null, mSimContact.mNumber,
                                mSimContact.mSlotId));
            } else {
                Debugger.logI("[ContactsProxyTest]SIM isn't accessible");
            }
        }
    }

    /**
     * Test "long insertSimContact(final String name, final String num, final
     * String email, final int slotId". while the name is null or the number is
     * null.
     */
    public void test14_insertSimContact() {
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                assertEquals(DatabaseRecordEntity.ID_NULL, mContactsProxy
                        .insertSimContact(null, mSimContact1.mNumber,
                                mSimContact1.mEmail, mSimContact1.mSlotId));
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                assertEquals(DatabaseRecordEntity.ID_NULL, mContactsProxy
                        .insertSimContact(null, mSimContact2.mNumber,
                                mSimContact2.mEmail, mSimContact2.mSlotId));
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                assertEquals(DatabaseRecordEntity.ID_NULL, mContactsProxy
                        .insertSimContact(null, mSimContact.mNumber,
                                mSimContact.mEmail, mSimContact.mSlotId));
            } else {
                Debugger.logI("[ContactsProxyTest]SIM isn't accessible");
            }
        }

    }

    /**
     * Test "long insertSimContact(final String name, final String num, final
     * String email, final int slotId".
     */
    public void test15_insertSimContact() {
        long result;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                result = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, mSimContact1.mEmail,
                        mSimContact1.mSlotId);
                assertTrue(result >= 0);
                // delete the inserted contacts in sim card for test.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact1.mName, mSimContact1.mNumber,
                        mSimContact1.mEmail, mSimContact1.mSlotId);
                assertTrue(deletedRows >= 0);
            } else {
                Debugger.logD(new Object[] {}, "SIM1 isn't accessible");
            }

            if (SystemInfoProxy.isSim2Accessible()) {
                result = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, mSimContact2.mEmail,
                        mSimContact2.mSlotId);
                assertTrue(result >= 0);
                // delete the inserted contacts in sim card for test.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact2.mName, mSimContact2.mNumber,
                        mSimContact2.mEmail, mSimContact2.mSlotId);
                assertTrue(deletedRows >= 0);
            } else {
                Debugger.logD(new Object[] {}, "SIM2 isn't accessible");
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                result = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, mSimContact.mEmail,
                        mSimContact.mSlotId);
                assertTrue(result >= 0);
                // int result2 = mCP.deleteSimContact(mSim.mName, mSim.mNumber,
                // mSim.mSlotId);
                // assertTrue(result2 >= 0);
                // delete the inserted contact in sim card for test.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact.mName, mSimContact.mNumber,
                        mSimContact.mEmail, mSimContact.mSlotId);
                assertTrue(deletedRows >= 0);
            } else {
                Debugger.logD(new Object[] {}, "SIM isn't accessible");
            }
        }
    }

    /**
     * Test "int updateSimContact(final String oldName, final String oldNumber,
     * final String newName, final String newNumber, final int slotId)" Params
     * not null.
     */
    public void test16_updateSimContact() {
        long result = -1;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, mSimContact1.mEmail,
                        mSimContact1.mSlotId);
                // update the sim contact.
                int updatedRows = mContactsProxy.updateSimContact(
                        mSimContact1.mName, mSimContact1.mNumber,
                        mSimNewContact.mName, mSimNewContact.mNumber,
                        mSimContact1.mSlotId);
                assertTrue(updatedRows > 0);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimNewContact.mName, mSimNewContact.mNumber,
                        mSimContact1.mSlotId);
                assertTrue(deletedRows >= 0);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, mSimContact2.mEmail,
                        mSimContact2.mSlotId);
                // update the sim contact.
                int updatedRows = mContactsProxy.updateSimContact(
                        mSimContact2.mName, mSimContact2.mNumber,
                        mSimNewContact.mName, mSimNewContact.mNumber,
                        mSimContact2.mSlotId);
                assertTrue(updatedRows > 0);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimNewContact.mName, mSimNewContact.mNumber,
                        mSimContact2.mSlotId);
                assertTrue(deletedRows >= 0);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, mSimContact.mEmail,
                        mSimContact.mSlotId);
                assertTrue(result >= 0);
                // update the sim contact.
                int updatedRows = mContactsProxy.updateSimContact(
                        mSimContact.mName, mSimContact.mNumber,
                        mSimNewContact.mName, mSimNewContact.mNumber,
                        mSimContact.mSlotId);
                assertTrue(updatedRows > 0);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimNewContact.mName, mSimNewContact.mNumber,
                        mSimContact.mSlotId);
                assertTrue(deletedRows >= 0);
            } else {
                Debugger.logI("[ContactsProxyTest]There is no sim card");
            }
        }

    }

    /**
     * oldName or oldNumber is null.
     */
    public void test17_updateSimContact() {
        long result = -1;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, mSimContact1.mEmail,
                        mSimContact1.mSlotId);
                // update the sim contact, the old name is null.
                int updatedRows = mContactsProxy.updateSimContact(null,
                        mSimContact1.mNumber, mSimNewContact.mName,
                        mSimNewContact.mNumber, mSimContact1.mSlotId);
                assertEquals(0, updatedRows);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact1.mName, mSimContact1.mNumber,
                        mSimContact1.mSlotId);
                assertTrue(deletedRows >= 0);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, mSimContact2.mEmail,
                        mSimContact2.mSlotId);
                // update the sim contact, the old name is null.
                int updatedRows = mContactsProxy.updateSimContact(null,
                        mSimContact2.mNumber, mSimNewContact.mName,
                        mSimNewContact.mNumber, mSimContact2.mSlotId);
                assertEquals(0, updatedRows);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact2.mName, mSimContact2.mNumber,
                        mSimContact2.mSlotId);
                assertTrue(deletedRows >= 0);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, mSimContact.mEmail,
                        mSimContact.mSlotId);
                assertTrue(result >= 0);
                // update the sim contact, the old name is null.
                int updatedRows = mContactsProxy.updateSimContact(null,
                        mSimContact.mNumber, mSimNewContact.mName,
                        mSimNewContact.mNumber, mSimContact.mSlotId);
                assertEquals(0, updatedRows);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact.mName, mSimContact.mNumber,
                        mSimContact.mSlotId);
                assertTrue(deletedRows >= 0);
            } else {
                Debugger.logI("[ContactsProxyTest]There is no sim card");
            }
        }

    }

    /**
     * newName or newNumber is null.
     */
    public void test18_updateSimContact() {
        long result = -1;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, mSimContact1.mEmail,
                        mSimContact1.mSlotId);
                // update the sim contact, the new name is null.
                int updatedRows = mContactsProxy.updateSimContact(
                        mSimContact1.mName, mSimContact1.mNumber, null, null,
                        mSimContact1.mSlotId);
                assertEquals(0, updatedRows);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact1.mName, mSimContact1.mNumber,
                        mSimContact1.mSlotId);
                assertTrue(deletedRows >= 0);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, mSimContact2.mEmail,
                        mSimContact2.mSlotId);
                // update the sim contact, the new name and new num is null.
                int updatedRows = mContactsProxy.updateSimContact(
                        mSimContact2.mName, mSimContact2.mNumber, null, null,
                        mSimContact2.mSlotId);
                assertEquals(0, updatedRows);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact2.mName, mSimContact2.mNumber,
                        mSimContact2.mSlotId);
                assertTrue(deletedRows >= 0);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, mSimContact.mEmail,
                        mSimContact.mSlotId);
                assertTrue(result >= 0);
                // update the sim contact, the new name is null.
                int updatedRows = mContactsProxy.updateSimContact(
                        mSimContact.mName, mSimContact.mNumber, null,
                        mSimNewContact.mNumber, mSimContact.mSlotId);
                assertEquals(0, updatedRows);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact.mName, mSimContact.mNumber,
                        mSimContact.mSlotId);
                assertTrue(deletedRows >= 0);
            } else {
                Debugger.logI("[ContactsProxyTest]There is no sim card");
            }
        }
    }

    /**
     * Test "int updateSimContact(final String oldName, final String oldNumber,
     * final String oEmail, final String newName, final String newNumber, final
     * String nEmail, final int slotId)" parmas not null.
     */
    public void test19_updateSimContact() {
        long result = -1;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, mSimContact1.mEmail,
                        mSimContact1.mSlotId);
                // update the sim contact.
                int updatedRows = mContactsProxy.updateSimContact(
                        mSimContact1.mName, mSimContact1.mNumber,
                        mSimContact1.mEmail, mSimNewContact.mName,
                        mSimNewContact.mNumber, mSimNewContact.mEmail,
                        mSimContact1.mSlotId);
                assertTrue(updatedRows > 0);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimNewContact.mName, mSimNewContact.mNumber,
                        mSimContact1.mSlotId);
                assertTrue(deletedRows >= 0);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, mSimContact2.mEmail,
                        mSimContact2.mSlotId);
                // update the sim contact, the old name is null.
                int updatedRows = mContactsProxy.updateSimContact(
                        mSimContact2.mName, mSimContact2.mNumber,
                        mSimContact2.mEmail, mSimNewContact.mName,
                        mSimNewContact.mNumber, mSimNewContact.mEmail,
                        mSimContact2.mSlotId);
                assertTrue(updatedRows > 0);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimNewContact.mName, mSimNewContact.mNumber,
                        mSimContact2.mSlotId);
                assertTrue(deletedRows >= 0);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, mSimContact.mEmail,
                        mSimContact.mSlotId);
                assertTrue(result >= 0);
                // update the sim contact.
                int updatedRows = mContactsProxy.updateSimContact(
                        mSimContact.mName, mSimContact.mNumber,
                        mSimContact.mEmail, mSimNewContact.mName,
                        mSimNewContact.mNumber, mSimNewContact.mEmail,
                        mSimContact.mSlotId);
                assertEquals(1, updatedRows);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimNewContact.mName, mSimNewContact.mNumber,
                        mSimContact.mSlotId);
                assertTrue(deletedRows >= 0);
            } else {
                Debugger.logI("[ContactsProxyTest]There is no sim card");
            }
        }
    }

    /**
     * oldName or oldNumber is null.
     */
    public void test20_updateSimContact() {
        long result = -1;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, mSimContact1.mEmail,
                        mSimContact1.mSlotId);
                // update the sim contact, the old name is null.
                int updatedRows = mContactsProxy.updateSimContact(null,
                        mSimContact1.mNumber, mSimContact1.mEmail,
                        mSimNewContact.mName, mSimNewContact.mNumber,
                        mSimNewContact.mEmail, mSimContact1.mSlotId);
                assertEquals(0, updatedRows);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact1.mName, mSimContact1.mNumber,
                        mSimContact1.mSlotId);
                assertTrue(deletedRows >= 0);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, mSimContact2.mEmail,
                        mSimContact2.mSlotId);
                // update the sim contact, the old name is null.
                int updatedRows = mContactsProxy.updateSimContact(null,
                        mSimContact2.mNumber, mSimContact2.mEmail,
                        mSimNewContact.mName, mSimNewContact.mNumber,
                        mSimNewContact.mEmail, mSimContact2.mSlotId);
                assertEquals(0, updatedRows);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact2.mName, mSimContact2.mNumber,
                        mSimContact2.mSlotId);
                assertTrue(deletedRows >= 0);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, mSimContact.mEmail,
                        mSimContact.mSlotId);
                assertTrue(result >= 0);
                // update the sim contact, the old name is null.
                int updatedRows = mContactsProxy.updateSimContact(null,
                        mSimContact.mNumber, mSimContact.mEmail,
                        mSimNewContact.mName, mSimNewContact.mNumber,
                        mSimNewContact.mEmail, mSimContact.mSlotId);
                assertEquals(0, updatedRows);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact.mName, mSimContact.mNumber,
                        mSimContact.mSlotId);
                assertTrue(deletedRows >= 0);
            } else {
                Debugger.logI("[ContactsProxyTest]There is no sim card");
            }
        }
    }

    /**
     * newName or newNumber is null.
     */
    public void test21_updateSimContact() {
        long result = -1;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, mSimContact1.mEmail,
                        mSimContact1.mSlotId);
                // update the sim contact, the new name is null.
                int updatedRows = mContactsProxy.updateSimContact(
                        mSimContact1.mName, mSimContact1.mNumber,
                        mSimContact1.mEmail, null, null, mSimNewContact.mEmail,
                        mSimContact1.mSlotId);
                assertEquals(0, updatedRows);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact1.mName, mSimContact1.mNumber,
                        mSimContact1.mSlotId);
                assertTrue(deletedRows >= 0);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, mSimContact2.mEmail,
                        mSimContact2.mSlotId);
                // update the sim contact, the old name is null.
                int updatedRows = mContactsProxy.updateSimContact(
                        mSimContact2.mName, mSimContact2.mNumber,
                        mSimContact2.mEmail, null, null, mSimNewContact.mEmail,
                        mSimContact2.mSlotId);
                assertEquals(0, updatedRows);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact2.mName, mSimContact2.mNumber,
                        mSimContact2.mSlotId);
                assertTrue(deletedRows >= 0);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, mSimContact.mEmail,
                        mSimContact.mSlotId);
                assertTrue(result >= 0);
                // update the sim contact, the new name is null.
                int updatedRows = mContactsProxy.updateSimContact(
                        mSimContact.mName, mSimContact.mNumber,
                        mSimContact.mEmail, null, mSimNewContact.mNumber,
                        mSimNewContact.mEmail, mSimContact.mSlotId);
                assertEquals(0, updatedRows);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact.mName, mSimContact.mNumber,
                        mSimContact.mSlotId);
                assertTrue(deletedRows >= 0);
            } else {
                Debugger.logI("[ContactsProxyTest]There is no sim card");
            }
        }

    }

    /**
     * Test"int deleteSimContact(final String name, final String deleteNumber,
     * final int slotId)"
     */
    public void test22_deleteSimContact() {
        long result = -1;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, mSimContact1.mSlotId);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact1.mName, mSimContact1.mNumber,
                        mSimContact1.mSlotId);
                assertTrue("deletedRows: " + deletedRows, deletedRows >= 0);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, mSimContact2.mSlotId);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact2.mName, mSimContact2.mNumber,
                        mSimContact2.mSlotId);
                assertTrue(deletedRows >= 0);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, mSimContact.mSlotId);
                assertTrue(result >= 0);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact.mName, mSimContact.mNumber,
                        mSimContact.mSlotId);
                assertTrue(deletedRows >= 0);
            } else {
                Debugger.logI("[ContactsProxyTest]There is no sim card");
            }
        }
    }

    /**
     * name is null.
     */
    public void test23_deleteSimContact() {
        long result = -1;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, mSimContact1.mSlotId);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(null,
                        mSimContact1.mNumber, mSimContact1.mSlotId);
                assertEquals(0, deletedRows);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, mSimContact2.mSlotId);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(null,
                        mSimContact2.mNumber, mSimContact2.mSlotId);
                assertEquals(0, deletedRows);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, mSimContact.mSlotId);
                assertTrue(result >= 0);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(null,
                        mSimContact.mNumber, mSimContact.mSlotId);
                assertEquals(0, deletedRows);
            } else {
                Debugger.logI("[ContactsProxyTest]There is no sim card");
            }
        }
    }

    /**
     * Test "int deleteSimContact(final String name, final String deleteNumber,
     * final String email, final int slotId)"
     */
    public void test24_deleteSimContact() {
        long result = -1;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, mSimContact1.mEmail,
                        mSimContact1.mSlotId);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact1.mName, mSimContact1.mNumber,
                        mSimContact1.mSlotId);
                assertTrue("deleteRows:" + deletedRows, deletedRows >= 0);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, mSimContact2.mEmail,
                        mSimContact2.mSlotId);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact2.mName, mSimContact2.mNumber,
                        mSimContact2.mSlotId);
                assertTrue(deletedRows >= 0);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, mSimContact.mEmail,
                        mSimContact.mSlotId);
                assertTrue(result >= 0);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(
                        mSimContact.mName, mSimContact.mNumber,
                        mSimContact.mSlotId);
                assertTrue(deletedRows >= 0);
            } else {
                Debugger.logI("[ContactsProxyTest]There is no sim card");
            }
        }

    }

    /**
     * name is null.
     */
    public void test25_deleteSimContact() {
        long result = -1;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, mSimContact1.mEmail,
                        mSimContact1.mSlotId);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(null,
                        mSimContact1.mNumber, mSimContact1.mSlotId);
                assertEquals(0, deletedRows);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, mSimContact2.mEmail,
                        mSimContact2.mSlotId);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(null,
                        mSimContact2.mNumber, mSimContact2.mSlotId);
                assertEquals(0, deletedRows);
            }

        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                // insert a sim contact.
                result = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, mSimContact.mEmail,
                        mSimContact.mSlotId);
                assertTrue(result >= 0);
                // delete the sim contact.
                int deletedRows = mContactsProxy.deleteSimContact(null,
                        mSimContact.mNumber, mSimContact.mSlotId);
                assertEquals(0, deletedRows);
            } else {
                Debugger.logI("[ContactsProxyTest]There is no sim card");
            }
        }

    }

    /**
     * Test "boolean[] deleteSimContacts(final String[] names, final String[]
     * numbers, final int slotId)".
     */
    public void test26_deleteSimContacts() {
        String[] names = { mSimContact.mName, mSimContact1.mName,
                mSimContact2.mName };
        String[] numbers = { mSimContact.mNumber, mSimContact1.mNumber,
                mSimContact2.mNumber };
        long result = -1;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                // insert 3 sim contacts.
                result = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, DataStoreLocations.SIM1);
                result = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, DataStoreLocations.SIM1);
                result = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, DataStoreLocations.SIM1);
                // delete the sim contacts.
                boolean[] results = mContactsProxy.deleteSimContacts(names,
                        numbers, DataStoreLocations.SIM1);
                assertEquals(3, results.length);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                // insert 3 sim contacts.
                result = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, DataStoreLocations.SIM2);
                result = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, DataStoreLocations.SIM2);
                result = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, DataStoreLocations.SIM2);
                // delete the sim contacts.
                boolean[] results = mContactsProxy.deleteSimContacts(names,
                        numbers, DataStoreLocations.SIM2);
                assertEquals(3, results.length);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                // insert 3 sim contacts.
                result = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, DataStoreLocations.SIM);
                result = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, DataStoreLocations.SIM);
                result = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, DataStoreLocations.SIM);
                // delete the sim contacts.
                boolean[] results = mContactsProxy.deleteSimContacts(names,
                        numbers, DataStoreLocations.SIM);
                assertEquals(3, results.length);
            } else {
                Debugger.logI("[ContactsProxyTest]There is no sim card");
            }
        }

        boolean[] results;
        results = mContactsProxy.deleteSimContacts(null, numbers,
                DataStoreLocations.SIM1);
        assertNull(results);
        results = mContactsProxy.deleteSimContacts(names, null,
                DataStoreLocations.SIM1);
        assertNull(results);
        names = new String[] { "1" };
        results = mContactsProxy.deleteSimContacts(names, numbers,
                DataStoreLocations.SIM1);
        assertNull(results);
    }

    /**
     * Test "RawContact getContact(final long id, final boolean withData)". The
     * withData is false.
     */
    public void test27_getContact() {
        boolean withData = true;
        RawContact newContact = getRawContact();
        long id = mContactsProxy.insertContact(newContact, withData);
        assertNotNull(mContactsProxy.getContact(id, withData));
    }

    /**
     * Test "RawContact getContact(final long id, final boolean withData)". The
     * withData is true.
     */
    public void test28_getContact() {
        boolean withData = true;
        RawContact newContact = getRawContact();
        long id = mContactsProxy.insertContact(newContact, withData);
        assertNotNull(mContactsProxy.getContact(id, withData));
    }

    public void test29_getContactSourceLocation() {
        RawContact rawContact = getRawContact();
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                rawContact.setSourceLocation(RawContact.SIM1);
                // insert a raw contact.
                long id = mContactsProxy.insertContact(rawContact, true);
                assertTrue("id: " + id, id > 0);
                // get the source location
                int sourceLocation = mContactsProxy
                        .getContactSourceLocation(id);
//                assertEquals(DataStoreLocations.SIM1, sourceLocation);
            }
            if (SystemInfoProxy.isSim2Accessible()) {

                rawContact.setSourceLocation(RawContact.SIM2);
                // insert a raw contact.
                long id = mContactsProxy.insertContact(rawContact, false);
                assertTrue("id " + id, id >= 0);
                // get the source location
                int sourceLocation = mContactsProxy
                        .getContactSourceLocation(id);
//                assertEquals(DataStoreLocations.SIM2, sourceLocation);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                rawContact.setSourceLocation(RawContact.SIM);
                // insert a raw contact.
                long id = mContactsProxy.insertContact(rawContact, false);
                // get the source location
                int sourceLocation = mContactsProxy
                        .getContactSourceLocation(id);
                // assertEquals(DataStoreLocations.SIM, sourceLocation);
            } else {
                Debugger.logI("[ContactsProxyTest]There is no sim card");
            }
        }
    }

    /**
     * Test
     * "long insertContact(final RawContact newContact, final boolean withData)"
     * The withData is true.
     */
    public void test30_insertContact() {
        boolean withData = true;
        RawContact newContact = getRawContact();
        newContact.setStoreLocation(RawContact.SOURCE_SIM1);
        long id = mContactsProxy.insertContact(newContact, withData);
        assertTrue(id >= 0);
        // delete the contact.
        mContactsProxy.deleteContact(id, true, RawContact.SOURCE_SIM1, null,
                null);
    }

    /**
     * Test
     * "long insertContact(final RawContact newContact, final boolean withData)"
     * The withData is false.
     */
    public void test31_insertContact() {
        boolean withData = false;
        RawContact newContact = getRawContact();
        // in phone.
        newContact.setStoreLocation(RawContact.SOURCE_PHONE);
        long id = mContactsProxy.insertContact(newContact, withData);
        assertTrue(id >= 0);
        // in sim.
        newContact.setStoreLocation(RawContact.SOURCE_SIM1);
        id = mContactsProxy.insertContact(newContact, withData);
        assertTrue(id >= 0);
        // delete the contact.
        mContactsProxy.deleteContact(id, true, RawContact.SOURCE_PHONE,
                newContact.getSimName(), newContact.getPhones().get(0)
                        .getNumber());
    }

    /**
     * Test "int updateContact(final long id, final int sourceLocation, final
     * String simName, final String simNumber, final RawContact newContact,
     * final boolean updatePIMData)"
     */
    public void test32_updateContact() {
        int result;
        RawContact rawContact = ContactsUtils.getRawContact();
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                rawContact.setSourceLocation(RawContact.SOURCE_SIM1);
                long rawContactId = mContactsProxy.insertContact(rawContact,
                        false);
                assertTrue(rawContactId > 0);
                result = mContactsProxy.updateContact(rawContactId,
                        RawContact.SOURCE_SIM1, mSimContact.mName,
                        mSimContact.mNumber, rawContact, true);
                assertTrue(result >= 0);
            }

            if (SystemInfoProxy.isSim2Accessible()) {
                rawContact.setSourceLocation(RawContact.SOURCE_SIM2);
                long rawContactId = mContactsProxy.insertContact(rawContact,
                        false);
                assertTrue(rawContactId > 0);
                result = mContactsProxy.updateContact(rawContactId,
                        RawContact.SOURCE_SIM2, mSimContact.mName,
                        mSimContact.mNumber, rawContact, true);
                assertTrue(result >= 0);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                rawContact.setSourceLocation(RawContact.SOURCE_SIM);
                long rawContactId = mContactsProxy.insertContact(rawContact,
                        false);
                assertTrue(rawContactId > 0);
                result = mContactsProxy.updateContact(rawContactId,
                        RawContact.SOURCE_SIM, mSimContact.mName,
                        mSimContact.mNumber, rawContact, true);
                assertTrue(result >= 0);
            }
        }

        rawContact.setSourceLocation(RawContact.SOURCE_PHONE);
        long rawContactId = mContactsProxy.insertContact(rawContact, true);
        assertTrue(rawContactId > 0);
        result = mContactsProxy.updateContact(rawContactId,
                RawContact.SOURCE_PHONE, mSimContact.mName,
                mSimContact.mNumber, rawContact, true);
        assertTrue(result >= 0);

        result = mContactsProxy.updateContact(rawContactId,
                RawContact.SOURCE_PHONE, mSimContact.mName,
                mSimContact.mNumber, null, true);
        assertTrue(result == 0);
    }

    /**
     * Test "int updateContact(final long id, final int sourceLocation, final
     * String simName, final String simNumber, final String simEmail, final
     * RawContact newContact, final boolean updatePIMData) "
     */
    public void test33_updateContact() {
        boolean withData = true;
        RawContact rawContact = getRawContact();
        rawContact.setStoreLocation(RawContact.SOURCE_SIM1);
        long id = mContactsProxy.insertContact(rawContact, withData);
        RawContact newContact = new RawContact();
        // newContact.setContactId(100);
        newContact.setLastTimeContacted(557668675);
        newContact.setStarred(true);
        newContact.setCustomRingtone("custom ringtone");
        newContact.setSendToVoicemail(false);
        newContact.setVersion(2);
        newContact.setDirty(true);
        ArrayList<StructuredName> names = new ArrayList<StructuredName>();
        StructuredName name = new StructuredName();
        name.setMimeType(StructuredName.MIME_TYPE);
        name.setDisplayName("displayName");
        name.setFamilyName("familyName");
        name.setGivenName("givenName");
        names.add(name);
        newContact.setNames(names);
        ArrayList<Phone> phones = new ArrayList<Phone>();
        Phone phone = new Phone();
        phone.setNumber(mSimContact2.mNumber);
        phone.setLabel("a phone num");
        phone.setMimeType(Phone.MIME_TYPE);
        phone.setType(Phone.TYPE_HOME);
        phones.add(phone);
        newContact.setPhones(phones);
        ArrayList<Photo> photos = new ArrayList<Photo>();
        newContact.setPhotos(photos);
        ArrayList<Email> emails = new ArrayList<Email>();
        Email email = new Email();
        email.setMimeType(Email.MIME_TYPE);
        email.setData(mSimContact2.mEmail);
        email.setLabel("meeting");
        email.setType(Email.TYPE_WORK);
        emails.add(email);
        newContact.setEmails(emails);
        ArrayList<Im> ims = new ArrayList<Im>();
        Im im = new Im();
        im.setMimeType(Im.MIME_TYPE);
        im.setData("2646455");
        im.setLabel("QQ");
        im.setProtocol(Im.PROTOCOL_QQ);
        im.setType(Im.TYPE_HOME);
        im.setCustomProtocol(Im.MIME_TYPE_STRING);
        ims.add(im);
        newContact.setIms(ims);
        ArrayList<StructuredPostal> postals = new ArrayList<StructuredPostal>();
        StructuredPostal postal = new StructuredPostal();
        postal.setMimeType(StructuredPostal.MIME_TYPE);
        postal.setCity("chengdu");
        postal.setCountry("China");
        postal.setLabel("Home");
        postal.setFormattedAddress(StructuredPostal.MIME_TYPE_STRING);
        postal.setNeighborhood("neighborhood");
        postal.setPostcode("11655");
        postal.setRegion("si chuang");
        postal.setStreet("tuoxin street");
        postal.setType(StructuredPostal.TYPE_OTHER);
        postals.add(postal);
        newContact.setPostals(postals);
        ArrayList<Organization> organizations = new ArrayList<Organization>();
        newContact.setOrganizations(organizations);
        ArrayList<Note> notes = new ArrayList<Note>();
        Note note = new Note();
        note.setMimeType(Note.MIME_TYPE);
        note.setNote("test note");
        notes.add(note);
        newContact.setNotes(notes);
        ArrayList<Nickname> nicknames = new ArrayList<Nickname>();
        Nickname nickname = new Nickname();
        nickname.setLabel("a nickname");
        nickname.setMimeType(Nickname.MIME_TYPE);
        nickname.setName("zhangsan");
        nickname.setType(Nickname.TYPE_OTHER_NAME);
        nicknames.add(nickname);
        newContact.setNicknames(nicknames);
        ArrayList<Website> websites = new ArrayList<Website>();
        Website website = new Website();
        website.setLabel("website");
        website.setMimeType(Website.MIME_TYPE);
        website.setType(Website.TYPE_HOME);
        website.setUrl("www.google.com");
        websites.add(website);
        newContact.setWebsites(websites);
        newContact.setSourceLocation(RawContact.SOURCE_SIM1);
        newContact.setSimName(mSimContact2.mName);

        int updatedRows = mContactsProxy.updateContact(id,
                RawContact.SOURCE_SIM1, mSimContact.mName, mSimContact.mNumber,
                mSimContact.mEmail, newContact, true);
        assertTrue(updatedRows >= 0);

        mContactsProxy.updateContact(id, RawContact.SOURCE_SIM,
                mSimContact.mName, mSimContact.mNumber, mSimContact.mEmail,
                newContact, true);
        mContactsProxy.updateContact(id, RawContact.SOURCE_SIM2,
                mSimContact.mName, mSimContact.mNumber, mSimContact.mEmail,
                newContact, true);
        mContactsProxy.updateContact(id, RawContact.SOURCE_PHONE,
                mSimContact.mName, mSimContact.mNumber, mSimContact.mEmail,
                newContact, true);
    }

    public void test33_updateContact2() {
        int result;
        RawContact rawContact = ContactsUtils.getRawContact();
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                rawContact.setSourceLocation(RawContact.SOURCE_SIM1);
                long rawContactId = mContactsProxy.insertContact(rawContact,
                        false);
                assertTrue(rawContactId > 0);
                result = mContactsProxy.updateContact(rawContactId,
                        RawContact.SOURCE_SIM1, mSimContact.mName,
                        mSimContact.mNumber, mSimContact.mEmail, rawContact,
                        true);
                assertTrue(result >= 0);
            }

            if (SystemInfoProxy.isSim2Accessible()) {
                rawContact.setSourceLocation(RawContact.SOURCE_SIM2);
                long rawContactId = mContactsProxy.insertContact(rawContact,
                        false);
                assertTrue(rawContactId > 0);
                result = mContactsProxy.updateContact(rawContactId,
                        RawContact.SOURCE_SIM2, mSimContact.mName,
                        mSimContact.mNumber, mSimContact.mEmail, rawContact,
                        true);
                assertTrue(result >= 0);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                rawContact.setSourceLocation(RawContact.SOURCE_SIM);
                long rawContactId = mContactsProxy.insertContact(rawContact,
                        false);
                assertTrue(rawContactId > 0);
                result = mContactsProxy.updateContact(rawContactId,
                        RawContact.SOURCE_SIM, mSimContact.mName,
                        mSimContact.mNumber, mSimContact.mEmail, rawContact,
                        true);
                assertTrue(result >= 0);
            }
        }

        rawContact.setSourceLocation(RawContact.SOURCE_PHONE);
        long rawContactId = mContactsProxy.insertContact(rawContact, true);
        assertTrue(rawContactId > 0);
        result = mContactsProxy.updateContact(rawContactId,
                RawContact.SOURCE_PHONE, mSimContact.mName,
                mSimContact.mNumber, mSimContact.mEmail, rawContact, true);
        assertTrue(result >= 0);

        result = mContactsProxy.updateContact(rawContactId,
                RawContact.SOURCE_PHONE, mSimContact.mName,
                mSimContact.mNumber, mSimContact.mEmail, null, true);
        assertTrue(result == 0);
    }

    /**
     * Test "int deleteContact(final long id, final boolean permanently, final
     * int sourceLocation,final String simName, final String simNumber)"
     */
    public void test34_deleteContact() {
        int result;
        RawContact rawContact = ContactsUtils.getRawContact();
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                rawContact.setSourceLocation(RawContact.SOURCE_SIM1);
                long rawContactId = mContactsProxy.insertContact(rawContact,
                        false);
                assertTrue(rawContactId > 0);
                result = mContactsProxy.deleteContact(rawContactId, true,
                        RawContact.SOURCE_SIM1, mSimContact.mName,
                        mSimContact.mNumber);
                assertTrue(result >= 0);
            }

            if (SystemInfoProxy.isSim2Accessible()) {
                rawContact.setSourceLocation(RawContact.SOURCE_SIM2);
                long rawContactId = mContactsProxy.insertContact(rawContact,
                        false);
                assertTrue(rawContactId > 0);
                result = mContactsProxy.deleteContact(rawContactId, true,
                        RawContact.SOURCE_SIM2, mSimContact.mName,
                        mSimContact.mNumber);
                assertTrue(result >= 0);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                rawContact.setSourceLocation(RawContact.SOURCE_SIM);
                long rawContactId = mContactsProxy.insertContact(rawContact,
                        false);
                assertTrue(rawContactId > 0);
                result = mContactsProxy.deleteContact(rawContactId, true,
                        RawContact.SOURCE_SIM, mSimContact.mName,
                        mSimContact.mNumber);
                assertTrue(result >= 0);
            }
        }

        rawContact.setSourceLocation(RawContact.SOURCE_PHONE);
        long rawContactId = mContactsProxy.insertContact(rawContact, true);
        assertTrue(rawContactId > 0);
        result = mContactsProxy
                .deleteContact(rawContactId, true, RawContact.SOURCE_PHONE,
                        mSimContact.mName, mSimContact.mNumber);
        assertTrue(result >= 0);

        result = mContactsProxy.deleteContact(rawContactId, true,
                RawContact.SOURCE_NONE, mSimContact.mName, mSimContact.mNumber);
        assertTrue(result == 0);

    }

    /**
     * Test "boolean[] deleteContacts(final long[] ids, final boolean
     * permanently, final int sourceLocation, final String[] simNames, final
     * String[] simNumbers)"
     */
    public void test35_deleteContacts() {
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                // insert 3 sim contacts.
                long[] ids = new long[3];
                ids[0] = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, DataStoreLocations.SIM1);
                ids[1] = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, DataStoreLocations.SIM1);
                ids[2] = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, DataStoreLocations.SIM1);
                // delete the sim contacts.
                String[] simNames = { mSimContact.mName, mSimContact1.mName,
                        mSimContact2.mName };
                String[] simNumbers = { mSimContact.mNumber,
                        mSimContact1.mNumber, mSimContact2.mNumber };
                boolean[] results = mContactsProxy.deleteContacts(ids, false,
                        DataStoreLocations.SIM1, simNames, simNumbers);
                assertEquals(3, results.length);
            }

            if (SystemInfoProxy.isSim2Accessible()) {
                // insert 3 sim contacts.
                long[] ids = new long[3];
                ids[0] = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, DataStoreLocations.SIM2);
                ids[1] = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, DataStoreLocations.SIM2);
                ids[2] = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, DataStoreLocations.SIM2);
                // delete the sim contacts.
                String[] simNames = { mSimContact.mName, mSimContact1.mName,
                        mSimContact2.mName };
                String[] simNumbers = { mSimContact.mNumber,
                        mSimContact1.mNumber, mSimContact2.mNumber };
                boolean[] results = mContactsProxy.deleteContacts(ids, false,
                        DataStoreLocations.SIM2, simNames, simNumbers);
                assertEquals(3, results.length);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                // insert 3 sim contacts.
                long[] ids = new long[3];
                ids[0] = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, DataStoreLocations.SIM);
                ids[1] = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, DataStoreLocations.SIM);
                ids[2] = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, DataStoreLocations.SIM);
                // delete the sim contacts.
                String[] simNames = { mSimContact.mName, mSimContact1.mName,
                        mSimContact2.mName };
                String[] simNumbers = { mSimContact.mNumber,
                        mSimContact1.mNumber, mSimContact2.mNumber };
                boolean[] results = mContactsProxy.deleteContacts(ids, false,
                        DataStoreLocations.SIM, simNames, simNumbers);
                assertEquals(3, results.length);
            } else {
                Debugger.logI("[ContactsProxyTest]There is no sim card");
            }
        }

        long[] ids = new long[] { 1, 2 };
        mContactsProxy.deleteContacts(ids, true, RawContact.SOURCE_PHONE, null,
                null);
        mContactsProxy.deleteContacts(ids, true, RawContact.SOURCE_SIM, null,
                null);
        mContactsProxy.deleteContacts(ids, true, RawContact.SOURCE_SIM1, null,
                null);
        mContactsProxy.deleteContacts(ids, true, RawContact.SOURCE_SIM2, null,
                null);
    }

    /**
     * Test "boolean[] deleteContacts(final long[] ids, final boolean
     * permanently, final int sourceLocation, final String[] simNames, final
     * String[] simNumbers, final String[] emails) "
     */
    public void test36_deleteContacts() {
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                // insert 3 sim contacts.
                long[] ids = new long[3];
                ids[0] = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, DataStoreLocations.SIM1);
                ids[1] = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, DataStoreLocations.SIM1);
                ids[2] = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, DataStoreLocations.SIM1);
                // delete the sim contacts.
                String[] simNames = { mSimContact.mName, mSimContact1.mName,
                        mSimContact2.mName };
                String[] simNumbers = { mSimContact.mNumber,
                        mSimContact1.mNumber, mSimContact2.mNumber };
                String[] emails = { mSimContact.mEmail, mSimContact1.mEmail,
                        mSimContact2.mEmail };
                boolean[] results = mContactsProxy.deleteContacts(ids, true,
                        DataStoreLocations.SIM1, simNames, simNumbers, emails);
                assertEquals(3, results.length);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                // insert 3 sim contacts.
                long[] ids = new long[3];
                ids[0] = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, DataStoreLocations.SIM2);
                ids[1] = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, DataStoreLocations.SIM2);
                ids[2] = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, DataStoreLocations.SIM2);
                // delete the sim contacts.
                String[] simNames = { mSimContact.mName, mSimContact1.mName,
                        mSimContact2.mName };
                String[] simNumbers = { mSimContact.mNumber,
                        mSimContact1.mNumber, mSimContact2.mNumber };
                String[] emails = { mSimContact.mEmail, mSimContact1.mEmail,
                        mSimContact2.mEmail };
                boolean[] results = mContactsProxy.deleteContacts(ids, true,
                        DataStoreLocations.SIM2, simNames, simNumbers, emails);
                assertEquals(3, results.length);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                // insert 3 sim contacts.
                long[] ids = new long[3];
                ids[0] = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, DataStoreLocations.SIM);
                ids[1] = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, DataStoreLocations.SIM);
                ids[2] = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, DataStoreLocations.SIM);
                // delete the sim contacts.
                String[] simNames = { mSimContact.mName, mSimContact1.mName,
                        mSimContact2.mName };
                String[] simNumbers = { mSimContact.mNumber,
                        mSimContact1.mNumber, mSimContact2.mNumber };
                String[] emails = { mSimContact.mEmail, mSimContact1.mEmail,
                        mSimContact2.mEmail };
                boolean[] results = mContactsProxy.deleteContacts(ids, true,
                        DataStoreLocations.SIM, simNames, simNumbers, emails);
                assertEquals(3, results.length);
            } else {
                Debugger.logI("[ContactsProxyTest]There is no sim card");
            }
        }
        long[] ids = new long[] { 1, 2 };
        mContactsProxy.deleteContacts(ids, true, RawContact.SOURCE_PHONE, null,
                null, null);
        mContactsProxy.deleteContacts(ids, true, RawContact.SOURCE_SIM, null,
                null, null);
        mContactsProxy.deleteContacts(ids, true, RawContact.SOURCE_SIM1, null,
                null, null);
        mContactsProxy.deleteContacts(ids, true, RawContact.SOURCE_SIM2, null,
                null, null);
    }

    /**
     * Test "int deleteAllContacts(final boolean permanently)".
     */
    public void test37_deleteAllContacts() {
        RawContact newContact = getRawContact();
        mContactsProxy.insertContact(newContact, false);
        int deleteContacts = mContactsProxy.deleteAllContacts(true);
        assertTrue("delete " + deleteContacts, deleteContacts > 0);
    }

    public void test38_updateAllContactForBackup() {
        RawContact newContact = getRawContact();
        mContactsProxy.insertContact(newContact, false);
        int updatedCounts = mContactsProxy.updateAllContactForBackup();
        assertTrue(updatedCounts > 0);
    }

    public void test39_deleteContactForBackup() {
        RawContact newContact = getRawContact();
        mContactsProxy.insertContact(newContact, false);
        int deletedCount = mContactsProxy.deleteContactForBackup();
        assertTrue(deletedCount > 0);
    }

    public void test40_fastImportDetailedContacts() {
        IRawBlockConsumer rawContactsConsumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {
            }

        };

        IRawBlockConsumer contactDataConsumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {
            }

        };
        // raw != null && outBuffer !=null.
        ByteBuffer outBuffer = Global.getByteBuffer();
        mContactsProxy.fastImportDetailedContacts(byteContact,
                rawContactsConsumer, contactDataConsumer, outBuffer);
        // raw == null.
        byte[] raw = null;
        mContactsProxy.fastImportDetailedContacts(raw, rawContactsConsumer,
                contactDataConsumer, outBuffer);
        // outBuffer == contactDataConsumer.
        outBuffer.clear();
        RawContact contact = getRawContact();
        contact.writeRawWithVersion(outBuffer, Config.VERSION_CODE);
        outBuffer.position(0);
        mContactsProxy.fastImportDetailedContacts(raw, rawContactsConsumer,
                contactDataConsumer, outBuffer);
        raw = outBuffer.array();
        contactDataConsumer = null;
        mContactsProxy.fastImportDetailedContacts(raw, rawContactsConsumer,
                contactDataConsumer, outBuffer);

    }

    public void test41_asyncGetAllGroups() {

        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {
            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        mContactsProxy.asyncGetAllGroups(consumer, buffer);
    }

    public void test42_asyncGetAllSimContacts() {
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {
            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        mContactsProxy.asyncGetAllSimContacts(consumer, buffer);
    }

    public void test43_asyncGetAllSimContacts() {
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {
            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        int slotId = -1;
        if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
            slotId = DataStoreLocations.SIM;
            mContactsProxy.asyncGetAllSimContacts(consumer, buffer, slotId);
        }
        if (SystemInfoProxy.isSim1Accessible()) {
            slotId = DataStoreLocations.SIM1;
            mContactsProxy.asyncGetAllSimContacts(consumer, buffer, slotId);
        }

        if (SystemInfoProxy.isSim2Accessible()) {
            slotId = DataStoreLocations.SIM2;
            mContactsProxy.asyncGetAllSimContacts(consumer, buffer, slotId);
        }
    }

    public void test44_asyncGetAllRawContacts() {
        IRawBlockConsumer consumer = new IRawBlockConsumer() {

            public void consume(byte[] block, int blockNo, int totalNo) {

            }

        };
        ByteBuffer buffer = Global.getByteBuffer();
        long idFrom = 0;
        long idTo = 10000;
        mContactsProxy.asyncGetAllRawContacts(consumer, buffer, idFrom, idTo);
    }

    public void test45_asyncGetAllRawContactsForBackup() {
        IRawBlockConsumer consumer = new IRawBlockConsumer() {

            public void consume(byte[] block, int blockNo, int totalNo) {

            }

        };
        ByteBuffer buffer = Global.getByteBuffer();
        long idFrom = 0;
        long idTo = 100;
        mContactsProxy.asyncGetAllRawContactsForBackup(consumer, buffer,
                idFrom, idTo);
    }

    public void test46_asyncGetAllRawContacts() {
        IRawBlockConsumer consumer = new IRawBlockConsumer() {

            public void consume(byte[] block, int blockNo, int totalNo) {

            }

        };
        ByteBuffer buffer = Global.getByteBuffer();
        long idFrom = 0;
        long idTo = 10000;
        mContactsProxy.asyncGetAllRawContacts(consumer, buffer, idFrom, idTo);
    }

    public void test47_asyncGetAllRawContactsForBackup() {
        IRawBlockConsumer consumer = new IRawBlockConsumer() {

            public void consume(byte[] block, int blockNo, int totalNo) {

            }

        };
        ByteBuffer buffer = Global.getByteBuffer();
        long idFrom = 0;
        long idTo = 10000;
        mContactsProxy.asyncGetAllRawContactsForBackup(consumer, buffer,
                idFrom, idTo);
    }

    /**
     * Test "void asyncGetAllContactData(final List<Integer> requiredMimeTypes,
     * final IRawBlockConsumer consumer, final ByteBuffer buffer)"
     */
    public void test48_asyncGetAllContactData() {
        IRawBlockConsumer consumer = new IRawBlockConsumer() {

            public void consume(byte[] block, int blockNo, int totalNo) {

            }

        };
        ByteBuffer buffer = Global.getByteBuffer();
        ArrayList<Integer> requiredMimeTypes = new ArrayList<Integer>();
        requiredMimeTypes.add(StructuredName.MIME_TYPE);
        requiredMimeTypes.add(Phone.MIME_TYPE);

        requiredMimeTypes.add(Email.MIME_TYPE);
        requiredMimeTypes.add(Im.MIME_TYPE);
        requiredMimeTypes.add(StructuredPostal.MIME_TYPE);
        requiredMimeTypes.add(Organization.MIME_TYPE);
        requiredMimeTypes.add(Nickname.MIME_TYPE);
        requiredMimeTypes.add(Note.MIME_TYPE);
        requiredMimeTypes.add(Website.MIME_TYPE);
        requiredMimeTypes.add(GroupMembership.MIME_TYPE);

//        mContactsProxy.asyncGetAllContactData(requiredMimeTypes, consumer,
//                buffer);
    }

    public void test49_insertGroup() {
        Group group = new Group();
        group.setAccount_name("usim" + USIMUtils.ACCOUNT_NAME_USIM);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        group.setVersion(Config.VERSION_STRING);
        group.setDeleted("false");
        group.setDirty("false");
        group.setGroup_visible("true");
        group.setNotes("notes");
        group.setShould_sync("true");
        group.setTitle("classmates");
        ArrayList<BaseContact> members = new ArrayList<BaseContact>();
        BaseContact bContact1 = new BaseContact();
        bContact1.setDisplayName("displayName");
        bContact1.setModifyTime(146541654);
        bContact1.setPrimaryNumber("15858474521");
        bContact1.setSimName("sim1");
        members.add(bContact1);
        group.setMembers(members);
        long id = mContactsProxy.insertGroup(group);
        assertTrue(id >= 0);
        int deleteRows = mContactsProxy.deleteGroup(id);
        assertEquals(1, deleteRows);
    }

    public void test50_insertGroups() {
        ArrayList<Group> groups = new ArrayList<Group>();
        Group group = new Group();
        group.setAccount_name(USIMUtils.ACCOUNT_NAME_LOCAL_PHONE);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_LOCAL_PHONE);
        group.setVersion(Config.VERSION_STRING);
        group.setDeleted("false");
        group.setDirty("false");
        group.setGroup_visible("true");
        group.setNotes("notes");
        group.setShould_sync("false");
        group.setTitle("classmates");

        Group group2 = new Group();
        group2.setAccount_name(USIMUtils.ACCOUNT_NAME_LOCAL_PHONE);
        group2.setAccount_type(USIMUtils.ACCOUNT_TYPE_LOCAL_PHONE);
        group2.setVersion(Config.VERSION_STRING);
        group2.setDeleted("true");
        group2.setDirty("true");
        group2.setGroup_visible("true");
        group2.setShould_sync("true");
        groups.add(group);
        int insertRows = mContactsProxy.insertGroups(groups);
        assertTrue(insertRows > 0);
    }

    /**
     * Test "int updateGroup(final long id, final Group newGroup)".
     */
    public void test51_updateGroup() {
        Group group = new Group();
        group.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        group.setVersion(Config.VERSION_STRING);
        group.setDeleted("false");
        group.setDirty("false");
        group.setGroup_visible("true");
        group.setNotes("notes");
        group.setShould_sync("false");
        group.setTitle("classmates");

        long id = mContactsProxy.insertGroup(group);
        Group newGroup = new Group();
        group.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        group.setVersion(Config.VERSION_STRING);
        newGroup.setDeleted("false");
        newGroup.setTitle("new Group2");
        BaseContact bContact1 = new BaseContact();
        bContact1.setDisplayName("displayName");
        bContact1.setModifyTime(146541654);
        bContact1.setPrimaryNumber("15858474521");
        bContact1.setSimName("sim1");
        ArrayList<BaseContact> members = new ArrayList<BaseContact>();
        members.add(bContact1);
        newGroup.setMembers(members);
        int updateRows = mContactsProxy.updateGroup(id, newGroup);
        assertTrue(updateRows > 0);

        // newGroup == null;
        newGroup = null;
        updateRows = mContactsProxy.updateGroup(id, newGroup);
        assertTrue(updateRows == 0);
    }

    /**
     * Test"int updateGroup(final long id, final Group newGroup, final String
     * oldName)".
     * 
     */
    public void test52_updateGroup() {
        Group group = new Group();
        group.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        group.setNotes("notes");
        group.setTitle("group5");
        long id = mContactsProxy.insertGroup(group);
        Group newGroup = new Group();
        group.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        group.setVersion(Config.VERSION_STRING);
        newGroup.setNotes("notes");
        newGroup.setTitle("new Group");
        int updateRows = mContactsProxy.updateGroup(id, newGroup, group
                .getTitle());
        assertTrue(updateRows > 0);
    }

    public void test53_updateGroupForRestore() {
        ArrayList<Group> groups = new ArrayList<Group>();
        Group group = new Group();
        group.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        group.setVersion(Config.VERSION_STRING);
        group.setDeleted("false");
        group.setDirty("true");
        group.setGroup_visible("true");
        group.setNotes("notes");
        group.setShould_sync("true");
        group.setTitle("group4");
        group.setId(1);
        Group group2 = new Group();
        group2.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM);
        group2.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        group2.setVersion(Config.VERSION_STRING);
        group2.setDeleted("false");
        group2.setDirty("true");
        group2.setGroup_visible("true");
        group2.setNotes("notes");
        group2.setShould_sync("true");
        group2.setTitle("group5");
        group2.setId(-255);
        groups.add(group);
        groups.add(group2);
//        int updateGroups = mContactsProxy.updateGroupForRestore(groups);
//        assertTrue(updateGroups > 0);
    }

    /**
     * Test"boolean[] deleteGroup(final long[] ids)".
     */
    public void test54_deleteGroup() {
        Group group = new Group();
        group.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        group.setVersion(Config.VERSION_STRING);
        group.setDeleted("false");
        group.setDirty("false");
        group.setGroup_visible("true");
        group.setNotes("notes");
        group.setShould_sync("false");
        group.setTitle("group3");
        Group group2 = new Group();
        group.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        group.setVersion(Config.VERSION_STRING);
        group2.setDeleted("true");
        group2.setDirty("false");
        group2.setGroup_visible("false");
        group2.setTitle("group3");
        group2.setShould_sync("true");
        ArrayList<BaseContact> members = new ArrayList<BaseContact>();
        BaseContact bContact1 = new BaseContact();
        bContact1.setDisplayName("displayName");
        bContact1.setModifyTime(146541654);
        bContact1.setPrimaryNumber("15858474521");
        bContact1.setSimName("sim1");
        members.add(bContact1);
        group2.setMembers(members);
        long ids[] = new long[2];
        ids[0] = mContactsProxy.insertGroup(group);
        ids[1] = mContactsProxy.insertGroup(group2);
        boolean[] result = mContactsProxy.deleteGroup(ids);
        assertEquals(2, result.length);
    }

    public void test55_deleteAllGroups() {
        Group group = new Group();
        group.setAccount_name("account1");
        group.setAccount_type("type");
        group.setDeleted("false");
        group.setDirty("false");
        group.setGroup_visible("true");
        group.setNotes("notes");
        mContactsProxy.insertGroup(group);
        int deleteGroups = mContactsProxy.deleteAllGroups();
        assertTrue(deleteGroups > 0);
    }

    public void test56_insertContactData() {
        RawContact rawContact = getRawContact();
        rawContact.setStoreLocation(RawContact.SOURCE_PHONE);
        long rawId = mContactsProxy.insertContact(rawContact, true);
        assertTrue("rawId" + rawId, rawId > 0);
        // ContactData data = new ContactData(DatabaseRecordEntity.ID_NULL,
        // Phone.MIME_TYPE, rawId, false, false, 0);
        Phone phone = new Phone();
        phone.setLabel("phone");
        phone.setMimeType(Phone.MIME_TYPE);
        phone.setNumber("12565251124");
        phone.setPrimary(true);
        phone.setRawContactId(rawId);
        phone.setType(Phone.TYPE_HOME);
        mContactsProxy.insertContactData(phone, true);

        Phone phone2 = null;
        long insertId = mContactsProxy.insertContactData(phone2, true);
        assertEquals(DatabaseRecordEntity.ID_NULL, insertId);
    }

    /**
     * Test "long[] insertGroupMembership(final long[] contactIds, final long
     * groupId)"
     */
    public void test57_insertGroupMembership() {
        RawContact rawContact = getRawContact();
        rawContact.setStoreLocation(RawContact.SOURCE_SIM1);
        long rawId = mContactsProxy.insertContact(rawContact, true);
        assertTrue("rawId" + rawId, rawId > 0);
        long[] contactId = { rawId };
        Group group = new Group();
        group.setAccount_name("account8" + USIMUtils.ACCOUNT_NAME_USIM);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        group.setVersion(Config.VERSION_STRING);
        group.setDeleted("true");
        group.setDirty("false");
        group.setGroup_visible("true");
        group.setNotes("notes");
        group.setShould_sync("false");
        group.setTitle("title1");
        ArrayList<BaseContact> members = new ArrayList<BaseContact>();
        BaseContact bContact1 = new BaseContact();
        bContact1.setDisplayName("displayName");
        bContact1.setModifyTime(146541654);
        bContact1.setPrimaryNumber("15858474521");
        bContact1.setSimName("sim1");
        members.add(bContact1);
        group.setMembers(members);
        long groupId = mContactsProxy.insertGroup(group);
        mContactsProxy.insertGroupMembership(contactId, groupId);
    }

    public void test58_isSyncNeedReinit() {
        assertNotNull(mContactsProxy.isSyncNeedReinit());
    }

    public void test59_getLastSyncDate() {
        assertNotNull(mContactsProxy.getLastSyncDate());
    }

    public void test60_updateSyncDate() {
        assertTrue(mContactsProxy.updateSyncDate(20120815));
    }

    public void test61_getMaxRawContactsId() {
        assertTrue(mContactsProxy.getMaxRawContactsId() >= 0);
    }

    public void test62_getMaxRawContactsIdByQuery() {
        assertTrue(mContactsProxy.getMaxRawContactsIdByQuery() >= 0);
    }

    public void test63_getSyncFlags() {
        long[] idSet = new long[10000];
        for (int i = 0; i < 10000; i++) {
            idSet[i] = i;
        }
        assertNotNull(mContactsProxy.getSyncFlags(idSet));
        // idSet == null;
        idSet = null;
        byte[] result = mContactsProxy.getSyncFlags(idSet);
        assertEquals(4, result.length);

        // idSet.length <= 0;
    }

    public void test64_getSimContactsCount() {
        assertTrue(mContactsProxy.getSimContactsCount() >= 0);
    }

    public void test65_getAvailableContactsCount() {
        assertTrue(mContactsProxy.getAvailableContactsCount() >= 0);
    }

    public void test66_deleteContactsSourcedOnPhone() {
        boolean withData = true;
        long id = -1;
        RawContact newContact = getRawContact();
        newContact.setStoreLocation(RawContact.SOURCE_PHONE);
        long[] ids = new long[10];
        for (int i = 0; i < 10; i++) {
            id = mContactsProxy.insertContact(newContact, withData);
            assertTrue(id >= 0);
            ids[i] = id;
        }
        boolean[] results = mContactsProxy.deleteContactsSourcedOnPhone(ids,
                true);
        assertEquals(10, results.length);
        // newContact == null;
        newContact = null;
        mContactsProxy.insertContact(newContact, withData);
    }

    public void test67_fastDeleteContactsSourcedOnPhone() {
        boolean withData = true;
        long id = -1;
        RawContact newContact = getRawContact();
        newContact.setStoreLocation(RawContact.SOURCE_PHONE);
        long[] ids = new long[10];
        for (int i = 0; i < 10; i++) {
            id = mContactsProxy.insertContact(newContact, withData);
            assertTrue(id >= 0);
            ids[i] = id;
        }
        int deleteCounts = mContactsProxy.fastDeleteContactsSourcedOnPhone(ids,
                true);
        assertEquals(10, deleteCounts);
    }

    /**
     * Test "boolean[] deleteContactsSourcedOnSim(final long[] ids, final
     * boolean permanently, final int sourceLocation, final String[] simNames,
     * final String[] simNumbers)".
     */
    public void test68_deleteContactsSourcedOnSim() {
        String[] simNames = { mSimContact.mName, mSimContact1.mName,
                mSimContact2.mName };
        String[] simNumbers = { mSimContact.mNumber, mSimContact1.mNumber,
                mSimContact2.mNumber };
        long id = -1;
        long[] ids = new long[3];
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                // insert 3 sim contacts.
                id = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, DataStoreLocations.SIM1);
                ids[0] = id;
                id = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, DataStoreLocations.SIM1);
                ids[1] = id;
                id = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, DataStoreLocations.SIM1);
                ids[2] = id;
                // delete the sim contacts.
                boolean[] results = mContactsProxy.deleteContactsSourcedOnSim(
                        ids, true, DataStoreLocations.SIM1, simNames,
                        simNumbers);
                assertEquals(3, results.length);
            }

            if (SystemInfoProxy.isSim2Accessible()) {
                // insert 3 sim contacts.
                id = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, DataStoreLocations.SIM2);
                ids[0] = id;
                id = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, DataStoreLocations.SIM2);
                ids[1] = id;
                id = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, DataStoreLocations.SIM2);
                ids[2] = id;
                // delete the sim contacts.
                boolean[] results = mContactsProxy.deleteContactsSourcedOnSim(
                        ids, true, DataStoreLocations.SIM1, simNames,
                        simNumbers);
                assertEquals(3, results.length);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                // insert 3 sim contacts.
                id = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, DataStoreLocations.SIM);
                ids[0] = id;
                id = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, DataStoreLocations.SIM);
                ids[1] = id;
                id = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, DataStoreLocations.SIM);
                ids[2] = id;
                // delete the sim contacts.
                boolean[] results = mContactsProxy.deleteContactsSourcedOnSim(
                        ids, true, DataStoreLocations.SIM1, simNames,
                        simNumbers);
                assertEquals(3, results.length);
            } else {
                Debugger.logI("[ContactsProxyTest]There is no sim card");
            }
        }
    }

    /**
     * Test "void asyncGetAllRawContacts(final IRawBlockConsumer consumer, final
     * ByteBuffer buffer)".
     */
    public void test69_asyncGetAllRawContacts() {
        IRawBlockConsumer consumer = new IRawBlockConsumer() {

            public void consume(byte[] block, int blockNo, int totalNo) {

            }

        };
        ByteBuffer buffer = Global.getByteBuffer();
        mContactsProxy.asyncGetAllRawContacts(consumer, buffer);
    }

    /**
     * Test "boolean[] deleteContactData(final long[] ids, final long groupId,
     * final Group group, final int[] simIndexes)"
     */
    public void test70_deleteContactData() {
        RawContact rawContact = getRawContact();
        long rawId = mContactsProxy.insertContact(rawContact, true);
        assertTrue("rawId" + rawId, rawId > 0);
        // ContactData data = new ContactData(DatabaseRecordEntity.ID_NULL,
        // Phone.MIME_TYPE, rawId, false, false, 0);
        Phone data = new Phone();
        data.setMimeType(Phone.MIME_TYPE);
        data.setRawContactId(rawId);
        data.setNumber("1254656556666");
        data.setType(Phone.TYPE_COMPANY_MAIN);
        data.setLabel("zhangsan");
        boolean validateContactId = false;
        long insertId = mContactsProxy.insertContactData(data,
                validateContactId);
        assertTrue("insertId = " + insertId, insertId >= 0);
        Phone newData = new Phone();
        newData.setLabel("lisi");
        newData.setMimeType(Phone.MIME_TYPE);
        newData.setNumber("13985475847");
        newData.setType(Phone.TYPE_HOME);
        newData.setRawContactId(rawId);
        int updateRows = mContactsProxy.updateContactData(insertId, newData,
                true);
        assertTrue(updateRows > 0);

        Im im = new Im();
        im.setData("464614616");
        im.setLabel("QQ");
        im.setMimeType(Im.MIME_TYPE);
        im.setProtocol(Im.PROTOCOL_QQ);
        im.setType(Im.TYPE_NONE);
        mContactsProxy.updateContactData(insertId, im, true);

        Email email = new Email();
        email.setData("test@qq.com");
        email.setLabel("my Email");
        email.setMimeType(Email.MIME_TYPE);
        email.setType(Email.TYPE_WORK);
        mContactsProxy.updateContactData(insertId, email, true);

        StructuredPostal postal = new StructuredPostal();
        postal.setCity("chengdu");
        postal.setCountry("china");
        postal.setFormattedAddress("sichuan chengdu");
        postal.setLabel("my postal");
        postal.setMimeType(StructuredPostal.MIME_TYPE);
        postal.setPostcode("1456465");
        postal.setType(StructuredPostal.TYPE_HOME);
        mContactsProxy.updateContactData(insertId, postal, true);

        Organization organization = new Organization();
        organization.setCompany("company");
        organization.setDepartment("department");
        organization.setJobDescription("jobDescription");
        organization.setLabel("label");
        organization.setMimeType(Organization.MIME_TYPE);
        organization.setOfficeLocation("officeLocation");
        organization.setPhoneticName("phoneticName");
        organization.setPhoneticNameStyle("phoneticNameStyle");
        organization.setTitle("my organization");
        organization.setType(Organization.TYPE_WORK);
        mContactsProxy.updateContactData(insertId, organization, true);

        Nickname nickName = new Nickname();
        nickName.setLabel("my nickName");
        nickName.setMimeType(Nickname.MIME_TYPE);
        nickName.setName("name");
        nickName.setType(Nickname.TYPE_INITIALS);
        mContactsProxy.updateContactData(insertId, nickName, true);

        Website website = new Website();
        website.setLabel("my website");
        website.setMimeType(Website.MIME_TYPE);
        website.setType(Website.TYPE_HOME);
        website.setUrl("www.baidu.com");
        mContactsProxy.updateContactData(insertId, website, true);

        Note note = new Note();
        note.setMimeType(Note.MIME_TYPE);
        note.setNote("a note");
        mContactsProxy.updateContactData(insertId, note, true);

        boolean[] result = null;
        Group group = ContactsUtils.getGroup();
        group.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        mContactsProxy.insertGroup(group);
        long[] ids = { insertId };
        result = mContactsProxy.deleteContactData(ids, 1, group,
                new int[] { 1 });
        assertNotNull(result);
        group.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM2);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        result = mContactsProxy.deleteContactData(ids, 1, group,
                new int[] { 1 });
        assertNotNull(result);
        result = mContactsProxy.deleteContactData(null, 1, group,
                new int[] { 1 });
        assertNull(result);
        long[] dataIds = new long[2];
        for (int i = 0; i < 2; i++) {
            dataIds[i] = i;
        }
        mContactsProxy.deleteContactData(dataIds);
    }

    /**
     * Test "boolean[] deleteContactsSourcedOnSim(final long[] ids, final
     * boolean permanently, final int sourceLocation, final String[] simNames,
     * final String[] simNumbers, final String[] emails)"
     */
    public void test71_deleteContactsSourcedOnSim() {
        String[] simNames = { mSimContact.mName, mSimContact1.mName,
                mSimContact2.mName };
        String[] simNumbers = { mSimContact.mNumber, mSimContact1.mNumber,
                mSimContact2.mNumber };
        String[] emails = { mSimContact.mEmail, mSimContact1.mEmail,
                mSimContact2.mEmail };
        long id = -1;
        long[] ids = new long[3];
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                // insert 3 sim contacts.
                id = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, DataStoreLocations.SIM1);
                ids[0] = id;
                id = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, DataStoreLocations.SIM1);
                ids[1] = id;
                id = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, DataStoreLocations.SIM1);
                ids[2] = id;
                // delete the sim contacts.
                boolean[] results = mContactsProxy.deleteContactsSourcedOnSim(
                        ids, true, DataStoreLocations.SIM1, simNames,
                        simNumbers, emails);
                assertEquals(3, results.length);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                // insert 3 sim contacts.
                id = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, DataStoreLocations.SIM2);
                ids[0] = id;
                id = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, DataStoreLocations.SIM2);
                ids[1] = id;
                id = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, DataStoreLocations.SIM2);
                ids[2] = id;
                // delete the sim contacts.
                boolean[] results = mContactsProxy.deleteContactsSourcedOnSim(
                        ids, true, DataStoreLocations.SIM1, simNames,
                        simNumbers, emails);
                assertEquals(3, results.length);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                // insert 3 sim contacts.
                id = mContactsProxy.insertSimContact(mSimContact.mName,
                        mSimContact.mNumber, DataStoreLocations.SIM);
                ids[0] = id;
                id = mContactsProxy.insertSimContact(mSimContact1.mName,
                        mSimContact1.mNumber, DataStoreLocations.SIM);
                ids[1] = id;
                id = mContactsProxy.insertSimContact(mSimContact2.mName,
                        mSimContact2.mNumber, DataStoreLocations.SIM);
                ids[2] = id;
                // delete the sim contacts.
                boolean[] results = mContactsProxy.deleteContactsSourcedOnSim(
                        ids, true, DataStoreLocations.SIM1, simNames,
                        simNumbers, emails);
                assertEquals(3, results.length);
            } else {
                Debugger.logI("[ContactsProxyTest]There is no sim card");
            }
        }
    }

    /**
     * Test "int deleteGroup(final long id, final Group group)".
     */
    public void test72_deleteGroup() {
        Group group = new Group();
        group.setAccount_name("account");
        group.setAccount_type("type");
        group.setDeleted("false");
        group.setDirty("false");
        group.setGroup_visible("true");
        group.setNotes("notes");
        group.setShould_sync("false");
        group.setTitle("classmates");
        ArrayList<BaseContact> members = new ArrayList<BaseContact>();
        BaseContact bContact1 = new BaseContact();
        bContact1.setDisplayName("displayName");
        bContact1.setModifyTime(146541654);
        bContact1.setPrimaryNumber("15858474521");
        bContact1.setSimName("sim1");
        members.add(bContact1);
        group.setMembers(members);
        long id;
        int deleteRows;
        id = mContactsProxy.insertGroup(group);
        assertTrue(id >= 0);
        deleteRows = mContactsProxy.deleteGroup(id, group);
        assertTrue(deleteRows >= 0);

        group = ContactsUtils.getGroup();
        id = mContactsProxy.insertGroup(group);
        assertTrue(id >= 0);
        deleteRows = mContactsProxy.deleteGroup(id, group);
        assertTrue(deleteRows >= 0);

        group.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        id = mContactsProxy.insertGroup(group);
        assertTrue(id >= 0);
        deleteRows = mContactsProxy.deleteGroup(id, group);
        assertTrue(deleteRows >= 0);

        group.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM2);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        id = mContactsProxy.insertGroup(group);
        assertTrue(id >= 0);
        deleteRows = mContactsProxy.deleteGroup(id, group);
        assertTrue(deleteRows >= 0);
    }

    public void test73_fastImportDetailedContacts() {

        IRawBlockConsumer rawContactsConsumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        IRawBlockConsumer contactDataConsumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer outBuffer = Global.getByteBuffer();
        ByteBuffer buffer = Global.getByteBuffer();
        RawContact rawContact = getRawContact();
        byte[] raw = null;

        buffer.putInt(1);
        rawContact.writeRawWithVersion(buffer, Config.VERSION_CODE);
        buffer.position(0);
        raw = buffer.array();

        mContactsProxy.fastImportDetailedContacts(null, rawContactsConsumer,
                contactDataConsumer, outBuffer);
        mContactsProxy.fastImportDetailedContacts(raw, null,
                contactDataConsumer, outBuffer);
        mContactsProxy.fastImportDetailedContacts(raw, rawContactsConsumer,
                null, outBuffer);
        mContactsProxy.fastImportDetailedContacts(raw, rawContactsConsumer,
                contactDataConsumer, outBuffer);

        outBuffer.clear();
        buffer.clear();
        rawContact.setSourceLocation(RawContact.SOURCE_SIM);
        buffer.putInt(1);
        rawContact.writeRawWithVersion(buffer, Config.VERSION_CODE);
        buffer.position(0);
        raw = buffer.array();
        mContactsProxy.fastImportDetailedContacts(raw, rawContactsConsumer,
                contactDataConsumer, outBuffer);

        outBuffer.clear();
        buffer.clear();
        rawContact.setSourceLocation(RawContact.SOURCE_SIM1);
        buffer.putInt(1);
        rawContact.writeRawWithVersion(buffer, Config.VERSION_CODE);
        buffer.position(0);
        raw = buffer.array();
        mContactsProxy.fastImportDetailedContacts(raw, rawContactsConsumer,
                contactDataConsumer, outBuffer);

        outBuffer.clear();
        buffer.clear();
        rawContact.setSourceLocation(RawContact.SOURCE_SIM2);
        buffer.putInt(1);
        rawContact.writeRawWithVersion(buffer, Config.VERSION_CODE);
        buffer.position(0);
        raw = buffer.array();
        mContactsProxy.fastImportDetailedContacts(raw, rawContactsConsumer,
                contactDataConsumer, outBuffer);

        outBuffer.clear();
        mContactsProxy.fastImportDetailedContacts(byteContact,
                rawContactsConsumer, contactDataConsumer, outBuffer);
    }

    public void test74_restoreDetailedContacts() {

        IRawBlockConsumer rawContactsConsumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        IRawBlockConsumer contactDataConsumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer outBuffer = Global.getByteBuffer();
        ByteBuffer buffer = Global.getByteBuffer();
        RawContact rawContact = getRawContact();
        byte[] raw = null;

        buffer.putInt(1);
        rawContact.writeRawWithVersion(buffer, Config.VERSION_CODE);
        buffer.position(0);
        raw = buffer.array();
        mContactsProxy.restoreDetailedContacts(raw, rawContactsConsumer,
                contactDataConsumer, outBuffer);
        mContactsProxy.restoreDetailedContacts(null, rawContactsConsumer,
                contactDataConsumer, outBuffer);
        mContactsProxy.restoreDetailedContacts(raw, null, contactDataConsumer,
                outBuffer);

        outBuffer.clear();
        buffer.clear();
        rawContact.setSourceLocation(RawContact.SOURCE_SIM);
        buffer.putInt(1);
        rawContact.writeRawWithVersion(buffer, Config.VERSION_CODE);
        buffer.position(0);
        raw = buffer.array();
        mContactsProxy.restoreDetailedContacts(raw, rawContactsConsumer,
                contactDataConsumer, outBuffer);

        outBuffer.clear();
        buffer.clear();
        rawContact.setSourceLocation(RawContact.SOURCE_SIM1);
        buffer.putInt(1);
        rawContact.writeRawWithVersion(buffer, Config.VERSION_CODE);
        buffer.position(0);
        raw = buffer.array();
        mContactsProxy.restoreDetailedContacts(raw, rawContactsConsumer,
                contactDataConsumer, outBuffer);

        outBuffer.clear();
        buffer.clear();
        rawContact.setSourceLocation(RawContact.SOURCE_SIM2);
        buffer.putInt(1);
        rawContact.writeRawWithVersion(buffer, Config.VERSION_CODE);
        buffer.position(0);
        raw = buffer.array();
        mContactsProxy.restoreDetailedContacts(raw, rawContactsConsumer,
                contactDataConsumer, outBuffer);

        outBuffer.clear();
        mContactsProxy.restoreDetailedContacts(byteContact,
                rawContactsConsumer, contactDataConsumer, outBuffer);
    }

    public void test75_slowSyncAddDetailedContacts() {
        RawContact rawContact = getRawContact();
        ByteBuffer buffer = Global.getByteBuffer();
        rawContact.writeRawWithVersion(buffer, Config.VERSION_CODE);
        buffer.position(0);
        byte[] raw = null;
        try {
            raw = buffer.array();
        } catch (UnsupportedOperationException e) {
            fail(e.getMessage());
        }
        mContactsProxy.slowSyncAddDetailedContacts(raw);
    }

    /**
     * Test
     * "boolean[] deleteGroup(final long[] ids, final ArrayList<Group> groups)".
     */
    public void test76_deleteGroup() {
        Group group = new Group();
        group.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        group.setDeleted("false");
        group.setDirty("false");
        group.setGroup_visible("true");
        group.setNotes("notes");
        group.setShould_sync("false");
        group.setTitle("classmates");
        Group group2 = new Group();
        group2.setAccount_name("accout2");
        group2.setAccount_type("2");
        group2.setDeleted("true");
        group2.setDirty("false");
        group2.setGroup_visible("false");
        group2.setTitle("group2");
        ArrayList<BaseContact> members = new ArrayList<BaseContact>();
        BaseContact bContact1 = new BaseContact();
        bContact1.setDisplayName("displayName");
        bContact1.setModifyTime(146541654);
        bContact1.setPrimaryNumber("15858474521");
        bContact1.setSimName("sim1");
        members.add(bContact1);
        group2.setMembers(members);
        long ids[] = new long[2];
        ArrayList<Group> groups = new ArrayList<Group>();
        groups.add(group);
        groups.add(group2);
        ids[0] = mContactsProxy.insertGroup(group);
        ids[1] = mContactsProxy.insertGroup(group2);
        boolean[] result = mContactsProxy.deleteGroup(ids, groups);
        assertEquals(2, result.length);
        result = mContactsProxy.deleteGroup(null, groups);
        assertNull(result);
    }

    public void test77_fastSyncGetContactData() {
        boolean withData = false;
        long id = -1;
        RawContact newContact = getRawContact();
        long[] ids = new long[10];
        for (int i = 0; i < 10; i++) {
            id = mContactsProxy.insertContact(newContact, withData);
            assertTrue(id >= 0);
            ids[i] = id;
        }
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        mContactsProxy.fastSyncGetContactData(ids, consumer, buffer);
    }

    public void test78_fastSyncGetRawContacts() {
        boolean withData = false;
        long id = -1;
        RawContact newContact = getRawContact();
        long[] ids = new long[10];
        for (int i = 0; i < 10; i++) {
            id = mContactsProxy.insertContact(newContact, withData);
            assertTrue(id >= 0);
            ids[i] = id;
        }
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {

            }
        };
        ByteBuffer buffer = Global.getByteBuffer();
        mContactsProxy.fastSyncGetRawContacts(ids, consumer, buffer);
    }

    public void test79_fastSyncUpdateDetailedContacts() {
        RawContact rawContact = getRawContact();
        long rawId = mContactsProxy.insertContact(rawContact, true);
        assertTrue("rawId" + rawId, rawId > 0);
        rawContact.setContactId(rawId);
        rawContact.setDisplayName("testContacts");
        rawContact.setSourceLocation(RawContact.SOURCE_PHONE);
        ByteBuffer buffer = Global.getByteBuffer();
        rawContact.writeRawWithVersion(buffer, Config.VERSION_CODE);
        buffer.position(0);
        byte[] raw = buffer.array();
        byte[] syncResultsRaw = mContactsProxy
                .fastSyncUpdateDetailedContacts(raw);
        assertNotNull(syncResultsRaw);
        syncResultsRaw = mContactsProxy
                .fastSyncUpdateDetailedContacts(byteContact);

        // raw == null.
        raw = null;
        byte[] ResultsRaw = mContactsProxy.fastSyncUpdateDetailedContacts(raw);
        assertNull(ResultsRaw);
    }

    public void test80_updateGroupForRestore() {
        Group group = new Group();
        group.setAccount_name("account1");
        group.setAccount_type("type");
        group.setDeleted("false");
        group.setDirty("false");
        group.setGroup_visible("true");
        group.setNotes("notes");
        group.setShould_sync("false");
        group.setTitle("classmates");
        Group group2 = new Group();
        group2.setAccount_name("accout2");
        group2.setAccount_type("2");
        group2.setDeleted("true");
        group2.setDirty("false");
        group2.setGroup_visible("false");
        group2.setTitle("group2");
        ArrayList<Group> groups = new ArrayList<Group>();
        mContactsProxy.updateGroupForRestore(groups);
    }

    /**
     * Test "long[] insertGroupMembership(final long[] contactIds, final Group
     * group)".
     */
    public void test81_insertGroupMembership() {
        RawContact rawContact = getRawContact();
        long rawId = mContactsProxy.insertContact(rawContact, true);
        assertTrue("rawId" + rawId, rawId > 0);
        long[] contactIds = { rawId };
        Group group = new Group();
        group.setAccount_name("account1");
        group.setAccount_type("type");
        group.setDeleted("false");
        group.setDirty("false");
        group.setGroup_visible("true");
        group.setNotes("notes");
        group.setShould_sync("false");
        group.setTitle("classmates");
        ArrayList<BaseContact> members = new ArrayList<BaseContact>();
        BaseContact bContact1 = new BaseContact();
        bContact1.setDisplayName("displayName");
        bContact1.setModifyTime(146541654);
        bContact1.setPrimaryNumber("15858474521");
        bContact1.setSimName("sim1");
        members.add(bContact1);
        group.setMembers(members);
        long[] results = null;
        results = mContactsProxy.insertGroupMembership(contactIds, group);
        assertNotNull(results);

        results = mContactsProxy.insertGroupMembership(null, group);
        assertNull(results);

        group.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        results = mContactsProxy.insertGroupMembership(contactIds, group);
        assertNotNull(results);

        group.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM2);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        results = mContactsProxy.insertGroupMembership(contactIds, group);
        assertNotNull(results);
    }

    public void test82_getAvailableContactsCount2() {
        int counts = mContactsProxy.getAvailableContactsCount2();
        assertTrue(counts > 0);
    }

    public void test83_insertGroupMembership() {
        RawContact rawContact = getRawContact();
        rawContact.setStoreLocation(RawContact.SOURCE_SIM1);
        long rawId = mContactsProxy.insertContact(rawContact, true);
        assertTrue("rawId" + rawId, rawId > 0);
        RawContact rawContact2 = mContactsProxy.getContact(rawId, true);
        int simIndex = rawContact2.getSimIndex();
        System.out.println(TAG + "simIdex:" + simIndex);
        Log.i(TAG, "sim index: " + simIndex);
        Log.e(TAG, "sim index: " + simIndex);

        long[] contactIds = { rawId };
        Group group = new Group();
        group.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        group.setDeleted("false");
        group.setDirty("false");
        group.setGroup_visible("true");
        group.setNotes("notes");
        group.setShould_sync("false");
        group.setTitle("classmates");
        ArrayList<BaseContact> members = new ArrayList<BaseContact>();
        BaseContact bContact1 = new BaseContact();
        bContact1.setDisplayName("displayName");
        bContact1.setModifyTime(146541654);
        bContact1.setPrimaryNumber("15858474521");
        bContact1.setSimName("sim1");
        members.add(bContact1);
        group.setMembers(members);

        long groupId = mContactsProxy.insertGroup(group);
        final int[] simIndexes = { simIndex };
        long[] result;
        result = mContactsProxy.insertGroupMembership(contactIds, groupId,
                group, simIndexes);
        assertNotNull(result);
        group.setAccount_name(USIMUtils.ACCOUNT_NAME_USIM2);
        result = mContactsProxy.insertGroupMembership(contactIds, groupId,
                group, simIndexes);
        assertNotNull(result);

        // contactIds == null.
        contactIds = null;
        result = mContactsProxy.insertGroupMembership(contactIds, groupId,
                group, simIndexes);
        assertNull(result);

        // group == null.
        long[] contactIds1 = { rawId };
        group = null;
        result = mContactsProxy.insertGroupMembership(contactIds1, groupId,
                group, simIndexes);
        assertNotNull(result);
    }

    public void test84_slowSyncGetAllContactData() {
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {
            }

        };
        ByteBuffer buffer = Global.getByteBuffer();
        mContactsProxy.slowSyncGetAllContactData(0, consumer, buffer);
    }

    public void test85_slowSyncGetAllRawContacts() {
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {
            }

        };
        ByteBuffer buffer = Global.getByteBuffer();
        mContactsProxy.slowSyncGetAllRawContacts(0, consumer, buffer);
    }

    public void test86_fastSyncGetAllSyncFlags() {
        IRawBlockConsumer consumer = new IRawBlockConsumer() {
            public void consume(byte[] block, int blockNo, int totalNo) {
            }

        };
        ByteBuffer buffer = Global.getByteBuffer();
        mContactsProxy.fastSyncGetAllSyncFlags(consumer, buffer);
    }

    public void test87_getRawContactsCount() {
        int counts = mContactsProxy.getRawContactsCount();
        assertTrue(counts > 0);
    }

    public void test88_fastSyncAddDetailedContacts() {
        mContactsProxy.fastSyncAddDetailedContacts(byteContact);
    }

    public void test89_fastSyncDeleteContacts() {
        long[] ids = new long[10000];
        for (int i = 0; i < 10000; i++) {
            ids[0] = 0;
        }
        mContactsProxy.fastSyncDeleteContacts(ids);
    }

    public void test90_deleteContacts() {
        mContactsProxy.deleteAllContacts(true);
        mContactsProxy.deleteAllGroups();
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

    private RawContact getRawContact() {
        RawContact rawContact = new RawContact();
        rawContact.setLastTimeContacted(557668675);
        rawContact.setStarred(true);
        rawContact.setCustomRingtone("custom ringtone");
        rawContact.setSendToVoicemail(false);
        rawContact.setVersion(2);
        rawContact.setDirty(true);
        rawContact.setSourceLocation(RawContact.SOURCE_PHONE);
        rawContact.setSimName("China mobile");
        rawContact.setSimId(-1);
        rawContact.setDisplayName("Test 01");

        ArrayList<StructuredName> names = new ArrayList<StructuredName>();
        StructuredName name = new StructuredName();
        name.setMimeType(StructuredName.MIME_TYPE);
        name.setDisplayName("displayName");
        name.setFamilyName("familyName");
        name.setGivenName("givenName");
        names.add(name);
        rawContact.setNames(names);
        ArrayList<Phone> phones = new ArrayList<Phone>();
        Phone phone = new Phone();
        phone.setNumber(mSimContact.mNumber);
        phone.setLabel("a phone num");
        phone.setMimeType(Phone.MIME_TYPE);
        phone.setType(Phone.TYPE_HOME);
        phones.add(phone);
        rawContact.setPhones(phones);
        ArrayList<Photo> photos = new ArrayList<Photo>();
        rawContact.setPhotos(photos);
        ArrayList<Email> emails = new ArrayList<Email>();
        Email email = new Email();
        email.setMimeType(Email.MIME_TYPE);
        email.setData(mSimContact.mEmail);
        email.setLabel("meeting");
        email.setType(Email.TYPE_WORK);
        emails.add(email);
        rawContact.setEmails(emails);
        ArrayList<Im> ims = new ArrayList<Im>();
        Im im = new Im();
        im.setMimeType(Im.MIME_TYPE);
        im.setData("5459655616");
        im.setLabel("QQ");
        im.setProtocol(Im.PROTOCOL_QQ);
        im.setType(Im.TYPE_HOME);
        im.setCustomProtocol(Im.MIME_TYPE_STRING);
        ims.add(im);
        rawContact.setIms(ims);
        ArrayList<StructuredPostal> postals = new ArrayList<StructuredPostal>();
        StructuredPostal postal = new StructuredPostal();
        postal.setMimeType(StructuredPostal.MIME_TYPE);
        postal.setCity("chengdu");
        postal.setCountry("China");
        postal.setLabel("Home");
        postal.setFormattedAddress(StructuredPostal.MIME_TYPE_STRING);
        postal.setNeighborhood("neighborhood");
        postal.setPostcode("11655");
        postal.setRegion("sichuang");
        postal.setStreet("xinhua street");
        postal.setType(StructuredPostal.TYPE_WORK);
        postals.add(postal);
        rawContact.setPostals(postals);
        ArrayList<Organization> organizations = new ArrayList<Organization>();
        rawContact.setOrganizations(organizations);
        ArrayList<Note> notes = new ArrayList<Note>();
        Note note = new Note();
        note.setMimeType(Note.MIME_TYPE);
        note.setNote("it's a note");
        notes.add(note);
        rawContact.setNotes(notes);
        ArrayList<Nickname> nicknames = new ArrayList<Nickname>();
        Nickname nickname = new Nickname();
        nickname.setLabel("a nickname");
        nickname.setMimeType(Nickname.MIME_TYPE);
        nickname.setName("nickname");
        nickname.setType(Nickname.TYPE_SHORT_NAME);
        nicknames.add(nickname);
        rawContact.setNicknames(nicknames);
        ArrayList<Website> websites = new ArrayList<Website>();
        Website website = new Website();
        website.setLabel("a website");
        website.setMimeType(Website.MIME_TYPE);
        website.setType(Website.TYPE_WORK);
        website.setUrl("www.mediatek.com");
        websites.add(website);
        rawContact.setWebsites(websites);
        ArrayList<GroupMembership> groupMemberships = new ArrayList<GroupMembership>();
        GroupMembership groupMembership = new GroupMembership();
        groupMembership.setGroupId(0);
        groupMemberships.add(groupMembership);
        rawContact.setGroupMemberships(groupMemberships);
        return rawContact;
    }

    private byte[] byteContact = { 0, 0, 0, 1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, -1, -1,
            -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, 0, 1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,
            0, 0, 9, 0, 0, 0, 10, 0, 50, 0, 51, 0, 49, 0, 50, 0, 51, 0, 32, 0,
            49, 0, 50, 0, 51, 0, 49, 0, 0, 0, 5, 0, 50, 0, 51, 0, 49, 0, 50, 0,
            51, 0, 0, 0, 0, 0, 0, 0, 4, 0, 49, 0, 50, 0, 51, 0, 49, 0, 0, 0, 0,
            0, 0, 0, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0,
            0, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, 0, 0, 0, 7, 0, 0, 0, 3, 0, 49, 0, 50, 0, 51, 0, 0, 0, 1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, 0, 0, 0, 7, 0, 0, 0, 8, 0, 49, 0, 50, 0,
            51, 0, 49, 0, 50, 0, 51, 0, 49, 0, 50, 0, 0, 0, 2, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, 0, 0, 0, 7, 0, 0, 0, 6, 0, 49, 0, 50, 0, 51, 0, 49, 0,
            50, 0, 51, 0, 0, 0, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 7, 0,
            0, 0, 7, 0, 49, 0, 50, 0, 51, 0, 49, 0, 50, 0, 51, 0, 49, 0, 0, 0,
            16, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 3, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 1,
            0, 0, 0, 4, 0, 49, 0, 50, 0, 51, 0, 49, 0, 0, 0, 1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,
            0, 0, 1, 0, 0, 0, 6, 0, 49, 0, 50, 0, 51, 0, 49, 0, 50, 0, 51, 0,
            0, 0, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, 0, 0, 0, 1, 0, 0, 0, 9, 0, 49, 0, 50, 0,
            51, 0, 49, 0, 50, 0, 51, 0, 49, 0, 50, 0, 51, 0, 0, 0, 0, 0, 0, 0,
            9, 0, 49, 0, 50, 0, 51, 0, 49, 0, 50, 0, 51, 0, 49, 0, 50, 0, 51,
            0, 0, 0, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 49, 0, 50, 0, 51, 0, 0, 0, 3,
            -1, -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, 0, 4, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 3, 0,
            0, 0, 17, 0, 50, 0, 51, 0, 49, 0, 50, 0, 51, 0, 32, 0, 49, 0, 50,
            0, 51, 0, 32, 0, 49, 0, 50, 0, 51, 0, 32, 0, 49, 0, 50, 0, 51, 0,
            0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 5, 0, 50, 0, 51, 0, 49, 0, 50, 0, 51,
            0, 0, 0, 0, -1, -1, -1, -1, 0, 0, 0, 3, 0, 49, 0, 50, 0, 51, 0, 0,
            0, 3, 0, 49, 0, 50, 0, 51, 0, 0, 0, 3, 0, 49, 0, 50, 0, 51, 0, 0,
            0, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, 0, 0, 0, 3, 0, 0, 0, 21, 0, 49, 0, 50, 0, 51, 0, 49, 0, 32, 0,
            50, 0, 51, 0, 49, 0, 50, 0, 51, 0, 49, 0, 32, 0, 49, 0, 50, 0, 51,
            0, 49, 0, 50, 0, 51, 0, 32, 0, 50, 0, 51, 0, 0, 0, 2, 0, 0, 0, 0,
            0, 0, 0, 4, 0, 49, 0, 50, 0, 51, 0, 49, 0, 0, 0, 0, -1, -1, -1, -1,
            0, 0, 0, 6, 0, 50, 0, 51, 0, 49, 0, 50, 0, 51, 0, 49, 0, 0, 0, 6,
            0, 49, 0, 50, 0, 51, 0, 49, 0, 50, 0, 51, 0, 0, 0, 2, 0, 50, 0, 51,
            0, 0, 0, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, 0, 0, 0, 3, 0, 0, 0, 23, 0, 49, 0, 50, 0, 51, 0, 32, 0, 49,
            0, 50, 0, 51, 0, 49, 0, 50, 0, 51, 0, 32, 0, 49, 0, 50, 0, 51, 0,
            49, 0, 50, 0, 32, 0, 51, 0, 49, 0, 50, 0, 51, 0, 49, 0, 50, 0, 0,
            0, 3, 0, 0, 0, 0, 0, 0, 0, 3, 0, 49, 0, 50, 0, 51, 0, 0, 0, 0, -1,
            -1, -1, -1, 0, 0, 0, 6, 0, 49, 0, 50, 0, 51, 0, 49, 0, 50, 0, 51,
            0, 0, 0, 5, 0, 49, 0, 50, 0, 51, 0, 49, 0, 50, 0, 0, 0, 6, 0, 51,
            0, 49, 0, 50, 0, 51, 0, 49, 0, 50, 0, 0, 0, 0, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 3, 0, 0, 0,
            18, 0, 49, 0, 50, 0, 51, 0, 32, 0, 49, 0, 50, 0, 51, 0, 32, 0, 49,
            0, 50, 0, 51, 0, 32, 0, 49, 0, 50, 0, 51, 0, 49, 0, 50, 0, 51, 0,
            0, 0, 0, 0, 0, 0, 4, 0, 50, 0, 51, 0, 51, 0, 51, 0, 0, 0, 3, 0, 49,
            0, 50, 0, 51, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, 0, 3, 0, 49, 0, 50,
            0, 51, 0, 0, 0, 3, 0, 49, 0, 50, 0, 51, 0, 0, 0, 6, 0, 49, 0, 50,
            0, 51, 0, 49, 0, 50, 0, 51, 0, 0, 0, 0, 0, 0, 0, 3, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 8, 0, 0,
            0, 3, 0, 49, 0, 50, 0, 51, 0, 0, 0, 1, -1, -1, -1, -1, 0, 0, 0, 3,
            0, 49, 0, 50, 0, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 8, 0, 0, 0, 5, 0, 49, 0,
            50, 0, 51, 0, 49, 0, 50, 0, 0, 0, 2, -1, -1, -1, -1, 0, 0, 0, 7, 0,
            51, 0, 49, 0, 50, 0, 51, 0, 49, 0, 50, 0, 51, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 8,
            0, 0, 0, 4, 0, 51, 0, 50, 0, 52, 0, 50, 0, 0, 0, 0, 0, 0, 0, 6, 0,
            50, 0, 51, 0, 52, 0, 50, 0, 51, 0, 52, 0, 0, 0, 5, 0, 51, 0, 52, 0,
            50, 0, 51, 0, 52, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 6, 0, 0, 0, 12, 0,
            49, 0, 50, 0, 51, 0, 49, 0, 50, 0, 51, 0, 49, 0, 50, 0, 51, 0, 49,
            0, 50, 0, 51, 0, 0, 0, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, 0, 0, 0, 4, 0, 0, 0, 6, 0, 49, 0, 50, 0,
            51, 0, 49, 0, 50, 0, 51, -1, -1, -1, 1, -1, -1, -1, -1, 0, 0, 0, 1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,
            0, 0, 5, 0, 0, 0, 14, 0, 104, 0, 116, 0, 116, 0, 112, 0, 58, 0, 47,
            0, 47, 0, 49, 0, 50, 0, 51, 0, 46, 0, 99, 0, 111, 0, 109, -1, -1,
            -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1 };
}
