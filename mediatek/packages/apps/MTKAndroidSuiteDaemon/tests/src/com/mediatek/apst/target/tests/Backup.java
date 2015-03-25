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

import android.test.AndroidTestCase;

import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.util.command.backup.DelAllBookmarkReq;
import com.mediatek.apst.util.command.backup.DelAllBookmarkRsp;
import com.mediatek.apst.util.command.backup.DelAllCalendarReq;
import com.mediatek.apst.util.command.backup.DelAllCalendarRsp;
import com.mediatek.apst.util.command.backup.DelAllContactsReq;
import com.mediatek.apst.util.command.backup.DelAllContactsRsp;
import com.mediatek.apst.util.command.backup.DelAllMsgsForBackupReq;
import com.mediatek.apst.util.command.backup.DelAllMsgsForBackupRsp;
import com.mediatek.apst.util.command.backup.GetAllBookmarkForBackupReq;
import com.mediatek.apst.util.command.backup.GetAllBookmarkForBackupRsp;
import com.mediatek.apst.util.command.backup.GetAllContsDataForBackupReq;
import com.mediatek.apst.util.command.backup.GetAllContsDataForBackupRsp;
import com.mediatek.apst.util.command.backup.GetAllContsForBackupReq;
import com.mediatek.apst.util.command.backup.GetAllContsForBackupRsp;
import com.mediatek.apst.util.command.backup.GetAllGroupsForBackupReq;
import com.mediatek.apst.util.command.backup.GetAllGroupsForBackupRsp;
import com.mediatek.apst.util.command.backup.GetAllSmsForBackupReq;
import com.mediatek.apst.util.command.backup.GetAllSmsForBackupRsp;
import com.mediatek.apst.util.command.backup.GetAppForBackupReq;
import com.mediatek.apst.util.command.backup.GetAppForBackupRsp;
import com.mediatek.apst.util.command.backup.GetAttendeesForBackupReq;
import com.mediatek.apst.util.command.backup.GetAttendeesForBackupRsp;
import com.mediatek.apst.util.command.backup.GetEventsForBackupReq;
import com.mediatek.apst.util.command.backup.GetEventsForBackupRsp;
import com.mediatek.apst.util.command.backup.GetMmsDataForBackupReq;
import com.mediatek.apst.util.command.backup.GetMmsDataForBackupRsp;
import com.mediatek.apst.util.command.backup.GetPhoneListReq;
import com.mediatek.apst.util.command.backup.GetPhoneListRsp;
import com.mediatek.apst.util.command.backup.GetRemindersForBackupReq;
import com.mediatek.apst.util.command.backup.GetRemindersForBackupRsp;
import com.mediatek.apst.util.command.backup.MediaBackupReq;
import com.mediatek.apst.util.command.backup.MediaBackupRsp;
import com.mediatek.apst.util.command.backup.MediaFileRenameReq;
import com.mediatek.apst.util.command.backup.MediaFileRenameRsp;
import com.mediatek.apst.util.command.backup.MediaGetStorageStateReq;
import com.mediatek.apst.util.command.backup.MediaGetStorageStateRsp;
import com.mediatek.apst.util.command.backup.MediaRestoreOverReq;
import com.mediatek.apst.util.command.backup.MediaRestoreOverRsp;
import com.mediatek.apst.util.command.backup.MediaRestoreReq;
import com.mediatek.apst.util.command.backup.MediaRestoreRsp;
import com.mediatek.apst.util.command.backup.RestoreBookmarkReq;
import com.mediatek.apst.util.command.backup.RestoreBookmarkRsp;
import com.mediatek.apst.util.command.backup.RestoreCalendarReq;
import com.mediatek.apst.util.command.backup.RestoreCalendarRsp;
import com.mediatek.apst.util.command.backup.RestoreContactsReq;
import com.mediatek.apst.util.command.backup.RestoreContactsRsp;
import com.mediatek.apst.util.command.backup.RestoreGroupReq;
import com.mediatek.apst.util.command.backup.RestoreGroupRsp;
import com.mediatek.apst.util.command.backup.RestoreMmsReq;
import com.mediatek.apst.util.command.backup.RestoreMmsRsp;
import com.mediatek.apst.util.command.backup.RestoreSmsReq;
import com.mediatek.apst.util.command.backup.RestoreSmsRsp;
import com.mediatek.apst.util.entity.app.ApplicationInfo;
import com.mediatek.apst.util.entity.bookmark.BookmarkData;
import com.mediatek.apst.util.entity.bookmark.BookmarkFolder;
import com.mediatek.apst.util.entity.calendar.CalendarEvent;
import com.mediatek.apst.util.entity.contacts.Group;
import com.mediatek.apst.util.entity.contacts.RawContact;
import com.mediatek.apst.util.entity.message.Sms;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;

