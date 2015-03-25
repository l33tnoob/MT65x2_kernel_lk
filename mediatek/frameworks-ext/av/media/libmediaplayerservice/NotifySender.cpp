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

/*
** this module is used to send notify message to mediaplayer to avoid deadlock.
** cameraservice create mediaplayer to play shuttersound, and it will call
** play() and seekTo() functions of mediaplayer, which will notify messages to mediaplayer.
** because cameraservice and mediaplayerservice are in mediaserver process,  notify() function
** of mediaplayer will be called directly instead of binder call.
**
** mediaplayer call seekTo get mediaplayer's lock, and then it will try to get awesomeplayer's lock.
** awesomeplayer's  event like onstreamdone has get its lock and send message to mediaplayer using function notify()
**which will try to get mediaplayer's lock, then deadlock happens.
*/
#define LOG_TAG "NotifySender"
#include "NotifySender.h"
#include <cutils/xlog.h>
#include <linux/rtpm_prio.h>
namespace android
{


NotifySender::NotifySender()
    : Thread(false)
{
    SXLOGD("NotifySender construct");
}

NotifySender::~NotifySender()
{
    SXLOGD("NotifySender deconstruct in");
    //  sp <NotifySender> strongMe = this;
    {
        AutoMutex lock(&mLock);
        requestExit();
        mWaitWorkCV.signal();
    }
    requestExitAndWait();
    clear();
    SXLOGD("NotifySender deconstruct out");
}

void NotifySender::clear()
{
    Mutex::Autolock _l(mLock);

    for (List<NotifyMessage *>::iterator i = mMsgQueue.begin(); i != mMsgQueue.end(); ++i) //clear any msg in queue
    {
        NotifyMessage *p = *i;
        delete p;
    }

    mMsgQueue.clear();
}

bool NotifySender::threadLoop()
{
    while (!exitPending())
    {
        NotifyMessage *p = 0;
        {
            Mutex::Autolock _l(mLock);

            while (mMsgQueue.empty())
            {
                mWaitWorkCV.wait(mLock);
            }

            if (exitPending())
            {
                break;
            }

            List<NotifyMessage *>::iterator i = mMsgQueue.begin();
            p = *i;
            mMsgQueue.erase(i);
            SXLOGV("messgesize=%d", mMsgQueue.size());
        }

        if (p)
        {
            p->mOwner->notify(p->mMsg, p->mExt1, p->mExt2, &p->mParcel);
            delete p;
        }

        /* for( List<NotifyMessage *>::iterator i = mMsgQueue.begin(); i!= mMsgQueue.end(); ++i) //send all notify msg in queue
         {
             NotifyMessage *p = *i;
             p->mOwner->notify(p->mMsg,p->mExt1,p->mExt2,&p->mParcel);
             delete p;
         }
         mMsgQueue.clear();
         */
    }

    clear();
    return false;
}
status_t NotifySender::readyToRun()
{

    int result = -1;

    if (result == -1)
    {
        struct sched_param sched_p;
        sched_getparam(0, &sched_p);
        sched_p.sched_priority = RTPM_PRIO_AUDIO_PLAYBACK;

        if (0 != sched_setscheduler(0, SCHED_RR, &sched_p))
        {
            SXLOGV("[%s] failed, errno: %d", __func__, errno);
        }
        else
        {
            sched_getparam(0, &sched_p);
            SXLOGV("sched_setscheduler ok, priority: %d", sched_p.sched_priority);
        }
    }

    return NO_ERROR;
}
void NotifySender::onFirstRef()
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    uint32_t pThis = (uint32_t)this;

    snprintf(buffer, SIZE, "NotifySender 0x%04x", pThis);

    run(buffer, PRIORITY_NORMAL);
}
status_t NotifySender::sendMessage(sp<IMediaPlayerClient> owner, int msg, int ext1, int ext2, const Parcel *obj)
{
    SXLOGD("sendMessage::owner(%p),msg(%d)", owner.get(), msg);
    Mutex::Autolock _l(mLock);
    NotifyMessage *pMsg = new NotifyMessage(owner, msg, ext1, ext2, obj);
    mMsgQueue.push_back(pMsg);
    mWaitWorkCV.signal();
    return NO_ERROR;
}


}
