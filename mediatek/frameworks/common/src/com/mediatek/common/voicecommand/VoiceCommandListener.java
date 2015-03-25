/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.common.voicecommand;

import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import com.mediatek.common.voicecommand.IVoiceCommandListener;

public abstract class VoiceCommandListener {

    public static final String VOICE_COMMAND_SERVICE = "voicecommand";
    public static final String VOICE_SERVICE_ACTION = "com.mediatek.voicecommand";
    public static final String VOICE_SERVICE_CATEGORY = "com.mediatek.nativeservice";

    private static final int ACTION_COMMON = 1;

    public static final int ACTION_MAIN_VOICE_COMMON = ACTION_COMMON + 0;
    public static final int ACTION_MAIN_VOICE_UI = ACTION_COMMON + 1;
    public static final int ACTION_MAIN_VOICE_TRAINING = ACTION_COMMON + 2;
    public static final int ACTION_MAIN_VOICE_RECOGNIZE = ACTION_COMMON + 3;
    public static final int ACTION_MAIN_VOICE_SETTING = ACTION_COMMON + 4;
    public static final int ACTION_MAIN_VOICE_UNREGISTER = ACTION_COMMON + 5;
    public static final int ACTION_MAIN_VOICE_CONTACTS = ACTION_COMMON + 6;

    public static final int ACTION_VOICE_COMMON_KEYWORD = ACTION_COMMON + 0;
    public static final int ACTION_VOICE_COMMON_COMMANDPATH = ACTION_COMMON + 1;
    public static final int ACTION_VOICE_COMMON_PROCSTATE = ACTION_COMMON + 2;

    public static final int ACTION_VOICE_UI_START = ACTION_COMMON + 0;
    public static final int ACTION_VOICE_UI_STOP = ACTION_COMMON + 1;
    public static final int ACTION_VOICE_UI_ENABLE = ACTION_COMMON + 2;
    public static final int ACTION_VOICE_UI_DISALBE = ACTION_COMMON + 3;
    public static final int ACTION_VOICE_UI_NOTIFY = ACTION_COMMON + 4;

    public static final int ACTION_VOICE_TRAINING_START = ACTION_COMMON + 0;
    public static final int ACTION_VOICE_TRAINING_STOP = ACTION_COMMON + 1;
    public static final int ACTION_VOICE_TRAINING_INTENSITY = ACTION_COMMON + 2;
    public static final int ACTION_VOICE_TRAINING_PSWDFILE = ACTION_COMMON + 3;
    public static final int ACTION_VOICE_TRAINING_NOTIFY = ACTION_COMMON + 4;
    public static final int ACTION_VOICE_TRAINING_RESET = ACTION_COMMON + 5;

    public static final int ACTION_VOICE_RECOGNIZE_START = ACTION_COMMON + 0;
    public static final int ACTION_VOICE_RECOGNIZE_INTENSITY = ACTION_COMMON + 1;
    public static final int ACTION_VOICE_RECOGNIZE_NOTIFY = ACTION_COMMON + 2;

    public static final int ACTION_VOICE_CONTACTS_START = ACTION_COMMON + 0;
    public static final int ACTION_VOICE_CONTACTS_STOP = ACTION_COMMON + 1;
    public static final int ACTION_VOICE_CONTACTS_ENABLE = ACTION_COMMON + 2;
    public static final int ACTION_VOICE_CONTACTS_DISABLE = ACTION_COMMON + 3;
    public static final int ACTION_VOICE_CONTACTS_INTENSITY = ACTION_COMMON + 4;
    public static final int ACTION_VOICE_CONTACTS_SELECTED = ACTION_COMMON + 5;
    public static final int ACTION_VOICE_CONTACTS_NOTIFY = ACTION_COMMON + 6;
    public static final int ACTION_VOICE_CONTACTS_NAME = ACTION_COMMON + 7;
    public static final int ACTION_VOICE_CONTACTS_SPEECHDETECTED = ACTION_COMMON + 8;
    public static final int ACTION_VOICE_CONTACTS_SEARCHCNT = ACTION_COMMON + 9;
    public static final int ACTION_VOICE_CONTACTS_ORIENTATION = ACTION_COMMON + 10;

