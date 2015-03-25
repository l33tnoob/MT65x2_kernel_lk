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

package com.mediatek.apst.target.event;

import java.util.ArrayList;
import java.util.List;

public final class EventDispatcher {
    private static EventDispatcher sInstance = new EventDispatcher();
    private List<IBatteryListener> mBatteryListeners;

    private List<IPackageListener> mPackageListeners;

    private List<ISdStateListener> mSdStateListeners;

    private List<ISimStateListener> mSimStateListeners;

    private List<ISmsListener> mSmsListeners;

    private List<IMmsListener> mMmsListeners;

    private List<IContactsListener> mContactsListeners;

    private List<ICalendarEventListener> mCalendarEventListeners;
    
    private List<IBackupAndRestoreListener> mBackupAndRestoreEventListeners;

    /**
     * Constructor.
     */
    private EventDispatcher() {
        // Currently only one listener(MainService)
        this.mBatteryListeners = new ArrayList<IBatteryListener>(1);
        this.mPackageListeners = new ArrayList<IPackageListener>(1);
        this.mSdStateListeners = new ArrayList<ISdStateListener>(1);
        this.mSimStateListeners = new ArrayList<ISimStateListener>(1);
        this.mSmsListeners = new ArrayList<ISmsListener>(1);
        this.mContactsListeners = new ArrayList<IContactsListener>(1);
        this.mMmsListeners = new ArrayList<IMmsListener>(1);
        this.mCalendarEventListeners = new ArrayList<ICalendarEventListener>(1);
        this.mBackupAndRestoreEventListeners = new ArrayList<IBackupAndRestoreListener>(1);
    }

    /**
     * @param listener
     *            The Battery listener.
     */
    public static synchronized void registerBatteryListener(
            IBatteryListener listener) {
        if (null != listener) {
            sInstance.mBatteryListeners.add(listener);
        }
    }

    /**
     * @param listener
     *            The package listener.
     */
    public static synchronized void registerPackageListener(
            IPackageListener listener) {
        if (null != listener) {
            sInstance.mPackageListeners.add(listener);
        }
    }

    /**
     * @param listener
     *            The sdcard state listener.
     */
    public static synchronized void registerSdStateListener(
            ISdStateListener listener) {
        if (null != listener) {
            sInstance.mSdStateListeners.add(listener);
        }
    }

    /**
     * @param listener
     *            The sim state listener.
     */
    public static synchronized void registerSimStateListener(
            ISimStateListener listener) {
        if (null != listener) {
            sInstance.mSimStateListeners.add(listener);
        }
    }

    /**
     * @param listener
     *            The sms listener.
     */
    public static synchronized void registerSmsListener(ISmsListener listener) {
        if (null != listener) {
            sInstance.mSmsListeners.add(listener);
        }
    }

    /**
     * @param listener
     *            The contact listener.
     */
    public static synchronized void registerContactsListener(
            IContactsListener listener) {
        if (null != listener) {
            sInstance.mContactsListeners.add(listener);
        }
    }

    /**
     * @param listener
     *            The mms listener.
     */
    public static synchronized void registerMmsListener(IMmsListener listener) {
        if (null != listener) {
            sInstance.mMmsListeners.add(listener);
        }
    }

    /**
     * @param listener
     *            The calendar event listener.
     */
    public static synchronized void registerCalendarEventListener(
            ICalendarEventListener listener) {
        if (null != listener) {
            sInstance.mCalendarEventListeners.add(listener);
        }
    }
    
    /**
     * @param listener
     *            The calendar event listener.
     */
    public static synchronized void registerBackupAndRestoreEventListener(
            IBackupAndRestoreListener listener) {
        if (null != listener) {
            sInstance.mBackupAndRestoreEventListeners.add(listener);
        }
    }

