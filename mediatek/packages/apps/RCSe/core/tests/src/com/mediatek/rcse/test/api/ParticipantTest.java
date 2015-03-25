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

package com.mediatek.rcse.test.api;

import android.content.ServiceConnection;
import android.os.RemoteException;
import android.test.AndroidTestCase;

import com.mediatek.rcse.api.FlightModeApi;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.service.IFlightMode;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.LauncherUtils;

import java.lang.reflect.Field;

/**
 * Test case for Participant
 *
 */
public class ParticipantTest extends AndroidTestCase {
    private static final String TAG = "Participant";
    private static final String CONTACT = "Test contact";
    private static final String DISPLAY_NAME = "Test display name";
    private static final int ARRAY_SIZE = 10;
    private static final int DEFAULT_HASH_CODE = 0;
    private static final int DESCRIBE_CONTENTS = 0;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Participant participant = new Participant(CONTACT, DISPLAY_NAME);
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.d(TAG, "tearDown()");
        super.tearDown();
        Utils.clearAllStatus();
    }

    /**
     * Test equals()
     */
    public void testCase01_equals() {
        Logger.d(TAG, "testCase01_equals() entry");
        Participant participant = new Participant(CONTACT, DISPLAY_NAME);
        String invalidParticipant = null;
        assertFalse(participant.equals(invalidParticipant));
        
        participant = new Participant(null, DISPLAY_NAME);
        Participant testParticipant = new Participant(null, DISPLAY_NAME);
        assertTrue(participant.equals(testParticipant));
        testParticipant = new Participant(CONTACT, DISPLAY_NAME);
        assertFalse(participant.equals(testParticipant));
        
        participant = new Participant(CONTACT, DISPLAY_NAME);
        testParticipant = new Participant(null, DISPLAY_NAME);
        assertFalse(participant.equals(testParticipant));
        testParticipant = new Participant(CONTACT, DISPLAY_NAME);
        assertTrue(participant.equals(testParticipant));
        Logger.d(TAG, "testCase01_equals() exit");
    }
    
    /**
     * Test newArray()
     */
    public void testCase02_newArray() {
        Logger.d(TAG, "testCase02_newArray() entry");
        Participant participant = new Participant(CONTACT, DISPLAY_NAME);
        Participant[] list = participant.CREATOR.newArray(ARRAY_SIZE);
        assertNotNull(list);
        assertEquals(ARRAY_SIZE, list.length);
        Logger.d(TAG, "testCase02_newArray() exit");
    }
    
    /**
     * Test hashCode()
     */
    public void testCase03_hashCode() {
        Logger.d(TAG, "testCase03_hashCode() entry");
        Participant participant = new Participant(null, DISPLAY_NAME);
        assertEquals(DEFAULT_HASH_CODE, participant.hashCode());
        participant = new Participant(CONTACT, DISPLAY_NAME);
        assertTrue(participant.hashCode() > DEFAULT_HASH_CODE);
        Logger.d(TAG, "testCase03_hashCode() exit");
    }
    
    /**
     * Test describeContents()
     */
    public void testCase04_describeContents() {
        Logger.d(TAG, "testCase04_describeContents() entry");
        Participant participant = new Participant(CONTACT, DISPLAY_NAME);
        assertEquals(DESCRIBE_CONTENTS, participant.describeContents());
        Logger.d(TAG, "testCase04_describeContents() exit");
    }
}
