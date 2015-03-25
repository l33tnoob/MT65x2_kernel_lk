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

#define LOG_TAG "TableOfContentThread"
#include <utils/Log.h>

#include "TableOfContentThread.h"
#include <cutils/sched_policy.h>
#include <cutils/iosched_policy.h>

#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <pthread.h>
#include <sched.h>
#include <cutils/xlog.h>

#define ENABLE_TOC_DBUGLOG

#ifdef ENABLE_TOC_DBUGLOG

#define TOC_PRINT_DBG(fmt, arg...) SXLOGD(fmt, ##arg)
#define TOC_PRINT_WARN(fmt, arg...) SXLOGW(fmt, ##arg)
#define TOC_PRINT_ERR(fmt, arg...)  SXLOGE("Err: %5d:, "fmt, __LINE__, ##arg)

#else
#define TOC_PRINT_DBG(a,...)
#define TOC_PRINT_WARN(a,...)
#define TOC_PRINT_ERR(a,...)
#endif



namespace android
{

TableOfContentThread::TableOfContentThread()
{
    TOC_PRINT_DBG("TableOfContentThread Construct !%p", this);
    mRunning = false;
    mStopped = true;
    mEntryCount = 0;
    mTocInterval = 0;
    mThread = -1;
    mTocComplete = false;
    mDuration = 0;
    mFirstFramePos = 0;
    mCurFilePos = mFirstFramePos;
    mNextFilePos = 0;
    mTocIntervalLeft = 1;
    mFrameNum = 0;
    mTocTimeUs = 0;
    mTocSize = 0;
    m_Toc.size = 0;


}

TableOfContentThread::~TableOfContentThread()
{
    TOC_PRINT_DBG("~TableOfContentThread %p!", this);
    stopTOCThread();
}

void TableOfContentThread::startTOCThread(off_t firstFramePos, uint32_t TocSize, uint32_t TocInterval)
{
    TOC_PRINT_DBG("TableOfContentThread::startTOCThread %p", this);

    if (mRunning)
    {
        return;
    }

    mFirstFramePos = firstFramePos;
    mCurFilePos = mFirstFramePos;
    mStopped = false;
    mTocSize = TocSize;
    mTocInterval = TocInterval;
    m_Toc.TocPos.setCapacity(mTocSize);
    m_Toc.TocTS.setCapacity(mTocSize);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&mThread, &attr, threadWrapper, this);
    pthread_attr_destroy(&attr);
    mRunning = true;
}
void TableOfContentThread::stopTOCThread()
{
    TOC_PRINT_DBG("stopTOCThread %p", this);

    if (mRunning)
    {
        void *dummy = 0;
        mStopped = true;
        pthread_join(mThread, &dummy);
        TOC_PRINT_DBG("stopTOCThread pthread_join");
        mRunning = false;
    }
}

//static
void *TableOfContentThread::threadWrapper(void *me)
{
    TOC_PRINT_DBG("TableOfContentThread::threadWrapper");
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_BACKGROUND);// Normal 10

    if (android_set_ioprio(PRIO_PROCESS, IoSchedClass_NONE, 0))
    {
        TOC_PRINT_ERR("set io prio fail:: tid= %d !!!", gettid());
    }

    static_cast<TableOfContentThread *>(me)->threadEntry();

    return NULL;
}

void TableOfContentThread::threadEntry()
{
    prctl(PR_SET_NAME, (unsigned long)"TableOfContentThread", 0, 0, 0);
    int t_pid = 0;
    int pri = getpriority(PRIO_PROCESS, t_pid);
    TOC_PRINT_DBG("TableOfContentThread::threadEntry ,priority = %d,pid=%d", pri, getpid());

    while (!mStopped)
    {
        Mutex::Autolock autoLock(mLock);

        if (useAFrameToTOC() != OK)
        {
            break;
        }
    }

    if (mStopped)
    {
        mEntryCount = 0;
        mTocComplete = false;
        TOC_PRINT_WARN("build TocTable process terminated ");
    }
    else
    {
        mTocComplete = true;
        mTocTimeUs = (mTocTimeUs + 500) / 1000 * 1000;
        mDuration = mTocTimeUs;
        sendDurationUpdateEvent(mDuration);
        TOC_PRINT_DBG("build TocTable process over--actual duration=%lld,mFrameNum=%d,mEntryCount=%d", mDuration, mFrameNum, mEntryCount);
    }

    return;
}

