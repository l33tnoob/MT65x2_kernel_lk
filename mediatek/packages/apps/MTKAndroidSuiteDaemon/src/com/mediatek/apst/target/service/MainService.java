/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.apst.target.service;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.widget.Toast;

import com.mediatek.apst.target.R;
import com.mediatek.apst.target.data.provider.calendar.CalendarEventContent;
import com.mediatek.apst.target.data.provider.message.MmsContent;
import com.mediatek.apst.target.data.provider.message.SmsContent;
import com.mediatek.apst.target.data.proxy.IRawBlockConsumer;
import com.mediatek.apst.target.data.proxy.ProxyManager;
import com.mediatek.apst.target.data.proxy.app.ApplicationProxy;
import com.mediatek.apst.target.data.proxy.bookmark.BookmarkProxy;
import com.mediatek.apst.target.data.proxy.calendar.CalendarProxy;
import com.mediatek.apst.target.data.proxy.contacts.ContactsProxy;
import com.mediatek.apst.target.data.proxy.media.MediaProxy;
import com.mediatek.apst.target.data.proxy.message.MessageProxy;
import com.mediatek.apst.target.data.proxy.sysinfo.SystemInfoProxy;
import com.mediatek.apst.target.event.Event;
import com.mediatek.apst.target.event.EventDispatcher;
import com.mediatek.apst.target.event.IBatteryListener;
import com.mediatek.apst.target.event.ICalendarEventListener;
import com.mediatek.apst.target.event.IContactsListener;
import com.mediatek.apst.target.event.IMmsListener;
import com.mediatek.apst.target.event.IPackageListener;
import com.mediatek.apst.target.event.ISdStateListener;
import com.mediatek.apst.target.event.ISimStateListener;
import com.mediatek.apst.target.event.ISmsListener;
import com.mediatek.apst.target.event.IBackupAndRestoreListener;
import com.mediatek.apst.target.ftp.FtpService;
import com.mediatek.apst.target.receiver.InternalReceiver;
import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.util.FeatureOptionControl;
import com.mediatek.apst.util.command.BaseCommand;
import com.mediatek.apst.util.command.ICommandBatch;
import com.mediatek.apst.util.command.ResponseCommand;
import com.mediatek.apst.util.command.UnsupportedRequestResponse;
import com.mediatek.apst.util.command.app.AsyncGetAllAppInfoReq;
import com.mediatek.apst.util.command.app.AsyncGetAllAppInfoRsp;
import com.mediatek.apst.util.command.app.NotiPackageInstalledReq;
import com.mediatek.apst.util.command.backup.DelAllBookmarkReq;
import com.mediatek.apst.util.command.backup.DelAllBookmarkRsp;
import com.mediatek.apst.util.command.backup.DelAllCalendarReq;
import com.mediatek.apst.util.command.backup.DelAllCalendarRsp;
import com.mediatek.apst.util.command.backup.DelAllContactsReq;
import com.mediatek.apst.util.command.backup.DelAllContactsRsp;
import com.mediatek.apst.util.command.backup.DelAllMsgsForBackupReq;
import com.mediatek.apst.util.command.backup.DelAllMsgsForBackupRsp;
import com.mediatek.apst.util.command.backup.EndBackupReq;
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
import com.mediatek.apst.util.command.backup.GetEmailForBackupRsp;
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
import com.mediatek.apst.util.command.backup.StartBackupReq;
import com.mediatek.apst.util.command.bookmark.AsyncDeleteBookmarkReq;
import com.mediatek.apst.util.command.bookmark.AsyncDeleteBookmarkRsp;
import com.mediatek.apst.util.command.bookmark.AsyncGetAllBookmarkInfoReq;
import com.mediatek.apst.util.command.bookmark.AsyncGetAllBookmarkInfoRsp;
import com.mediatek.apst.util.command.bookmark.AsyncInsertBookmarkReq;
import com.mediatek.apst.util.command.bookmark.AsyncInsertBookmarkRsp;
import com.mediatek.apst.util.command.calendar.AddEventReq;
import com.mediatek.apst.util.command.calendar.AddEventRsp;
import com.mediatek.apst.util.command.calendar.CalendarBatchReq;
import com.mediatek.apst.util.command.calendar.CalendarBatchRsp;
import com.mediatek.apst.util.command.calendar.DeleteEventReq;
import com.mediatek.apst.util.command.calendar.DeleteEventRsp;
import com.mediatek.apst.util.command.calendar.GetAttendeesReq;
import com.mediatek.apst.util.command.calendar.GetAttendeesRsp;
import com.mediatek.apst.util.command.calendar.GetCalendarsReq;
import com.mediatek.apst.util.command.calendar.GetCalendarsRsp;
import com.mediatek.apst.util.command.calendar.GetEventsReq;
import com.mediatek.apst.util.command.calendar.GetEventsRsp;
import com.mediatek.apst.util.command.calendar.GetRemindersReq;
import com.mediatek.apst.util.command.calendar.GetRemindersRsp;
import com.mediatek.apst.util.command.calendar.NotifyCalendarEventChangeReq;
import com.mediatek.apst.util.command.calendar.UpdateEventReq;
import com.mediatek.apst.util.command.calendar.UpdateEventRsp;
import com.mediatek.apst.util.command.contacts.AddContactDataReq;
import com.mediatek.apst.util.command.contacts.AddContactDataRsp;
import com.mediatek.apst.util.command.contacts.AddContactReq;
import com.mediatek.apst.util.command.contacts.AddContactRsp;
import com.mediatek.apst.util.command.contacts.AddGroupMembershipReq;
import com.mediatek.apst.util.command.contacts.AddGroupMembershipRsp;
import com.mediatek.apst.util.command.contacts.AddGroupReq;
import com.mediatek.apst.util.command.contacts.AddGroupRsp;
import com.mediatek.apst.util.command.contacts.AddSimContactReq;
import com.mediatek.apst.util.command.contacts.AddSimContactRsp;
import com.mediatek.apst.util.command.contacts.AsyncGetAllContactDataReq;
import com.mediatek.apst.util.command.contacts.AsyncGetAllContactDataRsp;
import com.mediatek.apst.util.command.contacts.AsyncGetAllGroupsReq;
import com.mediatek.apst.util.command.contacts.AsyncGetAllGroupsRsp;
import com.mediatek.apst.util.command.contacts.AsyncGetAllRawContactsReq;
import com.mediatek.apst.util.command.contacts.AsyncGetAllRawContactsRsp;
import com.mediatek.apst.util.command.contacts.AsyncGetAllSimContactsReq;
import com.mediatek.apst.util.command.contacts.AsyncGetAllSimContactsRsp;
import com.mediatek.apst.util.command.contacts.ContactsBatchReq;
import com.mediatek.apst.util.command.contacts.ContactsBatchRsp;
import com.mediatek.apst.util.command.contacts.DeleteAllContactsReq;
import com.mediatek.apst.util.command.contacts.DeleteAllContactsRsp;
import com.mediatek.apst.util.command.contacts.DeleteContactDataReq;
import com.mediatek.apst.util.command.contacts.DeleteContactDataRsp;
import com.mediatek.apst.util.command.contacts.DeleteContactReq;
import com.mediatek.apst.util.command.contacts.DeleteContactRsp;
import com.mediatek.apst.util.command.contacts.DeleteGroupReq;
import com.mediatek.apst.util.command.contacts.DeleteGroupRsp;
import com.mediatek.apst.util.command.contacts.DeleteSimContactReq;
import com.mediatek.apst.util.command.contacts.DeleteSimContactRsp;
import com.mediatek.apst.util.command.contacts.GetDetailedContactReq;
import com.mediatek.apst.util.command.contacts.GetDetailedContactRsp;
import com.mediatek.apst.util.command.contacts.ImportDetailedContactsReq;
import com.mediatek.apst.util.command.contacts.ImportDetailedContactsRsp;
import com.mediatek.apst.util.command.contacts.NotifyContactsContentChangeReq;
import com.mediatek.apst.util.command.contacts.NotifyContactsDataClearedReq;
import com.mediatek.apst.util.command.contacts.UpdateContactDataReq;
import com.mediatek.apst.util.command.contacts.UpdateContactDataRsp;
import com.mediatek.apst.util.command.contacts.UpdateDetailedContactReq;
import com.mediatek.apst.util.command.contacts.UpdateDetailedContactRsp;
import com.mediatek.apst.util.command.contacts.UpdateGroupReq;
import com.mediatek.apst.util.command.contacts.UpdateGroupRsp;
import com.mediatek.apst.util.command.contacts.UpdateRawContactReq;
import com.mediatek.apst.util.command.contacts.UpdateRawContactRsp;
import com.mediatek.apst.util.command.contacts.UpdateSimContactReq;
import com.mediatek.apst.util.command.contacts.UpdateSimContactRsp;
import com.mediatek.apst.util.command.media.GetAllMediaFilesReq;
import com.mediatek.apst.util.command.media.GetAllMediaFilesRsp;
import com.mediatek.apst.util.command.media.GetContentDirectoriesReq;
import com.mediatek.apst.util.command.media.GetContentDirectoriesRsp;
import com.mediatek.apst.util.command.media.MediaSyncOverReq;
import com.mediatek.apst.util.command.media.MediaSyncOverRsp;
import com.mediatek.apst.util.command.media.RenameFilesReq;
import com.mediatek.apst.util.command.media.RenameFilesRsp;
import com.mediatek.apst.util.command.message.AsyncGetAllMmsReq;
import com.mediatek.apst.util.command.message.AsyncGetAllMmsRsp;
import com.mediatek.apst.util.command.message.AsyncGetAllSmsReq;
import com.mediatek.apst.util.command.message.AsyncGetAllSmsRsp;
import com.mediatek.apst.util.command.message.AsyncGetPhoneListReq;
import com.mediatek.apst.util.command.message.AsyncGetPhoneListRsp;
import com.mediatek.apst.util.command.message.BeforeImportMmsReq;
import com.mediatek.apst.util.command.message.BeforeImportMmsRsp;
import com.mediatek.apst.util.command.message.ClearMessageBoxReq;
import com.mediatek.apst.util.command.message.ClearMessageBoxRsp;
import com.mediatek.apst.util.command.message.DeleteAllMessagesReq;
import com.mediatek.apst.util.command.message.DeleteAllMessagesRsp;
import com.mediatek.apst.util.command.message.DeleteMessageReq;
import com.mediatek.apst.util.command.message.DeleteMessageRsp;
import com.mediatek.apst.util.command.message.GetMmsDataReq;
import com.mediatek.apst.util.command.message.GetMmsDataRsp;
import com.mediatek.apst.util.command.message.GetMmsResourceReq;
import com.mediatek.apst.util.command.message.GetMmsResourceRsp;
import com.mediatek.apst.util.command.message.ImportMmsReq;
import com.mediatek.apst.util.command.message.ImportMmsRsp;
import com.mediatek.apst.util.command.message.ImportSmsReq;
import com.mediatek.apst.util.command.message.ImportSmsRsp;
import com.mediatek.apst.util.command.message.LockMessageReq;
import com.mediatek.apst.util.command.message.LockMessageRsp;
import com.mediatek.apst.util.command.message.MarkMessageAsReadReq;
import com.mediatek.apst.util.command.message.MarkMessageAsReadRsp;
import com.mediatek.apst.util.command.message.MessageBatchReq;
import com.mediatek.apst.util.command.message.MessageBatchRsp;
import com.mediatek.apst.util.command.message.MoveMessageToBoxReq;
import com.mediatek.apst.util.command.message.MoveMessageToBoxRsp;
import com.mediatek.apst.util.command.message.NotifyMessageDataClearedReq;
import com.mediatek.apst.util.command.message.NotifyMessageSentReq;
import com.mediatek.apst.util.command.message.NotifyNewMessageReq;
import com.mediatek.apst.util.command.message.ResendSmsReq;
import com.mediatek.apst.util.command.message.ResendSmsRsp;
import com.mediatek.apst.util.command.message.SaveSmsDraftReq;
import com.mediatek.apst.util.command.message.SaveSmsDraftRsp;
import com.mediatek.apst.util.command.message.SendSmsReq;
import com.mediatek.apst.util.command.message.SendSmsRsp;
import com.mediatek.apst.util.command.sync.CalendarFastSyncAddEventsReq;
import com.mediatek.apst.util.command.sync.CalendarFastSyncAddEventsRsp;
import com.mediatek.apst.util.command.sync.CalendarFastSyncDeleteEventsReq;
import com.mediatek.apst.util.command.sync.CalendarFastSyncDeleteEventsRsp;
import com.mediatek.apst.util.command.sync.CalendarFastSyncGetAttendeesReq;
import com.mediatek.apst.util.command.sync.CalendarFastSyncGetAttendeesRsp;
import com.mediatek.apst.util.command.sync.CalendarFastSyncGetEventsReq;
import com.mediatek.apst.util.command.sync.CalendarFastSyncGetEventsRsp;
import com.mediatek.apst.util.command.sync.CalendarFastSyncGetRemindersReq;
import com.mediatek.apst.util.command.sync.CalendarFastSyncGetRemindersRsp;
import com.mediatek.apst.util.command.sync.CalendarFastSyncInitReq;
import com.mediatek.apst.util.command.sync.CalendarFastSyncInitRsp;
import com.mediatek.apst.util.command.sync.CalendarFastSyncUpdateEventsReq;
import com.mediatek.apst.util.command.sync.CalendarFastSyncUpdateEventsRsp;
import com.mediatek.apst.util.command.sync.CalendarSlowSyncAddEventsReq;
import com.mediatek.apst.util.command.sync.CalendarSlowSyncAddEventsRsp;
import com.mediatek.apst.util.command.sync.CalendarSlowSyncGetAllAttendeesReq;
import com.mediatek.apst.util.command.sync.CalendarSlowSyncGetAllAttendeesRsp;
import com.mediatek.apst.util.command.sync.CalendarSlowSyncGetAllEventsReq;
import com.mediatek.apst.util.command.sync.CalendarSlowSyncGetAllEventsRsp;
import com.mediatek.apst.util.command.sync.CalendarSlowSyncGetAllRemindersReq;
import com.mediatek.apst.util.command.sync.CalendarSlowSyncGetAllRemindersRsp;
import com.mediatek.apst.util.command.sync.CalendarSlowSyncInitReq;
import com.mediatek.apst.util.command.sync.CalendarSlowSyncInitRsp;
import com.mediatek.apst.util.command.sync.CalendarSyncOverReq;
import com.mediatek.apst.util.command.sync.CalendarSyncOverRsp;
import com.mediatek.apst.util.command.sync.CalendarSyncStartReq;
import com.mediatek.apst.util.command.sync.CalendarSyncStartRsp;
import com.mediatek.apst.util.command.sync.ContactsFastSyncAddDetailedContactsReq;
import com.mediatek.apst.util.command.sync.ContactsFastSyncAddDetailedContactsRsp;
import com.mediatek.apst.util.command.sync.ContactsFastSyncDeleteContactsReq;
import com.mediatek.apst.util.command.sync.ContactsFastSyncDeleteContactsRsp;
import com.mediatek.apst.util.command.sync.ContactsFastSyncGetContactDataReq;
import com.mediatek.apst.util.command.sync.ContactsFastSyncGetContactDataRsp;
import com.mediatek.apst.util.command.sync.ContactsFastSyncGetRawContactsReq;
import com.mediatek.apst.util.command.sync.ContactsFastSyncGetRawContactsRsp;
import com.mediatek.apst.util.command.sync.ContactsFastSyncInitReq;
import com.mediatek.apst.util.command.sync.ContactsFastSyncInitRsp;
import com.mediatek.apst.util.command.sync.ContactsFastSyncUpdateDetailedContactsReq;
import com.mediatek.apst.util.command.sync.ContactsFastSyncUpdateDetailedContactsRsp;
import com.mediatek.apst.util.command.sync.ContactsSlowSyncAddDetailedContactsReq;
import com.mediatek.apst.util.command.sync.ContactsSlowSyncAddDetailedContactsRsp;
import com.mediatek.apst.util.command.sync.ContactsSlowSyncGetAllContactDataReq;
import com.mediatek.apst.util.command.sync.ContactsSlowSyncGetAllContactDataRsp;
import com.mediatek.apst.util.command.sync.ContactsSlowSyncGetAllRawContactsReq;
import com.mediatek.apst.util.command.sync.ContactsSlowSyncGetAllRawContactsRsp;
import com.mediatek.apst.util.command.sync.ContactsSlowSyncInitReq;
import com.mediatek.apst.util.command.sync.ContactsSlowSyncInitRsp;
import com.mediatek.apst.util.command.sync.ContactsSyncOverReq;
import com.mediatek.apst.util.command.sync.ContactsSyncOverRsp;
import com.mediatek.apst.util.command.sync.ContactsSyncStartReq;
import com.mediatek.apst.util.command.sync.ContactsSyncStartRsp;
import com.mediatek.apst.util.command.sysinfo.GetStorageReq;
import com.mediatek.apst.util.command.sysinfo.GetStorageRsp;
import com.mediatek.apst.util.command.sysinfo.GetSysInfoReq;
import com.mediatek.apst.util.command.sysinfo.GetSysInfoRsp;
import com.mediatek.apst.util.command.sysinfo.GreetingReq;
import com.mediatek.apst.util.command.sysinfo.GreetingRsp;
import com.mediatek.apst.util.command.sysinfo.NotifyBatteryReq;
import com.mediatek.apst.util.command.sysinfo.NotifySDStateReq;
import com.mediatek.apst.util.command.sysinfo.NotifySimStateReq;
import com.mediatek.apst.util.command.sysinfo.SimDetailInfo;
import com.mediatek.apst.util.command.sysinfo.SysInfoBatchReq;
import com.mediatek.apst.util.command.sysinfo.SysInfoBatchRsp;
import com.mediatek.apst.util.communication.comm.CommFactory;
import com.mediatek.apst.util.communication.common.CommHandler;
import com.mediatek.apst.util.communication.common.Dispatcher;
import com.mediatek.apst.util.communication.common.ICallback;
import com.mediatek.apst.util.communication.common.TransportEntity;
import com.mediatek.apst.util.communication.connManager.ConnManageEntity;
import com.mediatek.apst.util.entity.DatabaseRecordEntity;
import com.mediatek.apst.util.entity.app.ApplicationInfo;
import com.mediatek.apst.util.entity.bookmark.BookmarkData;
import com.mediatek.apst.util.entity.bookmark.BookmarkFolder;
import com.mediatek.apst.util.entity.calendar.CalendarEvent;
import com.mediatek.apst.util.entity.contacts.BaseContact;
import com.mediatek.apst.util.entity.contacts.ContactData;
import com.mediatek.apst.util.entity.contacts.Group;
import com.mediatek.apst.util.entity.contacts.RawContact;
import com.mediatek.apst.util.entity.media.MediaInfo;
import com.mediatek.apst.util.entity.message.Mms;
import com.mediatek.apst.util.entity.message.Sms;