public class Backup extends AndroidTestCase {
    DelAllContactsRsp mDelAllContactsRsp;
    DelAllMsgsForBackupRsp mDelAllMsgsForBackupRsp;
    GetAllBookmarkForBackupRsp mGetAllBookmarkForBackupRsp;
    GetAllContsDataForBackupReq mGetAllContsDataForBackupReq;
    GetAllContsDataForBackupRsp mGetAllContsDataForBackupRsp;
    GetAppForBackupReq mGetAppForBackupReq;
    GetAppForBackupRsp mGetAppForBackupRsp;
    MediaBackupReq mMediaBackupReq;
    MediaBackupRsp mMediaBackupRsp;
    MediaFileRenameReq mMediaFileRenameReq;
    RestoreContactsRsp mRestoreContactsRsp;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mDelAllContactsRsp = new DelAllContactsRsp(0);
        mDelAllMsgsForBackupRsp = new DelAllMsgsForBackupRsp(2);
        mGetAllBookmarkForBackupRsp = new GetAllBookmarkForBackupRsp(4);
        mGetAllContsDataForBackupReq = new GetAllContsDataForBackupReq();
        mGetAllContsDataForBackupRsp = new GetAllContsDataForBackupRsp(3);
        mGetAppForBackupReq = new GetAppForBackupReq();
        mGetAppForBackupRsp = new GetAppForBackupRsp(42);
        mMediaBackupReq = new MediaBackupReq();
        mMediaBackupRsp = new MediaBackupRsp(0);
        mMediaFileRenameReq = new MediaFileRenameReq();
        mRestoreContactsRsp = new RestoreContactsRsp(0);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01_constructor() {
        assertNotNull(new DelAllBookmarkReq());
        assertNotNull(new DelAllBookmarkRsp(1));
        assertNotNull(new DelAllCalendarReq());
        assertNotNull(new DelAllCalendarRsp(0));
        assertNotNull(new DelAllContactsReq());
        assertNotNull(mDelAllContactsRsp);
        assertNotNull(new DelAllMsgsForBackupReq());
        assertNotNull(new GetAllBookmarkForBackupReq());
        assertNotNull(mDelAllMsgsForBackupRsp);
        assertNotNull(mGetAllContsDataForBackupReq);
        assertNotNull(mGetAllContsDataForBackupRsp);
        assertNotNull(mGetAllBookmarkForBackupRsp);
        assertNotNull(new GetAllContsForBackupReq());
        assertNotNull(new GetAllContsForBackupRsp(0));
        assertNotNull(new GetAllGroupsForBackupReq());
        assertNotNull(new GetAllGroupsForBackupRsp(3));
        assertNotNull(new GetAllSmsForBackupReq());
        assertNotNull(new GetAllSmsForBackupRsp(2));
        assertNotNull(mGetAppForBackupReq);
        assertNotNull(new GetAttendeesForBackupReq());
        assertNotNull(new GetAttendeesForBackupRsp(2));
        assertNotNull(new GetEventsForBackupReq());
        assertNotNull(new GetEventsForBackupRsp(2));
        assertNotNull(new GetMmsDataForBackupReq());
        assertNotNull(new GetMmsDataForBackupRsp(3));
        assertNotNull(new GetPhoneListReq());
        assertNotNull(new GetPhoneListRsp(0));
        assertNotNull(new GetRemindersForBackupReq());
        assertNotNull(new GetRemindersForBackupRsp(3));
        assertNotNull(new MediaGetStorageStateReq());
        assertNotNull(new MediaRestoreOverRsp(0));
        assertNotNull(new RestoreBookmarkRsp(0));
        assertNotNull(new RestoreContactsReq());
    }