    public static final int ACTION_VOICE_SETTING_PROCESSLIST = ACTION_COMMON + 0;
    public static final int ACTION_VOICE_SETTING_PROCESSUPATE = ACTION_COMMON + 1;
    public static final int ACTION_VOICE_SETTING_LANGUAGELIST = ACTION_COMMON + 2;
    public static final int ACTION_VOICE_SETTING_LANGUAGEUPDATE = ACTION_COMMON + 3;
    public static final int ACTION_VOICE_SETTING_KEYWORDPATH = ACTION_COMMON + 4;
    public static final int ACTION_VOICE_SETTING_PROCESSUPATEALL = ACTION_COMMON + 5;

    public static final int ACTION_EXTRA_RESULT_SUCCESS = 1;
    public static final int ACTION_EXTRA_RESULT_ERROR = 10;
    public static final String ACTION_EXTRA_RESULT = "Result";
    public static final String ACTION_EXTRA_RESULT_INFO = "Result_Info";
    public static final String ACTION_EXTRA_RESULT_INFO1 = "Reslut_INfo1";
    public static final String ACTION_EXTRA_SEND = "Send";
    public static final String ACTION_EXTRA_SEND_INFO = "Send_Info";
    public static final String ACTION_EXTRA_SEND_INFO1 = "Send_Info1";

    private static final int VOICE_ERROR_RECOGNIZE = 0;
    private static final int VOICE_ERROR_TRAINING = 100;
    private static final int VOICE_ERROR_SETTING = 200;
    private static final int VOICE_ERROR_UI = 300;
    private static final int VOICE_ERROR_CONTACTS = 400;
    private static final int VOICE_ERROR_COMMON = 1000;

    public static final int VOICE_NO_ERROR = 0;
    public static final int VOICE_ERROR_RECOGNIZE_DENIED = VOICE_ERROR_RECOGNIZE + 1;
    public static final int VOICE_ERROR_RECOGNIZE_NOISY = VOICE_ERROR_RECOGNIZE + 2;
    public static final int VOICE_ERROR_RECOGNIZE_LOWLY = VOICE_ERROR_RECOGNIZE + 3;

    public static final int VOICE_ERROR_TRAINING_NOT_ENOUGH = VOICE_ERROR_TRAINING + 1;
    public static final int VOICE_ERROR_TRAINING_NOISY = VOICE_ERROR_TRAINING + 2;
    public static final int VOICE_ERROR_TRAINING_PASSWORD_DIFF = VOICE_ERROR_TRAINING + 3;
    public static final int VOICE_ERROR_TRAINING_PASSWORD_EXIST = VOICE_ERROR_TRAINING + 4;

    public static final int VOICE_ERROR_SETTING_PROCESS_GET = VOICE_ERROR_SETTING + 1;
    public static final int VOICE_ERROR_SETTING_PROCESS_UPDATE = VOICE_ERROR_SETTING + 2;
    public static final int VOICE_ERROR_SETTING_LANGUAGE_GET = VOICE_ERROR_SETTING + 3;
    public static final int VOICE_ERROR_SETTING_LANGUAGE_UPDATE = VOICE_ERROR_SETTING + 4;

    public static final int VOICE_ERROR_UI_INVALID = VOICE_ERROR_UI + 1;

    public static final int VOICE_ERROR_CONTACTS_VOICEINVALID = VOICE_ERROR_CONTACTS + 1;
    public static final int VOICE_ERROR_CONTACTS_SENDINVALID = VOICE_ERROR_CONTACTS + 2;

    public static final int VOICE_ERROR_COMMON_PROCESSOFF = VOICE_ERROR_COMMON + 1;
    public static final int VOICE_ERROR_COMMON_PERMISSION = VOICE_ERROR_COMMON + 2;
    public static final int VOICE_ERROR_COMMON_REGISTERED = VOICE_ERROR_COMMON + 3;
    public static final int VOICE_ERROR_COMMON_UNREGISTER = VOICE_ERROR_COMMON + 4;
    public static final int VOICE_ERROR_COMMON_ILLEGALPROCESS = VOICE_ERROR_COMMON + 5;
    public static final int VOICE_ERROR_COMMON_SERVICE = VOICE_ERROR_COMMON + 6;
    public static final int VOICE_ERROR_COMMON_INVALIDACTION = VOICE_ERROR_COMMON + 7;
    public static final int VOICE_ERROR_COMMON_INVALIDDATA = VOICE_ERROR_COMMON + 8;
    public static final int VOICE_ERROR_COMMON_NOTIFYFAIL = VOICE_ERROR_COMMON + 9;
}