import java.io.IOException;
import java.util.ArrayList;
import java.util.LinkedList;

/**
 * Class Name: MainService
 * <p>
 * Package: com.mediatek.apst.target.service
 * <p>
 * Created on: 2010-6-30
 * <p>
 * <p>
 * Description:
 * <p>
 * Main service of Android PC Sync Tool target daemon. Stays alive and handles
 * connection and communication with PC.
 * <p>
 * 
 * @author mtk80734 Siyang.Miao
 * @version V1.0
 */
public class MainService extends Service implements IBatteryListener,
        IPackageListener, ISdStateListener, ISimStateListener, ISmsListener,
        IContactsListener, ICalendarEventListener, IMmsListener, IBackupAndRestoreListener {
    // ==============================================================
    // Constants
    // ==============================================================
    private static final int MSG_CONNECTED = 1;
    private static final int MSG_CHECK_TIMEOUT = 2;
    private static final int MSG_SAFE_STOP = 3;
    private static final int MSG_FORCE_STOP = 4;

    private static final int RSP_ST_SEND_SINGLE = 1;
    private static final int RSP_ST_APPEND_BATCH = 2;
    private static final int RSP_ST_SEND_BATCH = 3;

    // ==============================================================
    // Fields
    // ==============================================================
    private static Boolean sHasInstance = false;
    private boolean mHasNotifiedStart = false;
    private boolean mConnected = false;
    private int mResponseState = RSP_ST_SEND_SINGLE;
    private boolean mPrepareToStop = false;

    private NotificationManager mNotiMgr;

    private InternalReceiver mBR;

    private CommHandler mComm;

    private Dispatcher mDispatcher;

    private ICallback mMainCallback;

    private ICommandBatch mCommandBatch;

    private ContactsObserver mContactsOb;
    private MessageObserver mMessageOb;
    private MulMessageObserver mMulMessageOb;
    private CalendarEventObserver mCalendarEventOb;

    private SystemInfoProxy mSysInfoProxy;
    private ContactsProxy mContactsProxy;
    private MessageProxy mMessageProxy;
    private CalendarProxy mCalendarProxy;
    private ApplicationProxy mApplicationProxy;
    private MediaProxy mMediaProxy;
    private BookmarkProxy mBookmarkProxy;

    private MainHandler mMsgHandler;

    // Threads -----------------------------------------------------------------
    // Thread for creating socket connection with PC side
    private Connector mConnector = new Connector();
    // Command handling thread
    private CommandHandler mCmdHandler = new CommandHandler(500); // add size to
    // 500
    // Command sending thread
    private CommandSender mCmdSender = new CommandSender(500);
    // Incoming SMS finding thread
    private IncomingSmsFinder mIncomingSmsFinder = new IncomingSmsFinder();
    // SMS sending thread
    private SmsSender mSmsSender = SmsSender.getInstance();

    // ==============================================================
    // Constructors
    // ==============================================================

    // ==============================================================
    // Getters
    // ==============================================================
    private boolean isConnected() {
        return mConnected;
    }

    // ==============================================================
    // Setters
    // ==============================================================
    private void setConnected(boolean connected) {
        this.mConnected = connected;
    }

    // ==============================================================
    // Methods
    // ==============================================================
    // @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    // @Override
    public void onCreate() {
        Global.sContext = this;
        mBR = new InternalReceiver(this);
        mNotiMgr = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        // Data operation proxy
        mSysInfoProxy = ProxyManager.getSystemInfoProxy(this);
        mContactsProxy = ProxyManager.getContactsProxy(this);
        mContactsProxy.registerSelfChangeObserver(mContactsOb);
        mMessageProxy = ProxyManager.getMessageProxy(this);
        mMessageProxy.registerSelfChangeObserver(mMessageOb);
        mMessageProxy.registerSelfChangeObserver(mMulMessageOb);
        mCalendarProxy = ProxyManager.getCalendarProxy(this);
        // CalendarEvent content observer
        mCalendarEventOb = new CalendarEventObserver(mMsgHandler,
                mCalendarProxy);
        mCalendarProxy.registerSelfChangeObserver(mCalendarEventOb);
        mApplicationProxy = ProxyManager.getApplicationProxy(this);
        mMediaProxy = ProxyManager.getMediaProxy(this);
        mBookmarkProxy = ProxyManager.getBookmarkProxy(this);
        // Target side is host in socket communication
        mComm = CommFactory.createCommHandler(CommFactory.HOST_SIDE);
        mDispatcher = Dispatcher.getInstance();
        // Main call back
        mMainCallback = new MainCallback();
        // Main message handler
        mMsgHandler = new MainHandler();
        // Contacts content observer
        mContactsOb = new ContactsObserver(mMsgHandler, mContactsProxy);
        // Message content observer
        mMessageOb = new MessageObserver(mMsgHandler, mMessageProxy);
        mMulMessageOb = new MulMessageObserver(mMsgHandler, mMessageProxy);
        mMessageProxy.registerSelfChangeObserver(mMulMessageOb);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        synchronized (sHasInstance) {
            if (!sHasInstance) {
                Debugger.logI("Try to start service...");
                sHasInstance = true;
                mConnector.start();
                // Check connection status after the specified timeout
                mMsgHandler.sendEmptyMessageDelayed(MSG_CHECK_TIMEOUT,
                        CommHandler.DEFAULT_TIMEOUT
                        // Just wait a little longer than default timeout in
                        // order to guarantee the server socket normally timeout
                        // and closed
                        + 500);
            }
        }
        // Init the feature option list
        Global.initFeatureOptionList();
        return START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        if (sHasInstance) {
            Debugger.logI("Try to destroy service...");
            deInit();
            sHasInstance = false;
            Debugger.logI("Service Destroyed.");
            // System.exit(0);
        }
        Intent intentFtp = new Intent(this, FtpService.class);
        this.stopService(intentFtp);
    }
    
    /**
     * Notify user about service's starting.
     */
    private void notifyStart(String text) {
//        CharSequence text = getText(R.string.noti_main_service_running);
        // Set the icon, scrolling text and timestamp
//        Notification notification = new Notification(
//                R.drawable.stat_pcsync_tool, text, System.currentTimeMillis());
        Notification notification = new Notification();
        notification.icon = R.drawable.stat_pcsync_tool;
        notification.tickerText = text;
        notification.when = System.currentTimeMillis();
        // Notification use ongoing service type. Thus, we could make it...
        // 1. Cannot be cleared when service is running
        // 2. Be automatically cleared when service is stopped
        notification.flags = Notification.FLAG_ONGOING_EVENT;
        // The PendingIntent to launch our activity if the user selects this
        // notification
        PendingIntent contentIntent = PendingIntent.getActivity(this, 0,
                new Intent(""), 0);
        // Set the info for the views that show in the notification panel.
        notification.setLatestEventInfo(this, getText(R.string.app_name), text,
                contentIntent);

        // Send the notification.
        mNotiMgr.notify(R.string.noti_main_service_started, notification);
//        this.startForeground(R.string.noti_main_service_started, notification);

        // Also display a toast to notify user.
//        Toast.makeText(this, R.string.noti_main_service_started,
//                Toast.LENGTH_SHORT).show();
//        mHasNotifiedStart = true;
    }

    /**
     * Notify user about service's stopping.
     */
    private void notifyStop() {
        if (mHasNotifiedStart) {
            mHasNotifiedStart = false;
            // Cancel the persistent notification.
            mNotiMgr.cancel(R.string.noti_main_service_started);
//            this.stopForeground(true);
            // Tell the user we stopped.
            Toast.makeText(this, R.string.noti_main_service_stopped,
                    Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * 
     */
    private void installCommunicationCallbacks() {
        mDispatcher.installCallback(ConnManageEntity.ConnManageFeatureID,
                mMainCallback);
        mDispatcher.installCallback(BaseCommand.FEATURE_MAIN, mMainCallback);
        mDispatcher
                .installCallback(BaseCommand.FEATURE_CONTACTS, mMainCallback);
        mDispatcher.installCallback(BaseCommand.FEATURE_MESSAGE, mMainCallback);
        mDispatcher.installCallback(BaseCommand.FEATURE_APPLICATION,
                mMainCallback);
        mDispatcher.installCallback(BaseCommand.FEATURE_SYNC, mMainCallback);
        mDispatcher.installCallback(BaseCommand.FEATURE_MEDIA, mMainCallback);
        mDispatcher
                .installCallback(BaseCommand.FEATURE_CALENDAR, mMainCallback);
        mDispatcher
                .installCallback(BaseCommand.FEATURE_BOOKMARK, mMainCallback);
        mDispatcher.installCallback(BaseCommand.FEATURE_CALENDAR_SYNC,
                mMainCallback);
        mDispatcher.installCallback(BaseCommand.FEATURE_BACKUP, mMainCallback);
        Debugger.logI("Communication callbacks installed...");
    }

    private void uninstallCommunicationCallbacks() {
        if (mDispatcher != null) {
            mDispatcher
                    .uninstallCallback(ConnManageEntity.ConnManageFeatureID);
            mDispatcher.uninstallCallback(BaseCommand.FEATURE_MAIN);
            mDispatcher.uninstallCallback(BaseCommand.FEATURE_CONTACTS);
            mDispatcher.uninstallCallback(BaseCommand.FEATURE_MESSAGE);
            mDispatcher.uninstallCallback(BaseCommand.FEATURE_APPLICATION);
            mDispatcher.uninstallCallback(BaseCommand.FEATURE_SYNC);
            mDispatcher.uninstallCallback(BaseCommand.FEATURE_MEDIA);
            mDispatcher.uninstallCallback(BaseCommand.FEATURE_BOOKMARK);
            mDispatcher.uninstallCallback(BaseCommand.FEATURE_CALENDAR);
            mDispatcher.uninstallCallback(BaseCommand.FEATURE_CALENDAR_SYNC);
            mDispatcher.uninstallCallback(BaseCommand.FEATURE_BACKUP);
        }
        Debugger.logI("Communication callbacks uninstalled...");
    }

    private void startThreads() {
        // Start command handling thread
        mCmdHandler.setMaxPriority(Thread.MAX_PRIORITY);
        mCmdHandler.start();
        // Start command sending thread
        mCmdSender.setMaxPriority(Thread.MAX_PRIORITY);
        mCmdSender.start();
        // Start other threads
        mIncomingSmsFinder.start();
        mSmsSender.start();
    }

    private void terminateThreads() {
        // Terminate other threads
        if (mIncomingSmsFinder != null) {
            mIncomingSmsFinder.terminate();
            mIncomingSmsFinder = null;
        }
        if (mSmsSender != null) {
            mSmsSender.terminate();
            mSmsSender = null;
        }
        // Terminate command sending thread
        if (mCmdSender != null) {
            mCmdSender.terminate();
            mCmdSender = null;
        }
        // Terminate command handling thread
        if (mCmdHandler != null) {
            mCmdHandler.terminate();
            mCmdHandler = null;
        }
    }

    private void registerEventListeners() {
        EventDispatcher.registerBatteryListener(this);
        EventDispatcher.registerPackageListener(this);
        EventDispatcher.registerSdStateListener(this);
        EventDispatcher.registerSimStateListener(this);
        EventDispatcher.registerSmsListener(this);
        EventDispatcher.registerContactsListener(this);
        EventDispatcher.registerCalendarEventListener(this);
        EventDispatcher.registerMmsListener(this);
        EventDispatcher.registerBackupAndRestoreEventListener(this);
        Debugger.logI("MainService registered for listening events...");
        // Register internal broadcast receiver
        mBR.registerAll();
        Debugger.logI("Broadcast receivers registered...");
        // Register and start content observers
        // TODO Not handled on PC side currently
        /*
         * getContentResolver().registerContentObserver(
         * ContactsContract.Data.CONTENT_URI, true, mContactsOb);
         * mContactsOb.start();
         */
        getContentResolver().registerContentObserver(SmsContent.CONTENT_URI,
                true, mMessageOb);
        getContentResolver().registerContentObserver(MmsContent.CONTENT_URI_OB,
                true, mMulMessageOb);
        mMessageOb.start();
        getContentResolver().registerContentObserver(
                CalendarEventContent.CONTENT_URI, true, mCalendarEventOb);
        mMulMessageOb.start();
        mCalendarEventOb.start();
        Debugger.logI("Content observers registered and started...");
    }

    private void unregisterEventListeners() {
        // Unregister content observers
        // TODO Not handled on PC side currently
        /*
         * mContactsOb.stop();
         * getContentResolver().unregisterContentObserver(mContactsOb);
         */
        mMessageOb.stop();
        getContentResolver().unregisterContentObserver(mMessageOb);
        mMulMessageOb.stop();
        mCalendarEventOb.stop();
        getContentResolver().unregisterContentObserver(mMulMessageOb);
        getContentResolver().unregisterContentObserver(mCalendarEventOb);
        Debugger.logI("Content observers stopped and unregistered...");
        // Unregister receivers
        if (mBR != null) {
            mBR.unregisterAll();
        }
        Debugger.logI("Broadcast receivers unregistered...");
        EventDispatcher.unregisterListener(this);
        Debugger.logI("MainService unregistered for listening events...");
    }

    private void init() {
        // Install communication callbacks
        installCommunicationCallbacks();
        // Start threads
        startThreads();
        // Register main service for listening events
        registerEventListeners();
        // Notify user about service's starting.
        notifyStart(getString(R.string.noti_main_service_running));
        Toast.makeText(this, R.string.noti_main_service_started,
                Toast.LENGTH_SHORT).show();
        mHasNotifiedStart = true;
    }

    private void deInit() {
        // Unregister event listeners
        unregisterEventListeners();
        // Terminate threads
        terminateThreads();
        // Close socket connection
        if (mComm != null) {
            try {
                setConnected(!mComm.closeConnection());
            } catch (IOException e) {
                Debugger.logE(e);
            }
            mComm = null;
        }
        Debugger.logI("Socket connection closed...");
        // Uninstall communication callbacks
        // This should be done after connection closed
        uninstallCommunicationCallbacks();
        // Notify user about service's stopping.
        notifyStop();
    }

    private synchronized boolean enqueueHandleCommand(BaseCommand cmd) {
        if (null != mCmdHandler) {
            return mCmdHandler.enqueue(cmd, true);
        } else {
            Debugger.logW(new Object[] { cmd },
                    "Command handler thread is null.");
            return false;
        }
    }

    private synchronized boolean enqueueSendCommand(BaseCommand cmd) {
        if (null != mCmdSender) {
            return mCmdSender.enqueue(cmd, true);
        } else {
            Debugger.logW(new Object[] { cmd },
                    "Command sender thread is null.");
            return false;
        }
    }

    /**
     * Should be called instead of stopSelf() to stop service safe after socket
     * connection is established. When called, service will only stop after
     * handle all commands left.
     */
    public void safeStop() {
        Debugger.logI("Wait for safe stopping...");
        mPrepareToStop = true;
        // Notify user about service's stopping.
        notifyStop();
    }

    /**
     * Executed when handling a response command.
     * 
     * @param rspCmd
     *            Response command to handle.
     */
    public void onRespond(BaseCommand rspCmd) {
        switch (mResponseState) {
        case RSP_ST_SEND_SINGLE:
            // Request is a single command, so send a single response
            enqueueSendCommand(rspCmd);
            break;

        case RSP_ST_APPEND_BATCH:
            // Request is a command batch, so append the single response into
            // response batch
            appendBatch(rspCmd);
            break;

        case RSP_ST_SEND_BATCH:
            // All requests in the request batch is handled, so send the whole
            // response batch
            appendBatch(rspCmd);
            enqueueSendCommand((BaseCommand) mCommandBatch);
            
            mResponseState = RSP_ST_SEND_SINGLE;
            break;

        default:
            break;
        }
    }

    /**
     * Append one command to the end of the current command batch.
     * 
     * @param cmd
     *            Command to append.
     */
    private synchronized void appendBatch(BaseCommand cmd) {
        if (null == cmd) {
            Debugger.logW(new Object[] { cmd }, "Command is null!");
            return;
        }

        if (mCommandBatch != null) {
            Debugger.logI(new Object[] { cmd }, "FeatureID="
                    + cmd.getFeatureID() + ", Token=" + cmd.getToken());
            mCommandBatch.getCommands().add(cmd);
        }
    }

    // @Override
    public void onBatteryStateChanged(Event event) {
        NotifyBatteryReq req = new NotifyBatteryReq();
        req.setToken(mDispatcher.getToken());
        req.setBatteryLevel(event.getInt(IBatteryListener.LEVEL));
        req.setBatteryScale(event.getInt(IBatteryListener.SCALE));

        enqueueSendCommand(req);
    }

    // @Override
    public void onSdStateChanged(Event event) {
        boolean mounted = event.getBoolean(ISdStateListener.MOUNTED);
        boolean writeable = event.getBoolean(ISdStateListener.WRITEABLE);

        NotifySDStateReq req = new NotifySDStateReq();
        req.setToken(mDispatcher.getToken());
        if (mounted) {
            req.setSdMounted(true);
            req.setSdWriteable(writeable);
            req.setSdCardPath(SystemInfoProxy.getSdPath());
            req.setSdCardTotalSpace(SystemInfoProxy.getSdTotalSpace());
            req.setSdCardAvailableSpace(SystemInfoProxy.getSdAvailableSpace());
        } else {
            req.setSdMounted(false);
            req.setSdWriteable(false);
            req.setSdCardPath(null);
            req.setSdCardTotalSpace(-1);
            req.setSdCardAvailableSpace(-1);
        }
        enqueueSendCommand(req);
    }

    // @Override
    public void onSimStateChanged(final Event event) {

        NotifySimStateReq req = new NotifySimStateReq();
        req.setToken(mDispatcher.getToken());
        req.setSimAccessible(mSysInfoProxy.isSimAccessible());
        req.setSim1Accessible(SystemInfoProxy.isSim1Accessible());
        req.setSim2Accessible(SystemInfoProxy.isSim2Accessible());
        req.setSim3Accessible(SystemInfoProxy.isSim3Accessible());
        req.setSim4Accessible(SystemInfoProxy.isSim4Accessible());
        // Added by Shaoying Han 2011-04-08
        req.setSimDetailInfo(Global
                .getSimInfoBySlot(com.mediatek.apst.util.entity.message.Message.SIM_ID));
        req.setSim1DetailInfo(Global
                .getSimInfoBySlot(com.mediatek.apst.util.entity.message.Message.SIM1_ID));
        req.setSim2DetailInfo(Global
                .getSimInfoBySlot(com.mediatek.apst.util.entity.message.Message.SIM2_ID));
        req.setSim3DetailInfo(Global
                .getSimInfoBySlot(com.mediatek.apst.util.entity.message.Message.SIM3_ID));
        req.setSim4DetailInfo(Global
                .getSimInfoBySlot(com.mediatek.apst.util.entity.message.Message.SIM4_ID));
        if (Config.MTK_GEMINI_SUPPORT)
        {
        	req.getSlotInfoList().add(Global
                    .getSimInfoBySlot(SimDetailInfo.SLOT_ID_ONE));
            req.getSlotInfoList().add(Global
                    .getSimInfoBySlot(SimDetailInfo.SLOT_ID_TWO));
            if (Config.MTK_3SIM_SUPPORT)
            {
            	req.getSlotInfoList().add(Global
                        .getSimInfoBySlot(SimDetailInfo.SLOT_ID_THREE));
            }
            if (Config.MTK_4SIM_SUPPORT)
            {
            	req.getSlotInfoList().add(Global
                        .getSimInfoBySlot(SimDetailInfo.SLOT_ID_FOUR));
            }
        } else {
        	SimDetailInfo detailInfo = Global.getSimInfoBySlot(SimDetailInfo.SLOT_ID_SINGLE);
            detailInfo.setAccessible(mSysInfoProxy.isSimAccessible());
            req.getSlotInfoList().add(detailInfo);
        }
        req.setInfoChanged(event.getBoolean(ISimStateListener.SIM_INFO_FLAG));
        req.setContactsCount(mContactsProxy.getAvailableContactsCount());
        enqueueSendCommand(req);

    }

    // @Override
    public void onSmsSent(Event event) {
        // Allow send next
        mSmsSender.allowSendNext();
        final long smsId = event.getLong(ISmsListener.SMS_ID);
        final long date = event.getLong(ISmsListener.DATE);
        final boolean sent = event.getBoolean(ISmsListener.SENT);
        new Thread() {

            // @Override
            public void run() {
                long ids[] = new long[1];
                long dates[] = new long[1];
                int box;
                ids[0] = smsId;
                dates[0] = date;
                if (sent) {
                    box = Sms.BOX_SENT;
                } else {
                    box = Sms.BOX_FAILED;
                }
                // Move SMS from outbox to sent/failed
                mMessageProxy.moveSmsToBox(ids, true, dates, box);
                NotifyMessageSentReq req = new NotifyMessageSentReq();
                req.setToken(mDispatcher.getToken());
                req.setId(smsId);
                req.setDate(date);
                req.setSent(sent);
                req
                        .setMessageType(com.mediatek.apst.util.entity.message.Message.TYPE_SMS);

                enqueueSendCommand(req);
            }

        }.start();
    }

    // @Override
    public void onSmsReceived(Event event) {
        mIncomingSmsFinder.appendTask(new NewSmsFinder.Clue(event
                .getLong(ISmsListener.AFTER_TIME_OF), event
                .getString(ISmsListener.ADDRESS), event
                .getString(ISmsListener.BODY), Sms.BOX_INBOX));
    }

    // @Override
    public void onSmsInserted(Event event) {
        if (!event.getBoolean(ISmsListener.BY_SELF)) {
            NotifyNewMessageReq req = new NotifyNewMessageReq();
            req.setToken(mDispatcher.getToken());
            req.setNewMessage((Sms) (event.get(ISmsListener.SMS)));

            enqueueSendCommand(req);
        }
    }

    // @Override
    public void onMmsInserted(Event event) {
        Debugger.logI(new Object[] { event }, "mms inserted");
        if (!event.getBoolean(IMmsListener.BY_SELF)) {
            NotifyNewMessageReq req = new NotifyNewMessageReq();
            req.setToken(mDispatcher.getToken());
            req.setNewMessage((Mms) (event.get(IMmsListener.MMS)));

            enqueueSendCommand(req);
        }
    }

    public void onMmsReceived(Event event) {
        Debugger.logI(new Object[] { event }, "mms received");
        NotifyNewMessageReq req = new NotifyNewMessageReq();
        req.setToken(mDispatcher.getToken());
        req.setNewMessage((Mms) (event.get(IMmsListener.MMS)));
        enqueueSendCommand(req);
    }

    public void onMmsSent(Event event) {

    }

    // @Override
    public void onContactsContentChanged(Event event) {
        if (!event.getBoolean(IContactsListener.BY_SELF)) {
            NotifyContactsContentChangeReq req = new NotifyContactsContentChangeReq();
            req.setToken(mDispatcher.getToken());

            enqueueSendCommand(req);
        }
    }

    // @Override
    public void onPackageAdded(Event event) {
        final int uid = event.getInt(IPackageListener.UID);
        new Thread() {

            // @Override
            public void run() {
                ArrayList<ApplicationInfo> apps = mApplicationProxy
                        .getApplicationsForUid(uid);
                if (apps.size() > 0) {
                    NotiPackageInstalledReq req = new NotiPackageInstalledReq();
                    req.setToken(mDispatcher.getToken());
                    req.setUid(uid);
                    req.setApplications(apps);

                    enqueueSendCommand(req);
                }
            }

        }.start();
    }

    // @Override
    public void onPackageDataCleared(Event event) {
        int uid = event.getInt(IPackageListener.UID);
        try {
            final int contactsAppUid = this.getPackageManager().getPackageInfo(
                    "com.android.contacts", 0).applicationInfo.uid;
            if (contactsAppUid == uid) {
                Debugger.logW(new Object[] { uid },
                        "Contacts package data cleared!");
                NotifyContactsDataClearedReq req = new NotifyContactsDataClearedReq();
                req.setToken(mDispatcher.getToken());

                enqueueSendCommand(req);
                return;
            }
            final int messageAppUid = this.getPackageManager().getPackageInfo(
                    "com.android.mms", 0).applicationInfo.uid;
            if (messageAppUid == uid) {
                Debugger.logW(new Object[] { uid },
                        "Messaging package data cleared! ");
                NotifyMessageDataClearedReq req = new NotifyMessageDataClearedReq();
                req.setToken(mDispatcher.getToken());

                enqueueSendCommand(req);
            }
        } catch (NameNotFoundException e) {
            Debugger.logE(new Object[] { uid }, null, e);
        }
    }
    
    public void onEmailBackupEnd(Event event) {
        Debugger.logI(new Object[] { event }, "Email backup end");
        GetEmailForBackupRsp rsp = new GetEmailForBackupRsp(mDispatcher.getToken());
        rsp.setPath(event.getString(IBackupAndRestoreListener.EMAIL_PATH));
        rsp.setFileName(event.getString(IBackupAndRestoreListener.EMAIL_FILE_NAME));
        rsp.setSuccess((event.getBoolean(IBackupAndRestoreListener.EMAIL_IS_SUCCESSFUL)));
        enqueueSendCommand(rsp);
    }
    
    public void onEmailRestoreEnd(Event event) {
        Debugger.logI(new Object[] { event }, "email restore end");
        // Do noting until there has the requirement
    }

    // @Override
    public void onCalendarEventContentChanged(Event event) {
        if (!event.getBoolean(ICalendarEventListener.BY_SELF)) {
            NotifyCalendarEventChangeReq req = new NotifyCalendarEventChangeReq();
            req.setToken(mDispatcher.getToken());
            req.setEvent((CalendarEvent) event
                    .get(ICalendarEventListener.CALENDAREVENT));

            enqueueSendCommand(req);
        }
    }

    // ==============================================================
    // Inner & Nested classes
    // ==============================================================
    class MainCallback implements ICallback {

        // @Override
        public boolean execute(TransportEntity entity) {
            Debugger.logI(new Object[] { entity }, "Entity received.");
            if (null == entity) {
                Debugger.logW("Entity is null.");

                return false;
            } else if (entity instanceof BaseCommand) {
                // Entity is a command
                BaseCommand cmd = (BaseCommand) entity;
                Debugger.logI("Entity is command: " + cmd);

                return enqueueHandleCommand(cmd);
            } else if (entity.getFeatureID() == ConnManageEntity.ConnManageFeatureID) {
                // Entity is a connection status entity
                ConnManageEntity connEntity = (ConnManageEntity) entity;
                int infoId = connEntity.getInfoID();
                Debugger.logI("Entity is connection status entity, infoID="
                        + infoId);

                switch (infoId) {
                case ConnManageEntity.disconnect_info_id:
                    // Normal disconnection
                    Debugger.logI("Disconnecting...");
                    mMsgHandler.sendEmptyMessage(MSG_SAFE_STOP);
                    break;

                case ConnManageEntity.expt_info_id:
                    // Unexcepted disconnection
                    Debugger.logE("Unexpected disconnection.");
                    mMsgHandler.sendEmptyMessage(MSG_SAFE_STOP);
                    break;

                default:
                    break;
                }

                return true;
            } else {
                return false;
            }
        }

    }

    // Threads -----------------------------------------------------------------
    // Socket connection creating thread
    class Connector extends Thread {

        // @Override
        public void run() {
            Debugger.logI("Thread started.");
            setConnected(mComm.createConnection());
            // Create socket connection
            if (isConnected()) {
                Debugger.logI("Create connection successfully!");
                mMsgHandler.sendEmptyMessage(MSG_CONNECTED);
            } else {
                Debugger.logW("Create connection failed.");
                mMsgHandler.sendEmptyMessage(MSG_FORCE_STOP);
            }
            Debugger.logI("Thread terminated.");
        }

    }

    // Command sending thread
    class CommandSender extends BlockingCommandHandlingThread {

        public CommandSender(int queueCapacity) {
            super(queueCapacity);
        }

        // @Override
        public String getClassName() {
            return "MainService$CommandSender";
        }

        // @Override
        public void handle(BaseCommand cmd) {
            if (null == cmd) {
                return;
            } else {
                send(cmd);
            }
        }

        /**
         * Send out commands.
         * 
         * @param cmd
         *            Command to send.
         */
        private void send(BaseCommand cmd) {
            if (null != mComm) {
                int token = mComm.sendPrimitive(cmd, cmd.getFeatureID());
                if (-1 != token) {
                    Debugger.logI(this.getClassName(), "send",
                            new Object[] { cmd }, "Successfully, FeatureID="
                                    + cmd.getFeatureID());
                } else {
                    Debugger.logW(this.getClassName(), "send",
                            new Object[] { cmd }, "Failed, FeatureID="
                                    + cmd.getFeatureID());
                }
            } else {
                Debugger.logW(this.getClassName(), "send",
                        new Object[] { cmd },
                        "Failed, socket connection is null!");
            }
        }

    }

    // Command handling thread
    class CommandHandler extends BlockingCommandHandlingThread {

        public CommandHandler(int queueCapacity) {
            super(queueCapacity);
        }

        // @Override
        public String getClassName() {
            return "MainService$CommandHandler";
        }

        // @Override
        public void handle(BaseCommand cmd) {
            if (null == cmd) {
                if (mPrepareToStop) {
                    // If no command is left to handle, and currently service is
                    // waiting to stop, stop service.
                    mMsgHandler.sendEmptyMessage(MSG_FORCE_STOP);
                    this.terminate();
                }
                return;
            }

            Debugger.logI(this.getClassName(), "handle", new Object[] { cmd },
                    "FeatureID=" + cmd.getFeatureID() + ", Token="
                            + cmd.getToken());
            boolean handled = false;
            if (BaseCommand.FEATURE_MAIN == cmd.getFeatureID()) {
                handled = handleMainFrameFeatures(cmd);
            } else if (BaseCommand.FEATURE_CONTACTS == cmd.getFeatureID()) {
                handled = handleContactsFeatures(cmd);
            } else if (BaseCommand.FEATURE_MESSAGE == cmd.getFeatureID()) {
                handled = handleMessageFeatures(cmd);
            } else if (BaseCommand.FEATURE_APPLICATION == cmd.getFeatureID()) {
                handled = handleApplicationFeatures(cmd);
            } else if (BaseCommand.FEATURE_SYNC == cmd.getFeatureID()) {
                handled = handleSyncFeatures(cmd);
            } else if (BaseCommand.FEATURE_MEDIA == cmd.getFeatureID()) {
                handled = handleMediaFeatures(cmd);
            } else if (BaseCommand.FEATURE_CALENDAR == cmd.getFeatureID()) {
                handled = handleCalendarFeatures(cmd);
            } else if (BaseCommand.FEATURE_CALENDAR_SYNC == cmd.getFeatureID()) {
                handled = handleCalendarSyncFeatures(cmd);
            } else if (BaseCommand.FEATURE_BOOKMARK == cmd.getFeatureID()) {
                handled = handleBookmarkFeatures(cmd);
            } else if (BaseCommand.FEATURE_BACKUP == cmd.getFeatureID()) {
                handled = handleBackupFeatures(cmd);
            } else {
                Debugger.logE(this.getClassName(), "handle",
                        new Object[] { cmd }, "Unsupported feature.");
            }

            if (!handled) {
                Debugger.logE(this.getClassName(), "handle",
                        new Object[] { cmd }, "): Unsupported command.");
                onRespond(new UnsupportedRequestResponse(cmd.getFeatureID(),
                        cmd.getToken()));
            }
        }

        /**
         * Handle commands of main frame feature.
         * 
         * @param cmd
         *            Command to handle.
         * @return True if command is handled, false if not.
         */
        public boolean handleMainFrameFeatures(BaseCommand cmd) {
            final int reqToken = cmd.getToken();
            if (cmd instanceof GreetingReq) {
                GreetingRsp rsp = new GreetingRsp(reqToken);
                // Get data and fill them in response command
                rsp.setVersionCode(Config.VERSION_CODE);
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof SysInfoBatchReq) {
                mCommandBatch = new SysInfoBatchRsp(reqToken);
                SysInfoBatchReq reqSysInfoBatch = (SysInfoBatchReq) cmd;
                int i = 0;
                mResponseState = RSP_ST_APPEND_BATCH;
                // Get data and fill them in response command
                for (BaseCommand innerCmd : reqSysInfoBatch.getCommands()) {
                    if (++i == reqSysInfoBatch.getCommands().size()) {
                        // It's the last command in batch, we can send out
                        // the response batch now
                        mResponseState = RSP_ST_SEND_BATCH;
                    }
                    handle(innerCmd);
                }
                mResponseState = RSP_ST_SEND_SINGLE;
            } else if (cmd instanceof GetSysInfoReq) {
                GetSysInfoRsp rsp = new GetSysInfoRsp(reqToken);
                // Get data and fill them in response command
                // Device & Firmware
                rsp.setDevice(SystemInfoProxy.getDevice());
                rsp.setManufacturer(SystemInfoProxy.getManufacturer());
                rsp.setFirmwareVersion(SystemInfoProxy.getFirmwareVersion());
                rsp.setModel(SystemInfoProxy.getModel());

                // Get feature option 2012-5-15 mtk54043
                
                rsp.setFeatureOptionList(FeatureOptionControl.getFeatureList());

                // Storage
                if (SystemInfoProxy.isSdMounted()) {
                    rsp.setSdMounted(true);
                    rsp.setSdWriteable(SystemInfoProxy.isSdWriteable());
                    rsp.setSdCardPath(SystemInfoProxy.getSdPath());
                    rsp.setSdCardTotalSpace(SystemInfoProxy.getSdTotalSpace());
                    rsp.setSdCardAvailableSpace(SystemInfoProxy
                            .getSdAvailableSpace());
                } else {
                    rsp.setSdMounted(false);
                    rsp.setSdWriteable(false);
                    rsp.setSdCardPath(null);
                    rsp.setSdCardTotalSpace(-1);
                    rsp.setSdCardAvailableSpace(-1);
                }
                rsp.setSDCardAndEmmcState(mSysInfoProxy.checkSDCardState());
                rsp.setInternalTotalSpace(SystemInfoProxy
                        .getInternalTotalSpace());
                rsp.setInternalAvailableSpace(SystemInfoProxy
                        .getInternalAvailableSpace());
                // Applications & Data
                rsp.setContactsCount(mContactsProxy
                                .getAvailableContactsCount());
                // rsp.setSimContactsCount(mContactsProxy.getSimContactsCount());
                rsp.setMessagesCount(mMessageProxy.getMessagesCount());
                rsp.setApplicationsCount(mApplicationProxy
                        .getApplicationsCount());
                // SIM
                if (Config.MTK_GEMINI_SUPPORT) {
                    rsp.setGemini(true);
                    rsp.setGemini3Sim(Config.MTK_3SIM_SUPPORT);
                    rsp.setGemini4Sim(Config.MTK_4SIM_SUPPORT);
                    
                    rsp.setSim1Accessible(SystemInfoProxy.isSim1Accessible());
                    rsp.setSim2Accessible(SystemInfoProxy.isSim2Accessible());
                    rsp.setSim3Accessible(SystemInfoProxy.isSim3Accessible());
                    rsp.setSim4Accessible(SystemInfoProxy.isSim4Accessible());
                    rsp.setSim1Info(Global
                            .getSimInfoBySlot(SimDetailInfo.SLOT_ID_ONE));
                    rsp.setSim2Info(Global
                            .getSimInfoBySlot(SimDetailInfo.SLOT_ID_TWO));
                    rsp.setSim3Info(Global
                            .getSimInfoBySlot(SimDetailInfo.SLOT_ID_THREE));
                    rsp.setSim4Info(Global
                            .getSimInfoBySlot(SimDetailInfo.SLOT_ID_FOUR));
                    
                    //2013-01-30 for Gemini plus 2.0
                    rsp.getSlotInfoList().add(Global
                            .getSimInfoBySlot(SimDetailInfo.SLOT_ID_ONE));
                    rsp.getSlotInfoList().add(Global
                            .getSimInfoBySlot(SimDetailInfo.SLOT_ID_TWO));
                    if (Config.MTK_3SIM_SUPPORT)
                    {
                    	rsp.getSlotInfoList().add(Global
                                .getSimInfoBySlot(SimDetailInfo.SLOT_ID_THREE));
                    }
                    if (Config.MTK_4SIM_SUPPORT)
                    {
                    	rsp.getSlotInfoList().add(Global
                                .getSimInfoBySlot(SimDetailInfo.SLOT_ID_FOUR));
                    }
                    
                    Debugger.logD(this.getClassName(), "handle",
                            new Object[] { cmd }, "Dual-SIM | SIM:"
                                    + rsp.isSimAccessible() + " | SIM1:"
                                    + rsp.isSim1Accessible() + " | SIM2:"
                                    + rsp.isSim2Accessible() + " | SIM3:"
                                    + rsp.isSim3Accessible() + " | SIM4:"
                                    + rsp.isSim4Accessible());
                } else {
                    rsp.setGemini(false);
                    rsp.setSimAccessible(mSysInfoProxy.isSimAccessible());
                    rsp.setSimInfo(Global
                            .getSimInfoBySlot(SimDetailInfo.SLOT_ID_SINGLE));
                    
                    //2013-01-30 for Gemini plus 2.0
                    SimDetailInfo detailInfo = Global.getSimInfoBySlot(SimDetailInfo.SLOT_ID_SINGLE);
                    detailInfo.setAccessible(mSysInfoProxy.isSimAccessible());
                    rsp.getSlotInfoList().add(detailInfo);
                    
                    Debugger.logD(this.getClassName(), "handle",
                            new Object[] { cmd }, "SINGLE_SIM | SIM:"
                                    + rsp.isSimAccessible() + " | SIM1:"
                                    + rsp.isSim1Accessible() + " | SIM2:"
                                    + rsp.isSim2Accessible());
                }
                rsp.setSimInfoList(Global.getAllSIMList());

                // Battery
                if (mBR != null) {
                    rsp.setBatteryLevel(mBR.getBatteryLevel());
                    rsp.setBatteryScale(mBR.getBatteryScale());
                } else {
                    rsp.setBatteryLevel(0);
                    rsp.setBatteryScale(0);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof GetStorageReq) {
                GetStorageRsp rsp = new GetStorageRsp(reqToken);
                // Get data and fill them in response command
                // Storage
                if (SystemInfoProxy.isSdMounted()) {
                    rsp.setSdMounted(true);
                    rsp.setSdWriteable(SystemInfoProxy.isSdWriteable());
                    rsp.setSdCardPath(SystemInfoProxy.getSdPath());
                    rsp.setSdCardTotalSpace(SystemInfoProxy.getSdTotalSpace());
                    rsp.setSdCardAvailableSpace(SystemInfoProxy
                            .getSdAvailableSpace());
                } else {
                    rsp.setSdMounted(false);
                    rsp.setSdWriteable(false);
                    rsp.setSdCardPath(null);
                    rsp.setSdCardTotalSpace(-1);
                    rsp.setSdCardAvailableSpace(-1);
                }
                rsp.setInternalTotalSpace(SystemInfoProxy
                        .getInternalTotalSpace());
                rsp.setInternalAvailableSpace(SystemInfoProxy
                        .getInternalAvailableSpace());
                // Send response command or append it to batch
                onRespond(rsp);
            } else {
                return false;
            }
            return true;
        }

        /**
         * Handle commands of 'Contacts' feature.
         * 
         * @param cmd
         *            Command to handle.
         * @return True if command is handled, false if not.
         */
        public boolean handleContactsFeatures(BaseCommand cmd) {
            final int reqToken = cmd.getToken();
            if (cmd instanceof ContactsBatchReq) {
                mCommandBatch = new ContactsBatchRsp(reqToken);
                ContactsBatchReq reqContactsBatch = (ContactsBatchReq) cmd;
                int i = 0;
                mResponseState = RSP_ST_APPEND_BATCH;
                // Get data and fill them in response command
                for (BaseCommand innerCmd : reqContactsBatch.getCommands()) {
                    if (++i == reqContactsBatch.getCommands().size()) {
                        // It's the last command in batch, we can send out
                        // the response batch now
                        mResponseState = RSP_ST_SEND_BATCH;
                    }
                    handle(innerCmd);
                }
                mResponseState = RSP_ST_SEND_SINGLE;
            } else if (cmd instanceof GetDetailedContactReq) {
                GetDetailedContactRsp rsp = new GetDetailedContactRsp(reqToken);
                // Get data and fill them in response command
                GetDetailedContactReq req = (GetDetailedContactReq) cmd;
                RawContact detailedContact = mContactsProxy.getContact(req
                        .getContactId(), true);
                if (null != detailedContact) {
                    rsp.setDetailedContact(detailedContact, Global
                            .getByteBuffer(), Config.VERSION_CODE);
                } else {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof AddGroupReq) {
                AddGroupRsp rsp = new AddGroupRsp(reqToken);
                // Get data and fill them in response command
                AddGroupReq req = (AddGroupReq) cmd;
                Group group = req.getGroup();
                long insertedGroupId = mContactsProxy.insertGroup(group);
                rsp.setInsertedId(insertedGroupId);
                if (insertedGroupId == DatabaseRecordEntity.ID_NULL) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    rsp.setResult(group);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof AddContactReq) {
                AddContactRsp rsp = new AddContactRsp(reqToken);
                // Get data and fill them in response command
                AddContactReq req = (AddContactReq) cmd;
                // For PC side
                rsp.setFromFeature(req.getFromFeature());
                RawContact contact = req.getContact();
                long result = mContactsProxy.insertContact(contact, true);
                if (result < 0) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                    rsp.setInsertedId(result);
                } else {
                    rsp.setStatusCode(ResponseCommand.SC_OK);
                    rsp.setInsertedId(result);
                    rsp.setResult(mContactsProxy.getContact(result, true));
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof AddSimContactReq) {
                AddSimContactRsp rsp = new AddSimContactRsp(reqToken);
                // Get data and fill them in response command
                AddSimContactReq req = (AddSimContactReq) cmd;
                BaseContact contact = req.getContact();
                long insertedSimContactId = mContactsProxy.insertSimContact(
                        contact.getDisplayName(), contact.getPrimaryNumber(),
                        contact.getStoreLocation());
                if (insertedSimContactId == DatabaseRecordEntity.ID_NULL) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof AddContactDataReq) {
                AddContactDataRsp rsp = new AddContactDataRsp(reqToken);
                // Get data and fill them in response command
                AddContactDataReq req = (AddContactDataReq) cmd;
                ContactData contactData = req.getData();
                long insertedContactDataId = mContactsProxy.insertContactData(
                        contactData, true);
                rsp.setInsertedId(insertedContactDataId);
                if (insertedContactDataId == DatabaseRecordEntity.ID_NULL) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    rsp.setResult(contactData);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof AddGroupMembershipReq) {
                AddGroupMembershipRsp rsp = new AddGroupMembershipRsp(reqToken);
                // Get data and fill them in response command
                AddGroupMembershipReq req = (AddGroupMembershipReq) cmd;
                Group group = req.getGroupEntry();
                int[] simIndex = null;
                if (FeatureOptionControl.CONTACT_N_USIMGROUP != 0) {
                    simIndex = req.getSimIndexes();
                }
                if (simIndex == null
                        || FeatureOptionControl.CONTACT_N_USIMGROUP == 0) {
                    rsp.setInsertedIds(mContactsProxy.insertGroupMembership(req
                            .getContactIds(), req.getGroupId()));
                } else {
                    rsp.setInsertedIds(mContactsProxy
                            .insertGroupMembership(req.getContactIds(), req
                                    .getGroupId(), group, simIndex));
                }

                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof UpdateGroupReq) {
                UpdateGroupRsp rsp = new UpdateGroupRsp(reqToken);
                // Get data and fill them in response command
                UpdateGroupReq req = (UpdateGroupReq) cmd;
                Group group = req.getNewOne();
                String oldName = null;
                if (FeatureOptionControl.CONTACT_N_USIMGROUP != 0) {
                    oldName = req.getOldName();
                }
                int updateGroupCount = 0;
                if (oldName == null
                        || FeatureOptionControl.CONTACT_N_USIMGROUP == 0) {
                    updateGroupCount = mContactsProxy.updateGroup(req
                            .getUpdateId(), group);
                } else {
                    updateGroupCount = mContactsProxy.updateGroup(req
                            .getUpdateId(), group, oldName);
                }

                if (updateGroupCount < 1) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof UpdateRawContactReq) {
                UpdateRawContactRsp rsp = new UpdateRawContactRsp(reqToken);
                // Get data and fill them in response command
                UpdateRawContactReq req = (UpdateRawContactReq) cmd;
                RawContact contact = req.getNewOne();
                int updateRawContactCount = mContactsProxy.updateContact(req
                        .getUpdateId(), RawContact.SOURCE_NONE, null, null,
                        contact, false/* Do not update PIM */);
                if (updateRawContactCount < 1) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof UpdateDetailedContactReq) {
                UpdateDetailedContactRsp rsp = new UpdateDetailedContactRsp(
                        reqToken);
                // Get data and fill them in response command
                UpdateDetailedContactReq req = (UpdateDetailedContactReq) cmd;
                RawContact contact = req.getNewOne();
                // int updateDetailedContactCount =
                // mContactsProxy.updateContact(
                // req.getUpdateId(), req.getSourceLocation(),
                // req.getSimName(), req.getSimNumber(), contact,
                // true/*Update PIM data*/);
                int updateDetailedContactCount = mContactsProxy
                        .updateContact(req.getUpdateId(), req
                                .getSourceLocation(), req.getSimName(), req
                                .getSimNumber(), req.getSimEmail(), contact,
                                true/* Update PIM data */);
                if (updateDetailedContactCount < 1) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    rsp.setResult(mContactsProxy.getContact(req.getUpdateId(),
                            true));
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof UpdateSimContactReq) {
                UpdateSimContactRsp rsp = new UpdateSimContactRsp(reqToken);
                // Get data and fill them in response command
                UpdateSimContactReq req = (UpdateSimContactReq) cmd;
                int updateSimContactCount = mContactsProxy.updateSimContact(req
                        .getOldName(), req.getOldNumber(), req.getNewOne()
                        .getDisplayName(), req.getNewOne().getPrimaryNumber(),
                        req.getSimId());
                if (updateSimContactCount < 1) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof UpdateContactDataReq) {
                UpdateContactDataRsp rsp = new UpdateContactDataRsp(reqToken);
                // Get data and fill them in response command
                UpdateContactDataReq req = (UpdateContactDataReq) cmd;
                ContactData contactData = req.getNewOne();
                int updateContactDataCount = mContactsProxy.updateContactData(
                        req.getUpdateId(), contactData, true);
                if (updateContactDataCount < 1) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof DeleteGroupReq) {
                DeleteGroupRsp rsp = new DeleteGroupRsp(reqToken);
                // Get data and fill them in response command
                DeleteGroupReq req = (DeleteGroupReq) cmd;
                
                ArrayList<Group> groups = null;
                if (FeatureOptionControl.CONTACT_N_USIMGROUP != 0) {
                    groups = req.getGroups();
                }

                if (groups == null
                        || FeatureOptionControl.CONTACT_N_USIMGROUP == 0) {
                    rsp.setDeleteResults(mContactsProxy.deleteGroup(req
                            .getDeleteIds()));
                } else {
                    rsp.setDeleteResults(mContactsProxy.deleteGroup(req
                            .getDeleteIds(), groups));
                }

                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof DeleteContactReq) {
                DeleteContactRsp rsp = new DeleteContactRsp(reqToken);
                // Get data and fill them in response command
                DeleteContactReq req = (DeleteContactReq) cmd;
                // rsp.setDeleteResults(mContactsProxy.deleteContacts(
                // req.getDeleteIds(), false, req.getSourceLocation(),
                // req.getSimNames(), req.getSimNumbers()));
                rsp.setDeleteResults(mContactsProxy
                        .deleteContacts(req.getDeleteIds(), false, req
                                .getSourceLocation(), req.getSimNames(), req
                                .getSimNumbers(), req.getSimEmails()));
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof DeleteAllContactsReq) {
                DeleteAllContactsRsp rsp = new DeleteAllContactsRsp(reqToken);
                // Get data and fill them in response command
                rsp.setDeleteCount(mContactsProxy.deleteAllContacts(true));
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof DeleteSimContactReq) {
                DeleteSimContactRsp rsp = new DeleteSimContactRsp(reqToken);
                // Get data and fill them in response command
                DeleteSimContactReq req = (DeleteSimContactReq) cmd;
                rsp.setDeleteResults(mContactsProxy.deleteSimContacts(req
                        .getNames(), req.getNumbers(), req.getSimId()));
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof DeleteContactDataReq) {
                DeleteContactDataRsp rsp = new DeleteContactDataRsp(reqToken);
                // Get data and fill them in response command
                DeleteContactDataReq req = (DeleteContactDataReq) cmd;

                Group group = req.getGroupEntry();
                int[] simIndex = null;
                simIndex = req.getSimIndexes();
               
                if (simIndex == null || FeatureOptionControl.CONTACT_N_USIMGROUP == 0) {
                    rsp.setDeleteResults(mContactsProxy.deleteContactData(req
                            .getDeleteIds()));
                } else {
                    rsp.setDeleteResults(mContactsProxy.deleteContactData(
                                    req.getDeleteIds(), req.getGroupId(),
                                    group, simIndex));
                }

                // rsp.setDeleteResults(mContactsProxy.deleteContactData(
                // req.getDeleteIds()));
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof AsyncGetAllGroupsReq) {
                mContactsProxy.asyncGetAllGroups(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        AsyncGetAllGroupsRsp rsp = new AsyncGetAllGroupsRsp(
                                reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        // Send response command or append it to batch
                        onRespond(rsp);
                    }

                }, Global.getByteBuffer());
            } else if (cmd instanceof AsyncGetAllSimContactsReq) {
                final boolean simAccesible = mSysInfoProxy.isSimAccessible();
                if (simAccesible) {
                    mContactsProxy.asyncGetAllSimContacts(
                            new IRawBlockConsumer() {

                                // @Override
                                public void consume(byte[] block, int blockNo,
                                        int totalNo) {
                                    AsyncGetAllSimContactsRsp rsp = new AsyncGetAllSimContactsRsp(
                                            reqToken);
                                    rsp.setRaw(block);
                                    rsp.setProgress(blockNo);
                                    rsp.setTotal(totalNo);
                                    // Send response command or append it to
                                    // batch
                                    onRespond(rsp);
                                }

                            }, Global.getByteBuffer());
                } else {
                    AsyncGetAllSimContactsRsp rsp = new AsyncGetAllSimContactsRsp(
                            reqToken);
                    rsp.setRaw(null);
                    rsp.setProgress(0);
                    rsp.setTotal(0);
                    // Send response command or append it to batch
                    onRespond(rsp);
                }
            } else if (cmd instanceof AsyncGetAllRawContactsReq) {
                mContactsProxy.asyncGetAllRawContacts(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        AsyncGetAllRawContactsRsp rsp = new AsyncGetAllRawContactsRsp(
                                reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        // Send response command or append it to batch
                        onRespond(rsp);
                    }

                }, Global.getByteBuffer());
            } else if (cmd instanceof AsyncGetAllContactDataReq) {
                mContactsProxy.asyncGetAllContactData(
                        ((AsyncGetAllContactDataReq) cmd)
                                .getRequestingDataTypes(),
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                AsyncGetAllContactDataRsp rsp = new AsyncGetAllContactDataRsp(
                                        reqToken);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        }, Global.getByteBuffer());
            } else if (cmd instanceof ImportDetailedContactsReq) {
                ImportDetailedContactsReq req = (ImportDetailedContactsReq) cmd;
                mContactsProxy.fastImportDetailedContacts(
                // raw bytes of importing detailed contacts
                        req.getRaw(),
                        // Raw contact consumer
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                ImportDetailedContactsRsp rsp = new ImportDetailedContactsRsp(
                                        reqToken);
                                rsp
                                        .setPhase(ImportDetailedContactsRsp.PHASE_RAW_CONTACT);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        },
                        // Contact data consumer
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                ImportDetailedContactsRsp rsp = new ImportDetailedContactsRsp(
                                        reqToken);
                                rsp
                                        .setPhase(ImportDetailedContactsRsp.PHASE_CONTACT_DATA);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        },
                        // output byte buffer
                        Global.getByteBuffer());
            } else {
                return false;
            }
            return true;
        }

        /**
         * Handle commands of 'Messages' feature.
         * 
         * @param cmd
         *            Command to handle.
         * @return True if command is handled, false if not.
         */
        public boolean handleMessageFeatures(BaseCommand cmd) {
            final int reqToken = cmd.getToken();
            if (cmd instanceof MessageBatchReq) {
                mCommandBatch = new MessageBatchRsp(reqToken);
                MessageBatchReq reqMessageBatch = (MessageBatchReq) cmd;
                int i = 0;
                mResponseState = RSP_ST_APPEND_BATCH;
                // Get data and fill them in response command
                for (BaseCommand innerCmd : reqMessageBatch.getCommands()) {
                    if (++i == reqMessageBatch.getCommands().size()) {
                        // It's the last command in batch, we can send out
                        // the response batch now
                        mResponseState = RSP_ST_SEND_BATCH;
                    }
                    handle(innerCmd);
                }
                mResponseState = RSP_ST_SEND_SINGLE;
            } else if (cmd instanceof AsyncGetPhoneListReq) {
                mMessageProxy.asyncGetPhoneList(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        AsyncGetPhoneListRsp rsp = new AsyncGetPhoneListRsp(
                                reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        // Send response command or append it to batch
                        onRespond(rsp);
                    }

                }, Global.getByteBuffer());
            } else if (cmd instanceof AsyncGetAllSmsReq) {
                mMessageProxy.asyncGetAllSms(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        AsyncGetAllSmsRsp rsp = new AsyncGetAllSmsRsp(reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        // Send response command or append it to batch
                        onRespond(rsp);
                    }
                }, Global.getByteBuffer());
            } else if (cmd instanceof AsyncGetAllMmsReq) {
                mMessageProxy.asyncGetAllMms(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        AsyncGetAllMmsRsp rsp = new AsyncGetAllMmsRsp(reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        // Send response command or append it to batch
                        onRespond(rsp);
                    }
                }, Global.getByteBuffer());
                // get MMS resource
            } else if (cmd instanceof GetMmsResourceReq) {
                final long id = ((GetMmsResourceReq) cmd).getMmsId();
                mMessageProxy.getOneMmsResource(new IRawBlockConsumer() {
                    public void consume(byte[] block, int blockNo, int totalNo) {

                        GetMmsResourceRsp rsp = new GetMmsResourceRsp(reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        rsp.setMmsId(id);

                        onRespond(rsp);
                    }
                }, Global.getByteBuffer(), id);
            } else if (cmd instanceof GetMmsDataReq) {
                final boolean isBackup = ((GetMmsDataReq) cmd).getIsBackup();
                LinkedList<Long> list = ((GetMmsDataReq) cmd).getImportList();
                mMessageProxy.getMmsData(new IRawBlockConsumer() {
                    public void consume(byte[] block, int blockNo, int totalNo) {

                        GetMmsDataRsp rsp = new GetMmsDataRsp(reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        rsp.setIsBackup(isBackup);
                        onRespond(rsp);
                    }
                }, Global.getByteBuffer(), isBackup, list);
            } else if (cmd instanceof ImportSmsReq) {
                ImportSmsRsp rsp = new ImportSmsRsp(reqToken);
                // Get data and fill them in response command
                ImportSmsReq req = (ImportSmsReq) cmd;
                ArrayList<Long> temp = new ArrayList<Long>();
                mMulMessageOb.stop();
                long[] insertedIds = mMessageProxy
                        .importSms(req.getRaw(), temp);
                if (null == insertedIds) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    if (null != temp) {
                        long[] threadIds = new long[temp.size()];
                        for (int i = 0; i < threadIds.length; i++) {
                            threadIds[i] = temp.get(i);
                        }
                        rsp.setThreadIds(threadIds);
                    }
                    rsp.setInsertedIds(insertedIds);
                }
                // Send response command or append it to batch
                onRespond(rsp);
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                mMulMessageOb.start();
            } else if (cmd instanceof ImportMmsReq) {
                ImportMmsRsp rsp = new ImportMmsRsp(reqToken);
                // Get data and fill them in response command
                ImportMmsReq req = (ImportMmsReq) cmd;
                ArrayList<Long> temp = new ArrayList<Long>();
                if (mMulMessageOb.getMaxMmsId() == 0) {
                    /** No mms in db, get mms id thought insert */
                    Debugger.logW(new Object[] { cmd }, ">>pdu table is null");
                    mMulMessageOb.onSelfChangeStart();
                    long maxMmsId = mMessageProxy.getMaxMmsIdByInsert();
                    /** Wait 1s to make sure MulMessageob get the right max mms id */
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    mMulMessageOb.onSelfChangeDone();
                    mMulMessageOb.setMaxMmsId(maxMmsId);
                }
                long[] insertedIds = mMessageProxy.importMms(req.getRaw(), temp);
                if (null == insertedIds) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    if (null != temp) {
                        long[] threadIds = new long[temp.size()];
                        for (int i = 0; i < threadIds.length; i++) {
                            threadIds[i] = temp.get(i);
                        }
                        rsp.setThreadIds(threadIds);
                    }
                    rsp.setInsertedIds(insertedIds);
                }
                // Send response command or append it to batch
                if (req.isIsLastImport()) {
                    onRespond(rsp);
                }
            } else if (cmd instanceof ClearMessageBoxReq) {
                ClearMessageBoxRsp rsp = new ClearMessageBoxRsp(reqToken);
                // Get data and fill them in response command
                ClearMessageBoxReq req = (ClearMessageBoxReq) cmd;
                rsp.setDeletedCount(mMessageProxy.clearMessageBox(req.getBox(),
                        req.isKeepLockedMessage()));
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof DeleteAllMessagesReq) {
                DeleteAllMessagesRsp rsp = new DeleteAllMessagesRsp(reqToken);
                // Get data and fill them in response command
                DeleteAllMessagesReq req = (DeleteAllMessagesReq) cmd;
                rsp.setDeletedCount(mMessageProxy.deleteAllMessages(req
                        .isKeepLockedMessage()));
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof DeleteMessageReq) {
                DeleteMessageRsp rsp = new DeleteMessageRsp(reqToken);
                // Get data and fill them in response command
                DeleteMessageReq req = (DeleteMessageReq) cmd;
                rsp.setDeleteSmsCount(mMessageProxy.deleteSms(req
                        .getDeleteSmsIds(), true, req.getDeleteSmsDates()));
                rsp.setDeleteMmsCount(mMessageProxy.deleteMms(req
                        .getDeleteMmsIds(), true, req.getDeleteMmsDates()));
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof MarkMessageAsReadReq) {
                MarkMessageAsReadRsp rsp = new MarkMessageAsReadRsp(reqToken);
                // Get data and fill them in response command
                MarkMessageAsReadReq req = (MarkMessageAsReadReq) cmd;
                rsp.setUpdateSmsCount(mMessageProxy.markSmsAsRead(req
                        .getUpdateSmsIds(), req.isRead()));
                rsp.setUpdateMmsCount(mMessageProxy.markMmsAsRead(req
                        .getUpdateMmsIds(), req.isRead()));
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof LockMessageReq) {
                LockMessageRsp rsp = new LockMessageRsp(reqToken);
                // Get data and fill them in response command
                LockMessageReq req = (LockMessageReq) cmd;
                rsp.setUpdateSmsCount(mMessageProxy.lockSms(req
                        .getUpdateSmsIds(), req.isLocked()));
                rsp.setUpdateMmsCount(mMessageProxy.lockMms(req
                        .getUpdateMmsIds(), req.isLocked()));
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof SaveSmsDraftReq) {
                SaveSmsDraftRsp rsp = new SaveSmsDraftRsp(reqToken);
                // Get data and fill them in response command
                SaveSmsDraftReq req = (SaveSmsDraftReq) cmd;
                Sms result = mMessageProxy.saveSmsDraft(req.getBody(), req
                        .getRecipients());
                if (null != result) {
                    rsp.setInsertedId(result.getId());
                    rsp.setThreadId(result.getThreadId());
                    rsp.setDate(result.getDate());
                } else {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof SendSmsReq) {
                SendSmsRsp rsp = new SendSmsRsp(reqToken);
                // Get data and fill them in response command
                SendSmsReq req = (SendSmsReq) cmd;
                rsp.setSimId(req.getSimId());
                mSmsSender.pause();
                Sms[] results = mMessageProxy.sendSms(req.getBody(), req
                        .getRecipients(), mSmsSender, req.getSimId());
                if (null == results) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    long[] insertedIds = new long[results.length];
                    long threadId = DatabaseRecordEntity.ID_NULL;
                    long[] dates = new long[results.length];
                    for (int i = 0; i < results.length; i++) {
                        insertedIds[i] = results[i].getId();
                        threadId = results[i].getThreadId();
                        dates[i] = results[i].getDate();
                    }
                    rsp.setInsertedIds(insertedIds);
                    rsp.setThreadId(threadId);
                    rsp.setDates(dates);
                }
                // Send response command or append it to batch
                onRespond(rsp);
                mSmsSender.resume();
            } else if (cmd instanceof MoveMessageToBoxReq) {
                MoveMessageToBoxRsp rsp = new MoveMessageToBoxRsp(reqToken);
                // Get data and fill them in response command
                MoveMessageToBoxReq req = (MoveMessageToBoxReq) cmd;
                int smsCount = mMessageProxy.moveSmsToBox(
                        req.getUpdateSmsIds(), true, req.getUpdateSmsDates(),
                        req.getBox());
                rsp.setUpdateSmsCount(smsCount);
                int mmsCount = mMessageProxy.moveMmsToBox(
                        req.getUpdateSmsIds(), true, req.getUpdateSmsDates(),
                        req.getBox());
                rsp.setUpdateMmsCount(mmsCount);
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof ResendSmsReq) {
                ResendSmsRsp rsp = new ResendSmsRsp(reqToken);
                // Get data and fill them in response command
                ResendSmsReq req = (ResendSmsReq) cmd;
                rsp.setSimId(req.getSimId());
                mSmsSender.pause();
                Sms result = mMessageProxy.resendSms(req.getId(),
                        req.getDate(), req.getBody(), req.getRecipient(),
                        mSmsSender, req.getSimId());
                if (null == result) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                    rsp.setErrorMessage(ResendSmsRsp.ERR_SMS_NOT_EXIST);
                } else {
                    rsp.setDate(result.getDate());
                }
                // Send response command or append it to batch
                onRespond(rsp);
                mSmsSender.resume();
            } else if (cmd instanceof BeforeImportMmsReq) {
                BeforeImportMmsRsp rsp = new BeforeImportMmsRsp(reqToken);

                // Send response command or append it to batch
                onRespond(rsp);
            } else {
                return false;
            }
            return true;
        }

        /**
         * Handle commands of 'Application' feature.
         * 
         * @param cmd
         *            Command to handle.
         * @return True if command is handled, false if not.
         */
        public boolean handleApplicationFeatures(BaseCommand cmd) {
            final int reqToken = cmd.getToken();
            if (cmd instanceof AsyncGetAllAppInfoReq) {
                mApplicationProxy.fastGetAllApplications(
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                AsyncGetAllAppInfoRsp rsp = new AsyncGetAllAppInfoRsp(
                                        reqToken);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        }, Global.getByteBuffer(),
                        ((AsyncGetAllAppInfoReq) cmd).getDestIconWidth(),
                        ((AsyncGetAllAppInfoReq) cmd).getDestIconHeight());
            } else {
                return false;
            }
            return true;
        }

        /**
         * Handle commands of 'Outlook Sync' feature.
         * 
         * @param cmd
         *            Command to handle.
         * @return True if command is handled, false if not.
         */
        public boolean handleSyncFeatures(BaseCommand cmd) {
            final int reqToken = cmd.getToken();
            if (cmd instanceof ContactsSyncStartReq) {
                ContactsSyncStartRsp rsp = new ContactsSyncStartRsp(reqToken);
                // Get data and fill them in response command
                rsp.setSyncNeedReinit(mContactsProxy.isSyncNeedReinit());
                rsp.setLastSyncDate(mContactsProxy.getLastSyncDate());
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof ContactsSyncOverReq) {
                ContactsSyncOverRsp rsp = new ContactsSyncOverRsp(reqToken);
                // Get data and fill them in response command
                ContactsSyncOverReq req = (ContactsSyncOverReq) cmd;
                mContactsProxy.updateSyncDate(req.getSyncDate());
                rsp
                        .setContactsCount(mContactsProxy
                                .getAvailableContactsCount());
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof ContactsSlowSyncInitReq) {
                ContactsSlowSyncInitRsp rsp = new ContactsSlowSyncInitRsp(
                        reqToken);
                // Get data and fill them in response command
                rsp.setCurrentMaxId(mContactsProxy.getMaxRawContactsId());
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof ContactsSlowSyncAddDetailedContactsReq) {
                ContactsSlowSyncAddDetailedContactsRsp rsp = new ContactsSlowSyncAddDetailedContactsRsp(
                        reqToken);
                // Get data and fill them in response command
                // Insert new raw contacts and get their sync flags after
                byte[] detailedContactsInRaw = ((ContactsSlowSyncAddDetailedContactsReq) cmd)
                        .getRaw();
                byte[] results = mContactsProxy
                        .slowSyncAddDetailedContacts(detailedContactsInRaw);
                if (null == results) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    rsp.setRaw(results);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof ContactsSlowSyncGetAllRawContactsReq) {
                long contactIdLimit = ((ContactsSlowSyncGetAllRawContactsReq) cmd)
                        .getContactIdLimit();
                mContactsProxy.slowSyncGetAllRawContacts(contactIdLimit,
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                ContactsSlowSyncGetAllRawContactsRsp rsp = new ContactsSlowSyncGetAllRawContactsRsp(
                                        reqToken);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        }, Global.getByteBuffer());
            } else if (cmd instanceof ContactsSlowSyncGetAllContactDataReq) {
                long contactIdLimit = ((ContactsSlowSyncGetAllContactDataReq) cmd)
                        .getContactIdLimit();
                mContactsProxy.slowSyncGetAllContactData(contactIdLimit,
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                ContactsSlowSyncGetAllContactDataRsp rsp = new ContactsSlowSyncGetAllContactDataRsp(
                                        reqToken);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        }, Global.getByteBuffer());
            } else if (cmd instanceof ContactsFastSyncInitReq) {
                mContactsProxy.fastSyncGetAllSyncFlags(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        ContactsFastSyncInitRsp rsp = new ContactsFastSyncInitRsp(
                                reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        // Send response command or append it to batch
                        onRespond(rsp);
                    }

                }, Global.getByteBuffer());
            } else if (cmd instanceof ContactsFastSyncGetRawContactsReq) {
                long[] requestedContactsIds = ((ContactsFastSyncGetRawContactsReq) cmd)
                        .getRequestedContactIds();
                mContactsProxy.fastSyncGetRawContacts(requestedContactsIds,
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                ContactsFastSyncGetRawContactsRsp rsp = new ContactsFastSyncGetRawContactsRsp(
                                        reqToken);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        }, Global.getByteBuffer());
            } else if (cmd instanceof ContactsFastSyncGetContactDataReq) {
                long[] requestedContactsIds = ((ContactsFastSyncGetContactDataReq) cmd)
                        .getRequestedContactIds();
                mContactsProxy.fastSyncGetContactData(requestedContactsIds,
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                ContactsFastSyncGetContactDataRsp rsp = new ContactsFastSyncGetContactDataRsp(
                                        reqToken);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        }, Global.getByteBuffer());
            } else if (cmd instanceof ContactsFastSyncAddDetailedContactsReq) {
                ContactsFastSyncAddDetailedContactsRsp rsp = new ContactsFastSyncAddDetailedContactsRsp(
                        reqToken);
                // Get data and fill them in response command
                // Insert new raw contacts and get their sync flags after
                byte[] detailedContactsInRaw = ((ContactsFastSyncAddDetailedContactsReq) cmd)
                        .getRaw();
                byte[] results = mContactsProxy
                        .fastSyncAddDetailedContacts(detailedContactsInRaw);
                if (null == results) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    rsp.setRaw(results);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof ContactsFastSyncUpdateDetailedContactsReq) {
                ContactsFastSyncUpdateDetailedContactsRsp rsp = new ContactsFastSyncUpdateDetailedContactsRsp(
                        reqToken);
                // Get data and fill them in response command
                // Insert new raw contacts and get their sync flags after
                byte[] detailedContactsInRaw = ((ContactsFastSyncUpdateDetailedContactsReq) cmd)
                        .getRaw();
                byte[] results = mContactsProxy
                        .fastSyncUpdateDetailedContacts(detailedContactsInRaw);
                if (null == results) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    rsp.setRaw(results);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof ContactsFastSyncDeleteContactsReq) {
                ContactsFastSyncDeleteContactsRsp rsp = new ContactsFastSyncDeleteContactsRsp(
                        reqToken);
                // Get data and fill them in response command
                ContactsFastSyncDeleteContactsReq req = (ContactsFastSyncDeleteContactsReq) cmd;
                rsp.setDeleteCount(mContactsProxy
                        .fastDeleteContactsSourcedOnPhone(req.getDeleteIds(),
                                false));
                // Send response command or append it to batch
                onRespond(rsp);
            } else {
                return false;
            }
            return true;
        }

        /**
         * Handle commands of 'Calendar' feature.
         * 
         * @param cmd
         *            Command to handle.
         * @return True if command is handled, false if not.
         */
        public boolean handleCalendarFeatures(BaseCommand cmd) {
            final int reqToken = cmd.getToken();
            if (cmd instanceof CalendarBatchReq) {
                mCommandBatch = new CalendarBatchRsp(reqToken);
                CalendarBatchReq reqCalendarBatch = (CalendarBatchReq) cmd;
                int i = 0;
                mResponseState = RSP_ST_APPEND_BATCH;
                // Get data and fill them in response command
                for (BaseCommand innerCmd : reqCalendarBatch.getCommands()) {
                    if (++i == reqCalendarBatch.getCommands().size()) {
                        // It's the last command in batch, we can send out
                        // the response batch now
                        mResponseState = RSP_ST_SEND_BATCH;
                    }
                    handle(innerCmd);
                }
                mResponseState = RSP_ST_SEND_SINGLE;
            }
            if (cmd instanceof AddEventReq) {
                AddEventRsp rsp = new AddEventRsp(reqToken);
                AddEventReq req = (AddEventReq) cmd;

                rsp.setFromFeature(req.getFromFeature());
                CalendarEvent event = req.getEvent();
                long insertedEventId = mCalendarProxy.insertEvent(event);
                rsp.setInsertedId(insertedEventId);

                if (insertedEventId == DatabaseRecordEntity.ID_NULL) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    rsp.setResult(mCalendarProxy.getEvent(insertedEventId,
                            true, true));
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof DeleteEventReq) {
                DeleteEventRsp rsp = new DeleteEventRsp(reqToken);
                // Get data and fill them in response command
                DeleteEventReq req = (DeleteEventReq) cmd;
                rsp.setDeleteResults(mCalendarProxy.deleteEvents(req
                        .getDeleteIds()));
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof GetCalendarsReq) {
                mCalendarProxy.getCalendars(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        GetCalendarsRsp rsp = new GetCalendarsRsp(reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        // Send response command or append it to batch
                        onRespond(rsp);
                    }

                }, Global.getByteBuffer());
            } else if (cmd instanceof GetEventsReq) {
                mCalendarProxy.getEvents(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        GetEventsRsp rsp = new GetEventsRsp(reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        // Send response command or append it to batch
                        onRespond(rsp);
                    }

                }, Global.getByteBuffer());
            } else if (cmd instanceof UpdateEventReq) {

                UpdateEventRsp rsp = new UpdateEventRsp(reqToken);
                // Get data and fill them in response command
                UpdateEventReq req = (UpdateEventReq) cmd;
                CalendarEvent event = req.getNewOne();
                int updateGroupCount = mCalendarProxy.updateEvent(req
                        .getUpdateId(), event);
                if (updateGroupCount < 1) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                }
                // Send response command or append it to batch
                onRespond(rsp);

            } else if (cmd instanceof GetRemindersReq) {
                mCalendarProxy.getReminders(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        GetRemindersRsp rsp = new GetRemindersRsp(reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        // Send response command or append it to batch
                        onRespond(rsp);
                    }

                }, Global.getByteBuffer());
            } else if (cmd instanceof GetAttendeesReq) {
                mCalendarProxy.getAttendees(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        GetAttendeesRsp rsp = new GetAttendeesRsp(reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        // Send response command or append it to batch
                        onRespond(rsp);
                    }

                }, Global.getByteBuffer());
            } else {
                return false;
            }
            return true;
        }

        /**
         * Handle commands of 'Outlook Calendar Sync' feature.
         * 
         * @param cmd
         *            Command to handle.
         * @return True if command is handled, false if not.
         */
        public boolean handleCalendarSyncFeatures(BaseCommand cmd) {
            final int reqToken = cmd.getToken();
            if (cmd instanceof CalendarSyncStartReq) {
                CalendarSyncStartRsp rsp = new CalendarSyncStartRsp(reqToken);
                // Get data and fill them in response command
                rsp.setSyncNeedReinit(mCalendarProxy.isSyncNeedReinit());
                rsp.setLastSyncDate(mCalendarProxy.getLastSyncDate());
                rsp.setLocalAccountId(mCalendarProxy.getLocalAccountId());
                rsp.setSyncAble(mCalendarProxy.isSyncAble());
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof CalendarSyncOverReq) {
                CalendarSyncOverRsp rsp = new CalendarSyncOverRsp(reqToken);
                // Get data and fill them in response command
                CalendarSyncOverReq req = (CalendarSyncOverReq) cmd;
                mCalendarProxy.updateSyncDate(req.getSyncDate());
                rsp.setEventsCount(mCalendarProxy.getPcSyncEventsCount());
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof CalendarSlowSyncInitReq) {
                CalendarSlowSyncInitRsp rsp = new CalendarSlowSyncInitRsp(
                        reqToken);
                // Get data and fill them in response command
                rsp.setCurrentMaxId(mCalendarProxy.getMaxEventId());
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof CalendarSlowSyncGetAllEventsReq) {
                long eventIdLimit = ((CalendarSlowSyncGetAllEventsReq) cmd)
                        .getEventIdLimit();
                mCalendarProxy.slowSyncGetAllEvents(eventIdLimit,
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                CalendarSlowSyncGetAllEventsRsp rsp = new CalendarSlowSyncGetAllEventsRsp(
                                        reqToken);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        }, Global.getByteBuffer());
            } else if (cmd instanceof CalendarSlowSyncGetAllRemindersReq) {
                long eventIdLimit = ((CalendarSlowSyncGetAllRemindersReq) cmd)
                        .getEventIdLimit();
                mCalendarProxy.slowSyncGetAllReminders(eventIdLimit,
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                CalendarSlowSyncGetAllRemindersRsp rsp = new CalendarSlowSyncGetAllRemindersRsp(
                                        reqToken);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        }, Global.getByteBuffer());
            } else if (cmd instanceof CalendarSlowSyncGetAllAttendeesReq) {
                long eventIdLimit = ((CalendarSlowSyncGetAllAttendeesReq) cmd)
                        .getEventIdLimit();
                mCalendarProxy.slowSyncGetAllAttendees(eventIdLimit,
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                CalendarSlowSyncGetAllAttendeesRsp rsp = new CalendarSlowSyncGetAllAttendeesRsp(
                                        reqToken);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        }, Global.getByteBuffer());
            } else if (cmd instanceof CalendarSlowSyncAddEventsReq) {
                CalendarSlowSyncAddEventsRsp rsp = new CalendarSlowSyncAddEventsRsp(
                        reqToken);
                // Get data and fill them in response command
                // Insert new raw contacts and get their sync flags after
                byte[] detailedEventsInRaw = ((CalendarSlowSyncAddEventsReq) cmd)
                        .getRaw();
                byte[] results = mCalendarProxy
                        .slowSyncAddEvents(detailedEventsInRaw);
                if (null == results) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    rsp.setRaw(results);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof CalendarFastSyncInitReq) {
                mCalendarProxy.fastSyncGetAllSyncFlags(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        CalendarFastSyncInitRsp rsp = new CalendarFastSyncInitRsp(
                                reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        // Send response command or append it to batch
                        onRespond(rsp);
                    }

                }, Global.getByteBuffer());
            } else if (cmd instanceof CalendarFastSyncGetEventsReq) {
                long[] requestedEventsIds = ((CalendarFastSyncGetEventsReq) cmd)
                        .getRequestedEventIds();
                mCalendarProxy.fastSyncGetEvents(requestedEventsIds,
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                CalendarFastSyncGetEventsRsp rsp = new CalendarFastSyncGetEventsRsp(
                                        reqToken);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        }, Global.getByteBuffer());
            } else if (cmd instanceof CalendarFastSyncGetRemindersReq) {
                long[] requestedEventsIds = ((CalendarFastSyncGetRemindersReq) cmd)
                        .getRequestedEventIds();
                mCalendarProxy.fastSyncGetReminders(requestedEventsIds,
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                CalendarFastSyncGetRemindersRsp rsp = new CalendarFastSyncGetRemindersRsp(
                                        reqToken);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        }, Global.getByteBuffer());
            } else if (cmd instanceof CalendarFastSyncGetAttendeesReq) {
                long[] requestedEventsIds = ((CalendarFastSyncGetAttendeesReq) cmd)
                        .getRequestedEventIds();
                mCalendarProxy.fastSyncGetAttendees(requestedEventsIds,
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                CalendarFastSyncGetAttendeesRsp rsp = new CalendarFastSyncGetAttendeesRsp(
                                        reqToken);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        }, Global.getByteBuffer());
            } else if (cmd instanceof CalendarFastSyncAddEventsReq) {
                CalendarFastSyncAddEventsRsp rsp = new CalendarFastSyncAddEventsRsp(
                        reqToken);
                // Get data and fill them in response command
                // Insert new events and get their sync flags after
                byte[] eventsInRaw = ((CalendarFastSyncAddEventsReq) cmd)
                        .getRaw();
                byte[] results = mCalendarProxy.fastSyncAddEvents(eventsInRaw);
                if (null == results) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    rsp.setRaw(results);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof CalendarFastSyncDeleteEventsReq) {
                CalendarFastSyncDeleteEventsRsp rsp = new CalendarFastSyncDeleteEventsRsp(
                        reqToken);
                // Get data and fill them in response command
                CalendarFastSyncDeleteEventsReq req = (CalendarFastSyncDeleteEventsReq) cmd;
                rsp.setDeleteCount(mCalendarProxy.fastDeleteEvents(req
                        .getDeleteIds()));
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof CalendarFastSyncUpdateEventsReq) {
                CalendarFastSyncUpdateEventsRsp rsp = new CalendarFastSyncUpdateEventsRsp(
                        reqToken);
                // Get data and fill them in response command
                // Insert new events and get their sync flags after
                byte[] eventsInRaw = ((CalendarFastSyncUpdateEventsReq) cmd)
                        .getRaw();
                byte[] results = mCalendarProxy
                        .fastSyncUpdateEvents(eventsInRaw);
                if (null == results) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    rsp.setRaw(results);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else {
                return false;
            }
            return true;
        }

        /**
         * Handle commands of 'Media Sync' feature.
         * 
         * @param cmd
         *            Command to handle.
         * @return True if command is handled, false if not.
         */
        public boolean handleMediaFeatures(BaseCommand cmd) {
            final int reqToken = cmd.getToken();
            if (cmd instanceof GetContentDirectoriesReq) {
                GetContentDirectoriesRsp rsp = new GetContentDirectoriesRsp(
                        reqToken);
                // Get data and fill them in response command
                MediaInfo[] result = mMediaProxy.getContentDirectories();
                if (null == result) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    rsp.setDirectories(result);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof GetAllMediaFilesReq) {
                GetAllMediaFilesRsp rsp = new GetAllMediaFilesRsp(reqToken);
                // Get data and fill them in response command
                GetAllMediaFilesReq req = (GetAllMediaFilesReq) cmd;
                MediaInfo[] result = mMediaProxy.getFiles(req
                        .getRequestedContentTypes());
                if (null == result) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    rsp.setFiles(result);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof RenameFilesReq) {
                RenameFilesRsp rsp = new RenameFilesRsp(reqToken);
                // Get data and fill them in response command
                RenameFilesReq req = (RenameFilesReq) cmd;
                boolean[] result = mMediaProxy.renameFiles(req.getOldPaths(),
                        req.getNewPaths());
                if (null == result) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    rsp.setResults(result);
                }
                // Send response command or append it to batch
                onRespond(rsp);
            } else if (cmd instanceof MediaSyncOverReq) {
                mMediaProxy.scan(MainService.this);
                MediaSyncOverRsp rsp = new MediaSyncOverRsp(reqToken);
                onRespond(rsp);
            } else {
                return false;
            }
            return true;
        }

        /**
         * Handle commands of 'Bookmark Sync' feature.
         * 
         * @param cmd
         *            Command to handle.
         * @return True if command is handled, false if not.
         * 
         *         add by mtk54043 Yu.Chen
         */
        public boolean handleBookmarkFeatures(BaseCommand cmd) {
            final int reqToken = cmd.getToken();

            if (cmd instanceof AsyncGetAllBookmarkInfoReq) {
                AsyncGetAllBookmarkInfoRsp rspCmd = new AsyncGetAllBookmarkInfoRsp(
                        reqToken);
                ArrayList<BookmarkData> mBookmarkDataList = new ArrayList<BookmarkData>();
                ArrayList<BookmarkFolder> mBookmarkFolderList = new ArrayList<BookmarkFolder>();
                mBookmarkProxy.asynGetAllBookmarks(mBookmarkDataList,
                        mBookmarkFolderList);
                rspCmd.setmBookmarkDataList(mBookmarkDataList);
                rspCmd.setmBookmarkFolderList(mBookmarkFolderList);
                onRespond(rspCmd);
            } else if (cmd instanceof AsyncInsertBookmarkReq) {
                ArrayList<BookmarkData> mBookmarkDataList = ((AsyncInsertBookmarkReq) cmd)
                        .getBookmarkDataList();
                ArrayList<BookmarkFolder> mBookmarkFolderList = ((AsyncInsertBookmarkReq) cmd)
                        .getBookmarkFolderList();
                AsyncInsertBookmarkRsp rspCmd = new AsyncInsertBookmarkRsp(
                        reqToken);
                // mBookmarkProxy.handleHistoryData(mBookmarkDataList);
                mBookmarkProxy.insertBookmark(mBookmarkDataList,
                        mBookmarkFolderList);
                // mBookmarkProxy.sendBroadcastToBrowser(MainService.this);

                // MainService.this.sendBroadcast(new
                // Intent("com.mediatek.apst.target.data.proxy.bookmark.sync"));
                // Debugger.logI(new
                // Object[]{cmd},"InsertBookmark_____SendBroadcastToBrowser");
                onRespond(rspCmd);
            } else if (cmd instanceof AsyncDeleteBookmarkReq) {
                AsyncDeleteBookmarkRsp rspCmd = new AsyncDeleteBookmarkRsp(
                        reqToken);
                mBookmarkProxy.deleteAll();
                // mBookmarkProxy.sendBroadcastToBrowser(MainService.this);
                // MainService.this.sendBroadcast(new
                // Intent("com.mediatek.apst.target.data.proxy.bookmark.sync"));
                // Debugger.logI(new
                // Object[]{cmd},"DeleteBookmark_____SendBroadcastToBrowser");
                onRespond(rspCmd);
            } else {
                return false;
            }
            return true;
        }

        /**
         * Handle commands of 'Backup/Retore' feature.
         * 
         * @param cmd
         *            Command to handle.
         * @return True if command is handled, false if not.
         */
        public boolean handleBackupFeatures(BaseCommand cmd) {
            final int reqToken = cmd.getToken();
            if (cmd instanceof GetAllGroupsForBackupReq) {
                mContactsProxy.asyncGetAllGroups(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        GetAllGroupsForBackupRsp rsp = new GetAllGroupsForBackupRsp(
                                reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        // Send response command or append it to batch
                        onRespond(rsp);
                    }

                }, Global.getByteBuffer());

            } else if (cmd instanceof GetAllContsForBackupReq) {
                mContactsProxy.asyncGetAllRawContactsForBackup(
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                GetAllContsForBackupRsp rsp = new GetAllContsForBackupRsp(
                                        reqToken);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        }, Global.getByteBuffer(), null, null);

            } else if (cmd instanceof GetAllContsDataForBackupReq) {
                mContactsProxy.asyncGetAllContactData(
                        ((GetAllContsDataForBackupReq) cmd)
                                .getRequestingDataTypes(),
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                GetAllContsDataForBackupRsp rsp = new GetAllContsDataForBackupRsp(
                                        reqToken);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        }, Global.getByteBuffer());

            } else if (cmd instanceof GetPhoneListReq) {
                mMessageProxy.asyncGetPhoneList(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        GetPhoneListRsp rsp = new GetPhoneListRsp(reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        // Send response command or append it to batch
                        onRespond(rsp);
                    }

                }, Global.getByteBuffer());

            } else if (cmd instanceof DelAllContactsReq) {
                DelAllContactsRsp rsp = new DelAllContactsRsp(reqToken);
                // Delete all groups
                mContactsProxy.deleteAllGroups();
                // Get data and fill them in response command
                rsp.setDeleteCount(mContactsProxy.deleteContactForBackup());
                // Send response command or append it to batch
                onRespond(rsp);

            } else if (cmd instanceof RestoreGroupReq) {
                RestoreGroupReq req = (RestoreGroupReq) cmd;
                RestoreGroupRsp rsp = new RestoreGroupRsp(reqToken);
                rsp.setCount(mContactsProxy.updateGroupForRestore(req
                        .getGroupList()));
                onRespond(rsp);

            } else if (cmd instanceof RestoreContactsReq) {
                RestoreContactsReq req = (RestoreContactsReq) cmd;
                mContactsProxy.restoreDetailedContacts(
                // raw bytes of importing detailed contacts
                        req.getRaw(),
                        // Raw contact consumer
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                RestoreContactsRsp rsp = new RestoreContactsRsp(
                                        reqToken);
                                rsp
                                        .setPhase(RestoreContactsRsp.PHASE_RAW_CONTACT);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        },
                        // Contact data consumer
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                RestoreContactsRsp rsp = new RestoreContactsRsp(
                                        reqToken);
                                rsp
                                        .setPhase(RestoreContactsRsp.PHASE_CONTACT_DATA);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        },
                        // output byte buffer
                        Global.getByteBuffer());

            } else if (cmd instanceof GetAllSmsForBackupReq) {
                mMessageProxy.asyncGetAllSms(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        GetAllSmsForBackupRsp rsp = new GetAllSmsForBackupRsp(
                                reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        onRespond(rsp);
                    }
                }, Global.getByteBuffer());

            } else if (cmd instanceof GetMmsDataForBackupReq) {

                mMessageProxy.getMmsData(new IRawBlockConsumer() {
                    public void consume(byte[] block, int blockNo, int totalNo) {

                        GetMmsDataForBackupRsp rsp = new GetMmsDataForBackupRsp(
                                reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        onRespond(rsp);
                    }
                }, Global.getByteBuffer(), true, null);

            } else if (cmd instanceof DelAllMsgsForBackupReq) {
                DelAllMsgsForBackupRsp rsp = new DelAllMsgsForBackupRsp(
                        reqToken);
                // Get data and fill them in response command
                rsp.setDeletedCount(mMessageProxy.deleteAllMessages(false));
                // Send response command or append it to batch
                onRespond(rsp);

            } else if (cmd instanceof RestoreSmsReq) {
                RestoreSmsRsp rsp = new RestoreSmsRsp(reqToken);
                // Get data and fill them in response command
                RestoreSmsReq req = (RestoreSmsReq) cmd;
                ArrayList<Long> temp = new ArrayList<Long>();
                mMulMessageOb.stop();
                long[] insertedIds = mMessageProxy
                        .importSms(req.getRaw(), temp);
                mMulMessageOb.start();
                if (null == insertedIds) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    if (null != temp) {
                        long[] threadIds = new long[temp.size()];
                        for (int i = 0; i < threadIds.length; i++) {
                            threadIds[i] = temp.get(i);
                        }
                        rsp.setThreadIds(threadIds);
                    }
                    rsp.setInsertedIds(insertedIds);
                }
                // Send response command or append it to batch
                onRespond(rsp);

            } else if (cmd instanceof RestoreMmsReq) {
                RestoreMmsRsp rsp = new RestoreMmsRsp(reqToken);
                // Get data and fill them in response command
                RestoreMmsReq req = (RestoreMmsReq) cmd;
                ArrayList<Long> temp = new ArrayList<Long>();
                mMulMessageOb.onSelfChangeStart();
                if (mMulMessageOb.getMaxMmsId() == 0) {
                    /** No mms in db, get mms id thought insert */
                    Debugger.logW(new Object[] { cmd }, ">>restore start ,pdu table is null");
                    long maxMmsId = mMessageProxy.getMaxMmsIdByInsert();
                    /** Wait 1s to make sure MulMessageob get the right max mms id */
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    mMulMessageOb.setMaxMmsId(maxMmsId);
                }
                long[] insertedIds = mMessageProxy.importMms(req.getRaw(), temp);
                mMulMessageOb.onSelfChangeDone();
                if (null == insertedIds) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    if (null != temp) {
                        long[] threadIds = new long[temp.size()];
                        for (int i = 0; i < threadIds.length; i++) {
                            threadIds[i] = temp.get(i);
                        }
                        rsp.setThreadIds(threadIds);
                    }
                    rsp.setInsertedIds(insertedIds);
                }
                // Send response command or append it to batch
                if (req.isIsLastImport()) {
                    onRespond(rsp);
                }
            } else if (cmd instanceof GetEventsForBackupReq) {
                mCalendarProxy.getEvents(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        GetEventsForBackupRsp rsp = new GetEventsForBackupRsp(
                                reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        // Send response command or append it to batch
                        onRespond(rsp);
                    }

                }, Global.getByteBuffer());

            } else if (cmd instanceof GetAttendeesForBackupReq) {
                mCalendarProxy.getAttendees(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        GetAttendeesForBackupRsp rsp = new GetAttendeesForBackupRsp(
                                reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        // Send response command or append it to batch
                        onRespond(rsp);
                    }

                }, Global.getByteBuffer());

            } else if (cmd instanceof GetRemindersForBackupReq) {
                mCalendarProxy.getReminders(new IRawBlockConsumer() {

                    // @Override
                    public void consume(byte[] block, int blockNo, int totalNo) {
                        GetRemindersForBackupRsp rsp = new GetRemindersForBackupRsp(
                                reqToken);
                        rsp.setRaw(block);
                        rsp.setProgress(blockNo);
                        rsp.setTotal(totalNo);
                        // Send response command or append it to batch
                        onRespond(rsp);
                    }

                }, Global.getByteBuffer());

            } else if (cmd instanceof DelAllCalendarReq) {
                DelAllCalendarRsp rsp = new DelAllCalendarRsp(reqToken);
                // Get data and fill them in response command
                mCalendarProxy.deleteAll();
                // Send response command or append it to batch
                onRespond(rsp);

            } else if (cmd instanceof RestoreCalendarReq) {

                RestoreCalendarReq req = (RestoreCalendarReq) cmd;
                RestoreCalendarRsp rsp = new RestoreCalendarRsp(reqToken);
                // Get data and fill them in response command
                rsp.setInsertedCount(mCalendarProxy.insertAllEvents(req
                        .getEvent()));
                // Send response command or append it to batch
                onRespond(rsp);

            } else if (cmd instanceof GetAllBookmarkForBackupReq) {
                GetAllBookmarkForBackupRsp rspCmd = new GetAllBookmarkForBackupRsp(
                        reqToken);
                ArrayList<BookmarkData> mBookmarkDataList = new ArrayList<BookmarkData>();
                ArrayList<BookmarkFolder> mBookmarkFolderList = new ArrayList<BookmarkFolder>();
                mBookmarkProxy.asynGetAllBookmarks(mBookmarkDataList,
                        mBookmarkFolderList);
                rspCmd.setmBookmarkDataList(mBookmarkDataList);
                rspCmd.setmBookmarkFolderList(mBookmarkFolderList);
                onRespond(rspCmd);

            } else if (cmd instanceof DelAllBookmarkReq) {
                DelAllBookmarkRsp rspCmd = new DelAllBookmarkRsp(reqToken);
                mBookmarkProxy.deleteAll();
                // mBookmarkProxy.sendBroadcastToBrowser(MainService.this);
                // MainService.this.sendBroadcast(new
                // Intent("com.mediatek.apst.target.data.proxy.bookmark.sync"));
                // Debugger.logI(new Object[] { cmd },
                // "DeleteBookmark_____SendBroadcastToBrowser");
                onRespond(rspCmd);

            } else if (cmd instanceof RestoreBookmarkReq) {
                ArrayList<BookmarkData> mBookmarkDataList = ((RestoreBookmarkReq) cmd)
                        .getmBookmarkDataList();
                ArrayList<BookmarkFolder> mBookmarkFolderList = ((RestoreBookmarkReq) cmd)
                        .getmBookmarkFolderList();
                RestoreBookmarkRsp rspCmd = new RestoreBookmarkRsp(reqToken);
                // mBookmarkProxy.handleHistoryData(mBookmarkDataList);
                mBookmarkProxy.insertBookmark(mBookmarkDataList,
                        mBookmarkFolderList);
                // mBookmarkProxy.sendBroadcastToBrowser(MainService.this);

                // MainService.this.sendBroadcast(new
                // Intent("com.mediatek.apst.target.data.proxy.bookmark.sync"));
                // Debugger.logI(new Object[] { cmd },
                // "InsertBookmark_____SendBroadcastToBrowser");
                onRespond(rspCmd);

            } else if (cmd instanceof MediaGetStorageStateReq) {

                MediaGetStorageStateRsp rsp = new MediaGetStorageStateRsp(
                        reqToken);
                boolean[] state = mSysInfoProxy.checkSDCardState();
                Debugger.logI("SDCard1 state :" + state[0]);
                Debugger.logI("SDCard2 state :" + state[1]);
                rsp.setStorageState(state);
                rsp.setmSdSwap(SystemInfoProxy.isSdSwap());
                rsp.setmInternalStoragePath(SystemInfoProxy.getInternalStoragePathSD());
                rsp.setmExternalStoragePath(SystemInfoProxy.getExternalStoragePath());
                onRespond(rsp);

            } else if (cmd instanceof MediaBackupReq) {

                MediaBackupReq req = (MediaBackupReq) cmd;
                MediaBackupRsp rsp = new MediaBackupRsp(reqToken);
                ArrayList<String> oldPathsArray = new ArrayList<String>();
                ArrayList<String> newPathsArray = new ArrayList<String>();

                mMediaProxy.getFilesUnderDirs(req.getBackupPaths(),
                        oldPathsArray, newPathsArray);
                Debugger.logI("oldPaths count: " + oldPathsArray.size());
                Debugger.logI("newPaths count: " + newPathsArray.size());
                String[] oldPaths = new String[oldPathsArray.size()];
                String[] newPaths = new String[newPathsArray.size()];
                for (int i = 0; i < oldPathsArray.size(); i++) {
                    oldPaths[i] = oldPathsArray.get(i);
                    newPaths[i] = newPathsArray.get(i) + "/APST_BACKUP0" + i;
                    Debugger.logI("oldPaths[" + i + "]: " + oldPaths[i]);
                    Debugger.logI("newPaths[" + i + "]: " + newPaths[i]);
                }
                rsp.setOldPaths(oldPaths);
                rsp.setNewPaths(newPaths);

                boolean[] resultRe = mMediaProxy.renameFiles(rsp.getOldPaths(),
                        rsp.getNewPaths());
                if (null == resultRe) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    Debugger.logI("resultRe is not null");
                }

                // ArrayList<String> oldPathsArrayTest = new
                // ArrayList<String>();
                // ArrayList<String> newPathsArrayTest = new
                // ArrayList<String>();
                // mMediaProxy.getFilesEndWith("/mnt/sdcard", ".jpg",
                // oldPathsArrayTest, newPathsArrayTest);
                // String[] oldPaths = new String[oldPathsArrayTest.size()];
                // String[] newPaths = new String[newPathsArrayTest.size()];
                // for (int i = 0; i < oldPathsArrayTest.size(); i++) {
                // oldPaths[i] = oldPathsArrayTest.get(i);
                // newPaths[i] = newPathsArrayTest.get(i) + "/APST_BACKUP0" + i;
                // Debugger.logI("oldPaths[" + i + "]: " + oldPaths[i]);
                // Debugger.logI("newPaths[" + i + "]: " + newPaths[i]);
                // }
                // rsp.setOldPaths(oldPaths);
                // rsp.setNewPaths(newPaths);
                //                                 
                // boolean[] resultRe = mMediaProxy.renameFiles(rsp
                // .getOldPaths(), rsp.getNewPaths());
                // if (null == resultRe) {
                // rsp.setStatusCode(ResponseCommand.SC_FAILED);
                // } else {
                //
                // }

                // Send response command or append it to batch
                onRespond(rsp);

            } else if (cmd instanceof MediaFileRenameReq) {

                MediaFileRenameRsp rsp = new MediaFileRenameRsp(reqToken);
                MediaFileRenameReq req = (MediaFileRenameReq) cmd;
                boolean[] result = mMediaProxy.renameFiles(req.getOldPaths(),
                        req.getNewPaths());
                if (null == result) {
                    rsp.setStatusCode(ResponseCommand.SC_FAILED);
                } else {
                    rsp.setResults(result);
                }
                // Send response command or append it to batch
                onRespond(rsp);

            } else if (cmd instanceof MediaRestoreReq) {

                MediaRestoreRsp rsp = new MediaRestoreRsp(reqToken);
                MediaRestoreReq req = (MediaRestoreReq) cmd;
                String restorePath = req.getRestorePath();
                if (null != restorePath) {
                    mMediaProxy.deleteAllFileUnder(restorePath);
                    mMediaProxy.deleteDirectory(restorePath);
                    mMediaProxy.createDirectory(restorePath);
                }
                // Send response command or append it to batch
                onRespond(rsp);

            } else if (cmd instanceof MediaRestoreOverReq) {

                MediaRestoreOverRsp rsp = new MediaRestoreOverRsp(reqToken);
                MediaRestoreOverReq req = (MediaRestoreOverReq) cmd;
                mMediaProxy.scan(MainService.this, req.getRestoreFilePath());
                // Send response command or append it to batch
                onRespond(rsp);

            } else if (cmd instanceof GetAppForBackupReq) {
                mApplicationProxy.fastGetAllApps2Backup(
                        new IRawBlockConsumer() {

                            // @Override
                            public void consume(byte[] block, int blockNo,
                                    int totalNo) {
                                GetAppForBackupRsp rsp = new GetAppForBackupRsp(
                                        reqToken);
                                rsp.setRaw(block);
                                rsp.setProgress(blockNo);
                                rsp.setTotal(totalNo);
                                // Send response command or append it to batch
                                onRespond(rsp);
                            }

                        }, Global.getByteBuffer(), ((GetAppForBackupReq) cmd)
                                .getDestIconWidth(), ((GetAppForBackupReq) cmd)
                                .getDestIconHeight());
            } else if (cmd instanceof StartBackupReq) {
                notifyStart(getString(R.string.noti_main_service_backup));
            } else if (cmd instanceof EndBackupReq) {
                notifyStart(getString(R.string.noti_main_service_running));
            } else {
                return false;
            }
            return true;
        }
    }

    class IncomingSmsFinder extends NewSmsFinder {

        public IncomingSmsFinder() {
            super();
        }

        @Override
        public String getClassName() {
            return "MainService$IncomingSmsFinder";
        }

        @Override
        public Sms findSms(long date, String address, String body, int box) {
            if (null != mMessageProxy) {
                Sms receivedSms = mMessageProxy.findSms(date, address, body,
                        box);
                if (null != receivedSms) {
                    Debugger.logD(this.getClassName(), "findSms", new Object[] {
                            date, address, body, box }, "id="
                            + receivedSms.getId() + ", date="
                            + receivedSms.getDate());

                    NotifyNewMessageReq req = new NotifyNewMessageReq();
                    req.setToken(mDispatcher.getToken());
                    req.setNewMessage(receivedSms);

                    enqueueSendCommand(req);
                    return receivedSms;
                } else {
                    return null;
                }
            } else {
                Debugger.logE(this.getClassName(), "findSms", new Object[] {
                        date, address, body, box }, "Message proxy is null.");
                return null;
            }
        }

    }

    class MainHandler extends Handler {

        // @Override
        public void handleMessage(Message msg) {
            if (msg == null) {
                Debugger.logW("Message received is null.");
                return;
            }

            switch (msg.what) {
            case MSG_CONNECTED:
                init();
                break;

            case MSG_CHECK_TIMEOUT:
                Debugger.logI("Check timeout for creating connection!");
                if (mConnector.isAlive() && !isConnected()) {
                    try {
                        Debugger.logW("Create connection timeout, "
                                + "try to interrupt connector thread.");
                        mConnector.interrupt();
                    } catch (SecurityException e) {
                        Debugger.logE("Exception occurs when try to "
                                + "interrupt connector thread.", e);
                        e.printStackTrace();
                    }
                    // Create connection failed, destroy service
                    stopSelf();
                }
                break;

            case MSG_SAFE_STOP:
                safeStop();
                break;

            case MSG_FORCE_STOP:
                stopSelf();
                break;

            default:
                Debugger.logW("Unknown message type: " + msg.what);
                break;
            }
        }
    }
}
