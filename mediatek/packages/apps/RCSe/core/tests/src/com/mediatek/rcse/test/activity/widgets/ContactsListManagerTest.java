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

package com.mediatek.rcse.test.activity.widgets;

import android.test.AndroidTestCase;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;

import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.mediatek.rcse.activities.RcsContact;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.test.Utils;

 /**
 * This class is used to test the functions of ContactsListManager.
 */
public class ContactsListManagerTest extends AndroidTestCase {
    private static final String TAG = "ContactsListManagerTest";

    private static final String MOCK_CONTACT_PHONE = "+8618612345678";
    private static final String MOCK_CONTACT_NAME = "678";
    private static final String MOCK_SORTKEY1 = "Key1";
    private static final long MOCK_PHONEID1 = 100010;
    private static final short MOCK_CONTACTSID1 = (short) 201;

    private static final String MOCK_CONTACT_PHONE2 = "+8618612345679";
    private static final String MOCK_CONTACT_NAME2 = "679";
    private static final String MOCK_SORTKEY2 = "Key2";
    private static final long MOCK_PHONEID2 = 1000101;
    private static final short MOCK_CONTACTSID2 = (short) 202;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        ContactsManager.createInstance(getContext());
        ContactsListManager.initialize(getContext());
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    /**
     * Test case for method getPhoneIdTobeShow.
     */
    public void testCase01_getPhoneIdTobeShow() throws Throwable, Exception {
        ContactsManager.getInstance().setImBlockedForContact(MOCK_CONTACT_PHONE2, true);

        RcsContact rcsContact1 = new RcsContact(MOCK_CONTACT_NAME, MOCK_CONTACT_PHONE,
                MOCK_SORTKEY1, MOCK_PHONEID1, MOCK_CONTACTSID1);
        RcsContact rcsContact2 = new RcsContact(MOCK_CONTACT_NAME2, MOCK_CONTACT_PHONE2,
                MOCK_SORTKEY2, MOCK_PHONEID2, MOCK_CONTACTSID2);
        ContactsListManager.getInstance().CONTACTS_LIST.add(rcsContact1);
        ContactsListManager.getInstance().CONTACTS_LIST.add(rcsContact2);
        
        List<Participant> participantList = new ArrayList<Participant>();
        Participant participant = new Participant(MOCK_CONTACT_PHONE2, MOCK_CONTACT_NAME2);
        participantList.add(participant);
        
        long[] phoneIds = ContactsListManager.getInstance().getPhoneIdTobeShow(participantList);
        assertNotNull(participant);
        assertTrue(phoneIds.length >= 1);
    }
    
    /**
     * Test case for method getPhoneNumberByPhoneId.
     */
    public void testCase02_getPhoneNumberByPhoneId() throws Throwable, Exception {
        RcsContact rcsContact1 = new RcsContact(MOCK_CONTACT_NAME, MOCK_CONTACT_PHONE,
                MOCK_SORTKEY1, MOCK_PHONEID1, MOCK_CONTACTSID1);
        
        ContactsListManager.getInstance().CONTACTS_LIST.add(rcsContact1);
        String phoneNumber = ContactsListManager.getInstance().getPhoneNumberByPhoneId(MOCK_PHONEID1);
        assertNotNull(phoneNumber);
        assertTrue(phoneNumber.equals(MOCK_CONTACT_PHONE));
    }
    
    /**
     * Test case for method setStrangerList.
     */
    public void testCase03_setStrangerList() throws Throwable, Exception {
        ContactsListManager.getInstance().STRANGER_LIST.clear();
        
        ContactsListManager.getInstance().setStrangerList(MOCK_CONTACT_PHONE, true);
        assertEquals(1, ContactsListManager.getInstance().STRANGER_LIST.size());
        ContactsListManager.getInstance().setStrangerList(MOCK_CONTACT_PHONE2, true);
        assertEquals(2, ContactsListManager.getInstance().STRANGER_LIST.size());
        ContactsListManager.getInstance().setStrangerList(MOCK_CONTACT_PHONE2, false);
        assertEquals(1, ContactsListManager.getInstance().STRANGER_LIST.size());
        ContactsListManager.getInstance().setStrangerList(MOCK_CONTACT_PHONE, false);
        assertEquals(0, ContactsListManager.getInstance().STRANGER_LIST.size());
    }
    
}
