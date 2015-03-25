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
package com.mediatek.rcse.test.model;

import android.os.Parcel;
import android.test.InstrumentationTestCase;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.test.Utils;

/**
 * This class is used to test class ParticipantInfo
 */
public class ParticipantInfoTest extends InstrumentationTestCase {
    private static final String TAG = "ParticipantInfoTest";
    private ParticipantInfo mParticipantInfo = null;
    private static final String CONTACT = "+34200000252";
    private static final String DISPLAY_NAME = "test";
    private static final String STATE = "test";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Participant participant = new Participant(CONTACT, DISPLAY_NAME);
        mParticipantInfo = new ParticipantInfo(participant, STATE);
       
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.v(TAG, "tearDown() entry");
        super.tearDown();
        Utils.clearAllStatus();
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
        Logger.v(TAG, "tearDown() exit");
    }

    /**
     * Test describeContents
     */
    public void testCase01_describeContents() {
        Logger.d(TAG, "testCase01_describeContents");
        assertEquals(0, mParticipantInfo.describeContents());
    }
    
    public void testCase02_writeToParcel(){
        Logger.d(TAG, "testCase02_writeToParcel");
        Parcel dest = Parcel.obtain();
        mParticipantInfo.writeToParcel(dest, 0);
    }
    
    public void testCase03_createFromParcel() {
        Logger.d(TAG, "testCase03_createFromParcel");
        Parcel source = Parcel.obtain();
        source.writeString(CONTACT);
        source.writeString(DISPLAY_NAME);
        source.writeString(STATE);
        ParticipantInfo  participantInfo = mParticipantInfo.CREATOR.createFromParcel(source);
    }

    public void testCase04_newArray() {
        Logger.d(TAG, "testCase04_newArray");
        ParticipantInfo [] participantInfos = mParticipantInfo.CREATOR.newArray(5);
        assertEquals(5, participantInfos.length);
    }

    public void testCase05_getContact() {
        Logger.d(TAG, "testCase05_getContact");
        assertEquals(CONTACT, mParticipantInfo.getContact());
        ParticipantInfo participantInfo = new ParticipantInfo(null, STATE);
        assertNull(participantInfo.getContact());
    }
}