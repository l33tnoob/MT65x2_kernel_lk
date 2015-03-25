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

import junit.framework.TestSuite;

public class ApstFunctionalTestRunner extends JUnitInstrumentationTestRunner {
    @Override
    public TestSuite getAllTests() {
        TestSuite tests = new TestSuite();
        tests.addTestSuite(ApplicationProxyTest.class);
        tests.addTestSuite(BookmarkProxyTest.class);
        tests.addTestSuite(ProxyManagerTest.class);
        tests.addTestSuite(CalendarProxyTest.class);
        tests.addTestSuite(TargetAddressTest.class);
        tests.addTestSuite(ContentProviderOperationBatchTest.class);
        tests.addTestSuite(MeasuredContentValuesTest.class);
        tests.addTestSuite(CalendarContentTest.class);
        tests.addTestSuite(ReminderContentTest.class);
        tests.addTestSuite(CalendarEventContentTest.class);
        tests.addTestSuite(AttendeeContentTest.class);
        tests.addTestSuite(MmsContentTest.class);
        tests.addTestSuite(SmsContentTest.class);
        tests.addTestSuite(RawBlockRequestTest.class);
        tests.addTestSuite(RawBlockResponseTest.class);
        tests.addTestSuite(AsyncGetAllAppInfoReqTest.class);
        tests.addTestSuite(CalendarEventSyncFlagTest.class);
        tests.addTestSuite(ContactsSyncFlagTest.class);
        tests.addTestSuite(FastCursorParserTest.class);
        tests.addTestSuite(ContactTest.class);
        tests.addTestSuite(ContactDataTest.class);
        tests.addTestSuite(EmailTest.class);
        tests.addTestSuite(GroupTest.class);
        tests.addTestSuite(GroupMembershipTest.class);
        tests.addTestSuite(ImTest.class);
        tests.addTestSuite(NicknameTest.class);
        tests.addTestSuite(NoteTest.class);
        tests.addTestSuite(OrganizationTest.class);
        tests.addTestSuite(PhoneTest.class);
        tests.addTestSuite(PhotoTest.class);
        tests.addTestSuite(DelAllBookmarkReqTest.class);
        tests.addTestSuite(ApplicationInfoTest.class);
        tests.addTestSuite(Backup.class);
        tests.addTestSuite(RawContactTest.class);
        tests.addTestSuite(ContactsProxyTest2.class);
        tests.addTestSuite(GroupContentTest.class);
        tests.addTestSuite(ContactDataContentTest.class);
        tests.addTestSuite(RawContactsContentTest.class);
        tests.addTestSuite(SimContactsContentTest.class);
        tests.addTestSuite(USIMUtilsTest.class);
        tests.addTestSuite(FastSimContactsCursorParserTest.class);
     // Remove for KK
//        tests.addTestSuite(FastSmsCursorParserTest.class);
//        tests.addTestSuite(FastMmsCursorParserTest.class);
//        tests.addTestSuite(FastMmsBackupCursorParserTest.class);
//        tests.addTestSuite(FastMmsResourceCursorParserTest.class);
        tests.addTestSuite(FastPhoneListCursorParserTest.class);
        tests.addTestSuite(GlobalTest.class);
        tests.addTestSuite(StringUtilsTest.class);
        tests.addTestSuite(SharedPrefsTest.class);
        tests.addTestSuite(ContactsObserverTest.class);
        tests.addTestSuite(CalendarEventObserverTest.class);
        tests.addTestSuite(MessageObserverTest.class);
        tests.addTestSuite(MulMessageObserverTest.class);
     // Remove for KK
//        tests.addTestSuite(MessageProxyTest.class);
        tests.addTestSuite(SystemInfoProxyTest.class);
        tests.addTestSuite(MediaProxyTest.class);
        tests.addTestSuite(FtpServiceTest.class);
        tests.addTestSuite(FtpServerTest.class);
        tests.addTestSuite(ContactsOperationBatchTest.class);
        tests.addTestSuite(DefaultUpdateBatchHelperTest.class); 
        return tests;
    }
}