status_t TableOfContentThread::useAFrameToTOC()
{
    int64_t frameUs = 0;

    if (getNextFramePos(&mCurFilePos, &mNextFilePos, &frameUs) != OK)
    {
        return ERROR_END_OF_STREAM;
    }

    //TOC_PRINT_DBG("curPos=%d,nextPos=%d,framenum=%u,PerframeUs=%lld",mCurFilePos,mNextFilePos,mFrameNum,frameUs);
    --mTocIntervalLeft;

    if (0 == mTocIntervalLeft)
    {

        if (mEntryCount >= mTocSize)
        {
            for (int i = 0; i < (mTocSize >> 1); i++)
            {
                //TOC_PRINT_DBG("m_Toc.mTocPos[%d]=%ld,m_Toc.mTocPos[%d]=%ld",i,m_Toc.TocPos[i],i<<1,m_Toc.TocPos[i<<1]);
                //TOC_PRINT_DBG("m_Toc.TocTS[%d]=%lld,m_Toc.TocTS[%d]=%lld",i,m_Toc.TocTS[i],i<<1,m_Toc.TocTS[i<<1]);
                m_Toc.TocPos.replaceAt(m_Toc.TocPos[i << 1], i);
                m_Toc.TocTS.replaceAt(m_Toc.TocTS[i << 1], i);
            }

            mEntryCount = mTocSize >> 1;
            mTocInterval <<= 1;
            TOC_PRINT_DBG("mTocInterval=%u", mTocInterval);
        }

        if (m_Toc.size < mTocSize)
        {
            m_Toc.TocPos.push(mCurFilePos);
            m_Toc.TocTS.push(mTocTimeUs);
            m_Toc.size++;
        }
        else
        {
            m_Toc.TocPos.replaceAt(mCurFilePos, mEntryCount);
            m_Toc.TocTS.replaceAt(mTocTimeUs, mEntryCount);
        }

        mEntryCount++;
        mTocIntervalLeft = mTocInterval;

    }

    mFrameNum++;
    mCurFilePos = mNextFilePos;
    mTocTimeUs += frameUs;
    return OK;
}

status_t TableOfContentThread::getFramePos(int64_t targetTimeUs, int64_t *pActualTimeUs, off_t *pActualPos, bool bMandatory)
{
    Mutex::Autolock autoLock(mLock);
    TOC_PRINT_DBG("getFramePos--TargetTimeUs=%lld,tableTimeUs=%lld,bMandatory =%d", targetTimeUs, mTocTimeUs, bMandatory);
    TOC_PRINT_DBG("getFramePos--mTocInterval=%u,mEntryCount=%u", mTocInterval, mEntryCount);
    int     entry = 0;
    off_t   curpos = 0;
    off_t   nexpos = 0;
    int64_t nowTs = 0;
    int64_t nextTs = 0;
    int64_t frameUs = 0;

    if (targetTimeUs > mTocTimeUs)
    {
        int64_t diffTimeUs = targetTimeUs - mTocTimeUs;

        if (bMandatory && !mTocComplete)
        {
            if (diffTimeUs < 480000000ll) //8 min
            {
                TOC_PRINT_DBG("Mandatory and little than 8 min");

                while (mTocTimeUs < targetTimeUs)
                {
                    if (useAFrameToTOC() != OK)
                    {
                        return ERROR_END_OF_STREAM;
                    }

                }
            }
            else
            {
                *pActualTimeUs  = m_Toc.TocTS[mEntryCount - 1];
                *pActualPos     = m_Toc.TocPos[mEntryCount - 1];
                TOC_PRINT_DBG("difftime >8min seek::curpos=%d,nowTs=%lld", *pActualPos, *pActualTimeUs);
                return BAD_VALUE;
            }
        }
        else
        {
            if (mEntryCount > 0)
            {
                *pActualTimeUs  = m_Toc.TocTS[mEntryCount - 1];
                *pActualPos     = m_Toc.TocPos[mEntryCount - 1];
            }
            else  //when table has no items,we can't vector ,otherwise ne.
            {
                *pActualTimeUs  = 0;
                *pActualPos     = 0;
            }

            TOC_PRINT_WARN("SeekPoint is not in toc and Non Mandatory curpos=%d,nowTs=%lld!", *pActualPos, *pActualTimeUs);
            return BAD_VALUE;
        }
    }

    off_t start = 0;
    off_t end = mEntryCount - 1;
    off_t mid = 0;
    TOC_PRINT_DBG("mEntryCount =%u,start=%d,end=%d", mEntryCount, start, end);

    if (end >= 0)
    {
        while (start <= end)
        {
            mid = (start + end) / 2;

            if (m_Toc.TocTS[mid] < targetTimeUs)
            {
                start = mid + 1;
            }
            else if (m_Toc.TocTS[mid] > targetTimeUs)
            {
                end = mid - 1;
            }
            else
            {
                *pActualTimeUs  = m_Toc.TocTS[mid];
                *pActualPos     = m_Toc.TocPos[mid];
                TOC_PRINT_DBG("seek::curpos=%d,nowTs=%lld", *pActualPos, *pActualTimeUs);
                return OK;
            }

            //LOGD("mid=%d,start=%d,end=%d",mid,start,end);
        }

        entry = end;
        curpos = m_Toc.TocPos[entry];
        nexpos = curpos;
        nowTs = m_Toc.TocTS[entry];
        nextTs = nowTs;
    }
    else
    {
        *pActualTimeUs = 0;
        *pActualPos = 0;
        TOC_PRINT_DBG("@@ SeekTable has no item.Return Start Location!curpos=%d,nowTs=%lld", *pActualPos, *pActualTimeUs);
        return OK;
    }

    TOC_PRINT_DBG("+seek::curpos=%d,nowTs=%lld,entry =%d", curpos, nowTs, entry);

    while (nowTs < targetTimeUs)
    {
        curpos = nexpos;
        nowTs  = nextTs;

        if (getNextFramePos(&curpos, &nexpos, &frameUs) != OK)
        {
            TOC_PRINT_DBG("ERROR_END_OF_STREAM");
            return ERROR_END_OF_STREAM;
        }

        nextTs += frameUs;
        frameUs = 0;
    }

    *pActualTimeUs  = nowTs;
    *pActualPos     = curpos;
    TOC_PRINT_DBG("-seek::curpos=%d,nowTs=%lld", curpos, nowTs);
    return OK;
}

}