    public void test02_getDeleteCount() {
        mDelAllContactsRsp.setDeleteCount(28);
        assertEquals(28, mDelAllContactsRsp.getDeleteCount());
    }

    public void test03_getDeletedCount() {
        mDelAllMsgsForBackupRsp.setDeletedCount(22);
        assertEquals(22, mDelAllMsgsForBackupRsp.getDeletedCount());
    }

    public void test04_getmBookmarkFolderList() {
        ArrayList<BookmarkFolder> mBookmarkFolderList = new ArrayList<BookmarkFolder>();
        mGetAllBookmarkForBackupRsp.setmBookmarkFolderList(mBookmarkFolderList);
        assertSame(mBookmarkFolderList, mGetAllBookmarkForBackupRsp
                .getmBookmarkFolderList());
    }

    public void test05_getmBookmarkDataList() {
        ArrayList<BookmarkData> mBookmarkDataList = new ArrayList<BookmarkData>();
        mGetAllBookmarkForBackupRsp.setmBookmarkDataList(mBookmarkDataList);
        assertSame(mBookmarkDataList, mGetAllBookmarkForBackupRsp
                .getmBookmarkDataList());
    }

    public void test06_getRequestingDataTypes() {
        mGetAllContsDataForBackupReq.setRequestingAllTypes();
        assertEquals(null, mGetAllContsDataForBackupReq
                .getRequestingDataTypes());
    }

    public void test07_appendRequestingDataType() {
        GetAllContsDataForBackupReq req = mGetAllContsDataForBackupReq
                .appendRequestingDataType(5);
        assertNotNull(req);
    }

    public void test08_getAll() {
        mGetAllContsDataForBackupRsp.setRaw(null);
        mGetAllContsDataForBackupRsp.getAll(Config.VERSION_CODE);

    }

    public void test09_getDestIconWidth() {
        mGetAppForBackupReq.setDestIconWidth(200);
        assertEquals(200, mGetAppForBackupReq.getDestIconWidth());
    }

    public void test10_getDestIconHeight() {
        mGetAppForBackupReq.setDestIconHeight(100);
        assertEquals(100, mGetAppForBackupReq.getDestIconHeight());
    }

    public void test11_builder() {
        assertNotNull(GetAppForBackupRsp.builder());
    }

    public void test12_builder2() {
        assertNotNull(GetAppForBackupRsp.builder(100));
    }

    public void test13_getContentType() {
        mMediaBackupReq.setContentType(MediaBackupReq.FEATURE_APPLICATION);
        assertEquals(MediaBackupReq.FEATURE_APPLICATION, mMediaBackupReq
                .getContentType());
    }

    public void test14_getBackupPaths() {
        ArrayList<String> backupPaths = new ArrayList<String>();
        mMediaBackupReq.setBackupPaths(backupPaths);
        assertSame(backupPaths, mMediaBackupReq.getBackupPaths());
    }

    public void test15_getFileSize() {
        mMediaBackupReq.setFileSize(3434);
        assertEquals(3434, mMediaBackupReq.getFileSize());
    }

    public void test16_getOldPaths() {
        String[] oldPaths = { "oldPath1", "oldPath2", "oldPath3" };
        mMediaBackupRsp.setOldPaths(oldPaths);
        assertSame(oldPaths, mMediaBackupRsp.getOldPaths());
    }