    /**
     * @param listener
     *            The event listener.
     */
    public static synchronized void unregisterListener(IEventListener listener) {
        sInstance.mBatteryListeners.remove(listener);
        sInstance.mPackageListeners.remove(listener);
        sInstance.mSdStateListeners.remove(listener);
        sInstance.mSimStateListeners.remove(listener);
        sInstance.mSmsListeners.remove(listener);
        sInstance.mContactsListeners.remove(listener);
        sInstance.mCalendarEventListeners.remove(listener);
        sInstance.mBackupAndRestoreEventListeners.remove(listener);
    }

    /**
     * @param event
     *            The source event.
     */
    public static void dispatchBatteryStateChangedEvent(Event event) {
        for (IBatteryListener listener : sInstance.mBatteryListeners) {
            listener.onBatteryStateChanged(event);
        }
    }

    /**
     * @param event
     *            The source event.
     */
    public static void dispatchPackageAddedEvent(Event event) {
        for (IPackageListener listener : sInstance.mPackageListeners) {
            listener.onPackageAdded(event);
        }
    }

    /**
     * @param event
     *            The source event.
     */
    public static void dispatchPackageDataClearedEvent(Event event) {
        for (IPackageListener listener : sInstance.mPackageListeners) {
            listener.onPackageDataCleared(event);
        }
    }

    /**
     * @param event
     *            The source event.
     */
    public static void dispatchSdStateChangedEvent(Event event) {
        for (ISdStateListener listener : sInstance.mSdStateListeners) {
            listener.onSdStateChanged(event);
        }
    }

    /**
     * @param event
     *            The source event.
     */
    public static void dispatchSimStateChangedEvent(Event event) {
        for (ISimStateListener listener : sInstance.mSimStateListeners) {
            listener.onSimStateChanged(event);
        }
    }

    /**
     * @param event
     *            The source event.
     */
    public static void dispatchSmsSentEvent(Event event) {
        for (ISmsListener listener : sInstance.mSmsListeners) {
            listener.onSmsSent(event);
        }
    }

    /**
     * @param event
     *            The source event.
     */
    public static void dispatchSmsReceivedEvent(Event event) {
        for (ISmsListener listener : sInstance.mSmsListeners) {
            listener.onSmsReceived(event);
        }
    }

    /**
     * @param event
     *            The source event.
     */
    public static void dispatchSmsInsertedEvent(Event event) {
        for (ISmsListener listener : sInstance.mSmsListeners) {
            listener.onSmsInserted(event);
        }
    }

    /**
     * @param event
     *            The source event.
     */
    public static void dispatchMmsInsertedEvent(Event event) {
        for (IMmsListener listener : sInstance.mMmsListeners) {
            listener.onMmsInserted(event);
        }
    }

    /**
     * @param event
     *            The source event.
     */
    public static void dispatchMmsReceiveEvent(Event event) {
        for (IMmsListener listener : sInstance.mMmsListeners) {
            listener.onMmsReceived(event);
        }
    }

    /**
     * @param event
     *            The source event.
     */
    public static void dispatchContactsContentChangedEvent(Event event) {
        for (IContactsListener listener : sInstance.mContactsListeners) {
            listener.onContactsContentChanged(event);
        }
    }

    /**
     * @param event
     *            The source event.
     */
    public static void dispatchCalendarEventChangedEvent(Event event) {
        for (ICalendarEventListener listener : sInstance.mCalendarEventListeners) {
            listener.onCalendarEventContentChanged(event);
        }
    }
    
    /**
     * @param event
     *            The source event.
     */
    public static void dispatchEmailBackupEndEvent(Event event) {
        for (IBackupAndRestoreListener listener : sInstance.mBackupAndRestoreEventListeners) {
            listener.onEmailBackupEnd(event);
        }
    }
    
    /**
     * @param event
     *            The source event.
     */
    public static void dispatchEmailRestoreEndEvent(Event event) {
        for (IBackupAndRestoreListener listener : sInstance.mBackupAndRestoreEventListeners) {
            listener.onEmailRestoreEnd(event);
        }
    }
}