    public void test17_getNewPaths() {
        String[] newPaths = { "newPath1", "newPath2", "newPath3" };
        mMediaBackupRsp.setNewPaths(newPaths);
        assertSame(newPaths, mMediaBackupRsp.getNewPaths());
    }

    public void test18_getDirs() {
        ArrayList<File> dirs = new ArrayList<File>();
        mMediaBackupRsp.setDirs(dirs);
        assertSame(dirs, mMediaBackupRsp.getDirs());
    }

    public void test19_getOldPaths() {
        String[] oldPaths = { "oldPath1", "oldPath2", "oldPath3" };
        mMediaFileRenameReq.setOldPaths(oldPaths);
        assertSame(oldPaths, mMediaFileRenameReq.getOldPaths());
    }

    public void test20_getNewPaths() {
        String[] newPaths = { "newPath1", "newPath2", "newPath3" };
        mMediaFileRenameReq.setNewPaths(newPaths);
        assertSame(newPaths, mMediaFileRenameReq.getNewPaths());
    }

    public void test21_getResults() {
        boolean[] results = { true, false };
        MediaFileRenameRsp mMediaFileRenameRsp = new MediaFileRenameRsp(0);
        mMediaFileRenameRsp.setResults(results);
        assertSame(results, mMediaFileRenameRsp.getResults());
    }

    public void test22_getStorageState() {
        boolean[] storageState = { true, true, false };
        MediaGetStorageStateRsp mediaGetStorageStateRsp = new MediaGetStorageStateRsp(
                3);
        mediaGetStorageStateRsp.setStorageState(storageState);
        assertSame(storageState, mediaGetStorageStateRsp.getStorageState());
    }

    public void test23_getRestoreFilePath() {
        String restoreFilePath = "data/data/";
        MediaRestoreOverReq mediaRestoreOverReq = new MediaRestoreOverReq();
        mediaRestoreOverReq.setRestoreFilePath(restoreFilePath);
        assertEquals(restoreFilePath, mediaRestoreOverReq.getRestoreFilePath());
    }

    public void test24_getContentType() {
        MediaRestoreReq mediaRestoreReq = new MediaRestoreReq();
        mediaRestoreReq.setContentType(MediaRestoreReq.FEATURE_APPLICATION);
        assertEquals(MediaRestoreReq.FEATURE_APPLICATION, mediaRestoreReq
                .getContentType());
    }

    public void test25_getRestorePath() {
        String restoreFilePath = "data/data/";
        MediaRestoreReq mediaRestoreReq = new MediaRestoreReq();
        mediaRestoreReq.setRestorePath(restoreFilePath);
        assertEquals(restoreFilePath, mediaRestoreReq.getRestorePath());
    }

    public void test26_getFileSize() {
        MediaRestoreReq mediaRestoreReq = new MediaRestoreReq();
        mediaRestoreReq.setFileSize(100);
        assertEquals(100, mediaRestoreReq.getFileSize());
    }

    public void test27_getResults() {
        boolean[] results = { false, true, true };
        MediaRestoreRsp mediaRestoreRsp = new MediaRestoreRsp(0);
        mediaRestoreRsp.setResults(results);
        assertSame(results, mediaRestoreRsp.getResults());
    }

    public void test28_getSDState() {
        MediaRestoreRsp mediaRestoreRsp = new MediaRestoreRsp(0);
        mediaRestoreRsp.setSDState(0);
        assertEquals(0, mediaRestoreRsp.getSDState());
    }

    public void test29_getmBookmarkDataList() {
        ArrayList<BookmarkData> dataList = new ArrayList<BookmarkData>();
        RestoreBookmarkReq restoreBookmarkReq = new RestoreBookmarkReq();
        restoreBookmarkReq.setmBookmarkDataList(dataList);
        assertSame(dataList, restoreBookmarkReq.getmBookmarkDataList());
    }

    public void test30_getmBookmarkFolderList() {
        ArrayList<BookmarkFolder> folderList = new ArrayList<BookmarkFolder>();
        RestoreBookmarkReq restoreBookmarkReq = new RestoreBookmarkReq();
        restoreBookmarkReq.setmBookmarkFolderList(folderList);
        assertSame(folderList, restoreBookmarkReq.getmBookmarkFolderList());
    }

    public void test31_getEvent() {
        ArrayList<CalendarEvent> eventList = new ArrayList<CalendarEvent>();
        RestoreCalendarReq restoreCalendarReq = new RestoreCalendarReq();
        restoreCalendarReq.setEvent(eventList);
        assertSame(eventList, restoreCalendarReq.getEvent());
    }

    public void test32_getInsertedCount() {
        RestoreCalendarRsp restoreCalendarRsp = new RestoreCalendarRsp(0);
        restoreCalendarRsp.setInsertedCount(200);
        assertEquals(200, restoreCalendarRsp.getInsertedCount());
    }

    public void test33_getPhase() {
        mRestoreContactsRsp.setPhase(RestoreContactsRsp.PHASE_CONTACT_DATA);
        assertEquals(RestoreContactsRsp.PHASE_CONTACT_DATA, mRestoreContactsRsp
                .getPhase());
    }

    public void test34_getGroupList() {
        ArrayList<Group> list = new ArrayList<Group>();
        RestoreGroupReq restoreGroupReq = new RestoreGroupReq();
        restoreGroupReq.setGroupList(list);
        assertSame(list, restoreGroupReq.getGroupList());
    }

    public void test35_getCount() {
        RestoreGroupRsp restoreGroupRsp = new RestoreGroupRsp(0);
        restoreGroupRsp.setCount(30);
        assertEquals(30, restoreGroupRsp.getCount());
    }

    public void test36_isIsLastImport() {
        RestoreMmsReq restoreMmsReq = new RestoreMmsReq();
        restoreMmsReq.setIsLastImport(true);
        assertEquals(true, restoreMmsReq.isIsLastImport());
    }

    public void test37_getInsertedIds() {
        long[] insertedIds = { 23, 35, 55, 43 };
        RestoreMmsRsp restoreMmsRsp = new RestoreMmsRsp(0);
        restoreMmsRsp.setInsertedIds(insertedIds);
        assertTrue(Arrays.equals(insertedIds, restoreMmsRsp.getInsertedIds()));
    }

    public void test38_getThreadIds() {
        long[] threadIds = { 22, 55, 7, 40 };
        RestoreMmsRsp restoreMmsRsp = new RestoreMmsRsp(0);
        restoreMmsRsp.setThreadIds(threadIds);
        assertTrue(Arrays.equals(threadIds, restoreMmsRsp.getThreadIds()));
    }

    public void test39_builder() {
        assertNotNull(RestoreSmsReq.builder());
        assertNotNull(RestoreSmsReq.builder(100));
    }

    public void test40_getInsertedIds() {
        long[] insertedIds = { 1, 3, 5, 6, 9 };
        RestoreSmsRsp restoreSmsRsp = new RestoreSmsRsp(0);
        restoreSmsRsp.setInsertedIds(insertedIds);
        assertTrue(Arrays.equals(insertedIds, restoreSmsRsp.getInsertedIds()));
    }

    public void test41_getThreadIds() {
        long[] threadIds = { 12, 3, 23, 88 };
        RestoreSmsRsp restoreSmsRsp = new RestoreSmsRsp(0);
        restoreSmsRsp.setThreadIds(threadIds);
        assertTrue(Arrays.equals(threadIds, restoreSmsRsp.getThreadIds()));
    }

    public void test42_getAll() {
        GetAllContsForBackupRsp contsRsp = new GetAllContsForBackupRsp(10);
        contsRsp.setRaw(null);
        assertNotNull(contsRsp.getAll(Config.VERSION_CODE));
    }

    public void test43_getAll() {
        GetAllGroupsForBackupRsp groupBackupRsp = new GetAllGroupsForBackupRsp(
                11);
        groupBackupRsp.setRaw(null);
        assertNotNull(groupBackupRsp.getAll(Config.VERSION_CODE));
    }

    public void test44_getAll() {
        GetAllSmsForBackupRsp smsBackupRsp = new GetAllSmsForBackupRsp(8);
        smsBackupRsp.setRaw(null);
        assertNotNull(smsBackupRsp.getAll(Config.VERSION_CODE));
    }

    /**
     * class:GetAppForBackupRsp.
     */
    public void test45_getResults() {
        mGetAppForBackupRsp.setRaw(null);
        assertNotNull(mGetAppForBackupRsp.getResults());
    }

    /**
     * test GetAppForBackupRsp's inner class: Build.
     */
    public void test46_appendAppInfo() {
        GetAppForBackupRsp.Builder builder = GetAppForBackupRsp.builder();
        ApplicationInfo appInfo = new ApplicationInfo();
        assertNotNull(builder.appendAppInfo(appInfo));
    }

    public void test47_build() {
        GetAppForBackupRsp.Builder builder = GetAppForBackupRsp.builder();
        assertNotNull(builder.build());
    }

    public void test48_getAll() {
        GetAttendeesForBackupRsp attendeeBackupRsp = new GetAttendeesForBackupRsp(
                10);
        attendeeBackupRsp.setRaw(null);
        assertNotNull(attendeeBackupRsp.getAll(Config.VERSION_CODE));
    }

    public void test49_getAll() {
        GetEventsForBackupRsp eventBackup = new GetEventsForBackupRsp(2);
        eventBackup.setRaw(null);
        assertNotNull(eventBackup.getAll(Config.VERSION_CODE));
    }

    /**
     * GetMmsDataForBackupRsp.
     */
    public void test50_getMmsDataBuffer() {
        GetMmsDataForBackupRsp mmsDataRsp = new GetMmsDataForBackupRsp(33);
        mmsDataRsp.setRaw(null);
        assertNotNull(mmsDataRsp.getMmsDataBuffer());
    }

    public void test60_getAll() {
        GetRemindersForBackupRsp reminderBackupRsp = new GetRemindersForBackupRsp(
                8);
        reminderBackupRsp.setRaw(null);
        reminderBackupRsp.getAll(Config.VERSION_CODE);
    }

    /**
     * RestoreContactsReq.
     */
    public void test61_builder() {
        assertNotNull(RestoreContactsReq.builder());
    }

    /**
     * RestoreContactsReq.
     */
    public void test62_builder() {
        assertNotNull(RestoreContactsReq.builder(200));
    }

    /**
     * inner class of RestoreContactsReq.
     */
    public void test63_appendContact() {
        RestoreContactsReq.Builder builder = RestoreContactsReq.builder();
        RawContact contact = new RawContact();
        assertNotNull(builder.appendContact(contact, Config.VERSION_CODE));
    }

    /**
     * inner class of RestoreContactsReq.
     */
    public void test64_build() {
        RestoreContactsReq.Builder builder = RestoreContactsReq.builder(500);
        assertNotNull(builder.build());
    }

    /**
     * inner class of the RestoreSmsRsp.
     */
    // public void test65_appendSms() {
    // Sms sms = new Sms();
    // sms.setBody("message body");
    // sms.setBox(0);
    // sms.setDate(2012);
    // sms.setDate_sent(2012);
    // sms.setId(11);
    // sms.setLocked(false);
    // sms.setRead(false);
    // sms.setServiceCenter("serviceCenter");
    // RestoreSmsReq.Builder builder = RestoreSmsReq.builder(200);
    // assertNotNull(builder.appendSms(sms, Config.VERSION_CODE));
    // }

    /**
     * inner class of the RestoreSmsRsp.
     */
    public void test66_build() {
        RestoreSmsReq.Builder builder = RestoreSmsReq.builder(200);
        assertNotNull(builder.build());
    }
}
