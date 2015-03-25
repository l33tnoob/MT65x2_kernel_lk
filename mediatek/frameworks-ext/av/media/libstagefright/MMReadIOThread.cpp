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

#define LOG_TAG "MMReadIOThread"
#include <utils/Log.h>

#include "MMReadIOThread.h"
#include <cutils/sched_policy.h>
#include <cutils/iosched_policy.h>

#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <pthread.h>
#include <sched.h>
#include <cutils/xlog.h>

#define ENABLE_RIODBUGLOG

#ifdef ENABLE_RIODBUGLOG
#define ALOGV(fmt, arg...)  SXLOGV(fmt, ##arg)
#define ALOGD(fmt, arg...)  SXLOGD(fmt, ##arg)
#define ALOGW(fmt, arg...)  SXLOGW(fmt, ##arg)
#define ALOGE(fmt, arg...)  SXLOGE("Err: %5d:, "fmt, __LINE__, ##arg)
#endif

#define MM_READIO_ALIGN_SIZE 4096

static long long MMRIOGetTimeMs()
{
    struct timeval t1;
    long long ms;

    gettimeofday(&t1, NULL);
    ms = t1.tv_sec * 1000LL + t1.tv_usec / 1000;

    return ms;
}

namespace android
{

MMReadIOThread::MMReadIOThread()
{
    ALOGD("MMReadIOThread Construct %p", this);
    mRunning = false;
    mStopped = true;
    mThread = -1;
    mRioComplete = false;
    mDuration = 0;
    mFirstFramePos = 0;
    mNextFilePos = 0;
    mRioIntervalLeft = 1;
    mFrameNum = 0;
    mRioTimeUs = 0;
    mRioStat = OK;

    mRioFrameSize = 0;
    mRioFrameCnt = 0;
    mResetStart = false;


}

MMReadIOThread::~MMReadIOThread()
{
    ALOGD("MReadIOThread Construct~~ %p!", this);
    stopRIOThread();
}

uint32_t MMReadIOThread::GetBestMinFrameCount(uint32_t frameSize)
{
    ALOGD("GetMinFrameCount %p frmsize %d!", this, frameSize);

    if (frameSize >= 4 * 1024)
    {
        return 4;
    }
    else if (frameSize >= 2 * 1024)
    {
        return 8;
    }
    else if (frameSize >= 1 * 1024)
    {
        return 16;
    }
    else
    {
        return ((16 * 1024 + frameSize - 1) / frameSize);
    }

}

void MMReadIOThread::startRIOThread(const sp<DataSource> &dataSource, off_t firstFramePos, uint32_t frameSize, uint32_t cacheCount)
{
    ALOGD("MMReadIOThread::startRIOThread - %p, startpos %d, frmsize %d, cnt %d, run %d",
          this, firstFramePos,  frameSize, cacheCount, mRunning);

    if (mRunning)
    {
        ALOGW("startRIOThread already running");
        return;
    }

    Mutex::Autolock autoLock(mLock);

    mDataSource = dataSource;
    mFirstFramePos = firstFramePos;
    mCurrentPos = firstFramePos;
    mStopped = false;
    mRioFrameSize = frameSize;
    mRioFrameCnt = cacheCount;
    mCachedSize = 0;

    mWritePtr = 0;
    mReadPtr = 0;

    mInnerBufferSize = mRioFrameSize * mRioFrameCnt;

    if ((mInnerBufferSize / 4) >= 8 * 1024)
    {
        mAlignReadSize = 8 * 1024;
    }
    else if ((mInnerBufferSize / 4) >= 4 * 1024)
    {
        mAlignReadSize = 4 * 1024;
    }
    else if ((mInnerBufferSize / 4) >= 2 * 1024)
    {
        mAlignReadSize = 2 * 1024;
    }
    else if ((mInnerBufferSize / 4) >= 1 * 1024)
    {
        mAlignReadSize = 1024;
    }
    else
    {
        mAlignReadSize = mRioFrameSize;
    }

    mAlignAddSize = 0;
    mInnerGroup = new MediaBufferGroup;
    mInnerGroup->add_buffer(new MediaBuffer(frameSize * cacheCount + mAlignReadSize));
    status_t err = mInnerGroup->acquire_buffer(&mInnerBuffer);

    if (err != OK)
    {
        ALOGE("acquire_innnerbuffer fail");
        return ;
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&mThread, &attr, RioThreadWrapper, this);
    pthread_attr_destroy(&attr);
    mRunning = true;

    ALOGD("MMReadIOThread::startRIOThread + %p, startpos done", this);
}

void MMReadIOThread::stopRIOThread()
{
    ALOGD("stopRIOThread %p, mRunning %d", this, mRunning);

    if (mRunning)
    {
        void *dummy = 0;
        mStopped = true;
        mBufferEmptyed.signal();
        mBufferFilled.signal();

        pthread_join(mThread, &dummy);
        ALOGD("stopRIOThread pthread_join");
        mRunning = false;

        if (mInnerBuffer != NULL)
        {
            mInnerBuffer->release();
        }

        mInnerBuffer = NULL;
        delete mInnerGroup;
        mInnerGroup = NULL;

    }
}

void *MMReadIOThread::RioThreadWrapper(void *me)
{
    ALOGD("MMReadIOThread::threadWrapper");
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_BACKGROUND);// Normal 10
    static_cast<MMReadIOThread *>(me)->RioThreadEntry();

    return NULL;
}

void MMReadIOThread::RioThreadEntry()
{
    prctl(PR_SET_NAME, (unsigned long)"MMReadIOThread", 0, 0, 0);
    int t_pid = 0;
    int pri = getpriority(PRIO_PROCESS, t_pid);
    ALOGD("MMReadIOThread::threadEntry ,priority = %d,pid=%d", pri, getpid());

    while (!mStopped)
    {
        size_t emptysize = GetEmptySize();

        if ((emptysize < mAlignReadSize)
                || (mRioStat == ERROR_END_OF_STREAM))
        {
            mBufferEmptyed.wait(mEmptyedLock);
        }

        size_t readsize = 0;
        size_t readsize2 = 0;

        if (mStopped)
        {
            break ;
        }

        if (mRioStat == ERROR_END_OF_STREAM)
        {
            ALOGD("threadEntry EOS continue");
            continue;
        }

        emptysize = GetEmptySize();
        off_t mTmpReadPtr = mReadPtr;
        bool mTmpResetStat = mResetStart;
        int pretime = 0 ;

        if (emptysize >= mAlignReadSize)
        {
            pretime = MMRIOGetTimeMs();
            {
                ALOGV("threadEntry running1, Align %d, R %d, W %d, BSize %d, Empty %d",
                      mAlignReadSize, mTmpReadPtr, mWritePtr, mInnerBufferSize, emptysize);
                readsize = mDataSource->readAt(mCurrentPos, mInnerBuffer->data() + mWritePtr, mAlignReadSize);
				if((readsize>mAlignReadSize)||(readsize<0)){
					readsize=0;
				}				
                UpdateWritePtr(readsize, mTmpResetStat);
            }
        }
        else
        {
            ALOGV("threadEntry running2, Align %d, R %d, W %d, BSize %d, Empty %d",
                  mAlignReadSize, mTmpReadPtr, mWritePtr, mInnerBufferSize, emptysize);
            continue;
        }

        int nexttime = MMRIOGetTimeMs();

        if ((nexttime - pretime) > 75)
        {
            ALOGW("threadEntry readio too long %d !!!!!!!!", (nexttime - pretime));
        }

        if (((readsize + readsize2) <= 0)
                && (emptysize != 0))
        {
            mRioStat = ERROR_END_OF_STREAM;
        }

    }

    if (mStopped)
    {
        mRioComplete = false;
        ALOGD("build RioTable process terminated ");
    }
    else
    {
        mRioComplete = true;
        mRioTimeUs = (mRioTimeUs + 500) / 1000 * 1000;
        mDuration = mRioTimeUs;
    }

    return;
}



ssize_t MMReadIOThread::ReadBitsteam(void *data, size_t size)
{
    if ((size <= 0) || (size > mRioFrameSize))
    {
        ALOGE("ReadBitsteam mRioFrameSize %d readsize error: %d", mRioFrameSize, size);
        return 0;
    }

    if (data == NULL)
    {
        ALOGE("ReadBitsteam dataptr is Null %d", size);
        return 0;
    }

    if ((mRioStat == ERROR_END_OF_STREAM) && (mReadPtr == mWritePtr))
    {
        return 0;
    }

    ALOGV("ReadBitsteam - size %d, Cache %d, mRioStat %d", size, GetReadySize(), mRioStat);

    while (GetReadySize() < size) /// if data is not enough
    {
        ///ALOGV("ReadBitsteam wait, mRioStat %d", mRioStat);
        if (mRioStat == ERROR_END_OF_STREAM)
        {
            break;
        }

        mBufferEmptyed.signal();
        status_t err = mBufferFilled.waitRelative(mFilledLock, 75000000LL);

        if (err != OK)
        {
            ALOGW("too long to wait for fill buffers");
        }

        if (mStopped)
        {
            return 0;
        }

    }

    ALOGV("ReadBitsteam req + %d, R %d, W %d, BSize %d, Add %d, Cache %d",
          size, mReadPtr, mWritePtr, mInnerBufferSize, mAlignAddSize, mCachedSize);

    if (mReadPtr +  size <= mWritePtr)
    {
        memcpy(data, mInnerBuffer->data() + mReadPtr, size);
        return size ;
    }
    else // mReadPtr +  size > mWritePtr
    {
        int tempcachesize = GetReadySize();

        if (mRioStat == ERROR_END_OF_STREAM)
        {
            if (mWritePtr >= mReadPtr)
            {
                if (tempcachesize <= size)
                {
                    memcpy(data, mInnerBuffer->data() + mReadPtr, tempcachesize);
                }
                else
                {
                    memcpy(data, mInnerBuffer->data() + mReadPtr, size);
                }
            }
            else if ((mReadPtr + size) <= (mInnerBufferSize + mAlignAddSize)) /// mWritePtr < mReadPtr
            {
                memcpy(data, mInnerBuffer->data() + mReadPtr, size);
            }
            else /// mWritePtr < mReadPtr
            {
                memcpy(data, mInnerBuffer->data() + mReadPtr, mInnerBufferSize + mAlignAddSize - mReadPtr);

                if (tempcachesize <= size)
                {
                    memcpy(data + mInnerBufferSize + mAlignAddSize - mReadPtr, mInnerBuffer->data() , mWritePtr);
                }
                else
                {
                    memcpy(data + mInnerBufferSize + mAlignAddSize - mReadPtr, mInnerBuffer->data() ,
                           size - (mInnerBufferSize + mAlignAddSize - mReadPtr));
                }
            }

            return tempcachesize;
        }

        if ((mReadPtr + size) <= mInnerBufferSize + mAlignAddSize)
        {
            memcpy(data, mInnerBuffer->data() + mReadPtr, size);
            return size;
        }
        else
        {
            memcpy(data, mInnerBuffer->data() + mReadPtr, mInnerBufferSize + mAlignAddSize - mReadPtr);

            if (tempcachesize >= size)
            {
                memcpy(data + mInnerBufferSize + mAlignAddSize - mReadPtr, mInnerBuffer->data() ,
                       size - (mInnerBufferSize + mAlignAddSize - mReadPtr));
                return size;
            }
            else
            {
                memcpy(data + mInnerBufferSize + mAlignAddSize - mReadPtr, mInnerBuffer->data() ,
                       tempcachesize - (mInnerBufferSize + mAlignAddSize - mReadPtr));
                return tempcachesize;
            }
        }

    }

}

status_t MMReadIOThread::ResetReadioPtr(off_t frameStartPos)
{
    ALOGD("ResetReadioPtr %x, r %d, w %d", frameStartPos, mReadPtr,  mWritePtr);
    Mutex::Autolock autoLock(mLock);

    mReadPtr = 0;
    mWritePtr = 0;
    mFirstFramePos = frameStartPos;
    mCachedSize = 0;
    mAlignAddSize = 0;
    mCurrentPos = frameStartPos;
    mRioStat = OK;
    mResetStart = true;

    return OK;
}

status_t MMReadIOThread::UpdateReadPtr(size_t size)
{
    ALOGV("UpdateReadPtr- %d, r %d, w %d, Cache %d, add %d", size, mReadPtr,  mWritePtr, mCachedSize, mAlignAddSize);
    Mutex::Autolock autoLock(mLock);

    if ((mReadPtr + size) < (mInnerBufferSize + mAlignAddSize))
    {
        mReadPtr += size;
    }
    else
    {
        mReadPtr = (mReadPtr + size - mInnerBufferSize - mAlignAddSize);
        mAlignAddSize = 0;
    }

    if ((GetEmptySize() >= (mInnerBufferSize / 4)) ///mInnerBufferSize/2)
            && (mRioStat != ERROR_END_OF_STREAM))
    {
        ALOGD("Signal to read %d", GetEmptySize());
        mBufferEmptyed.signal();
    }

    mCachedSize -= size;

    if (mCachedSize > (mInnerBufferSize + mAlignAddSize))
    {
        ALOGW("UpdateWritePtr r _error+ %d, r %d, w %d, Cache %d, bsize %d, pos %x, add %d",
              size, mReadPtr,  mWritePtr, mCachedSize, mInnerBufferSize, mCurrentPos, mAlignAddSize);
        assert(0);
    }

    ALOGV("UpdateReadPtr+ %d, r %d, w %d, Cache %d, add %d", size, mReadPtr,  mWritePtr, mCachedSize, mAlignAddSize);
    return OK;

}

status_t MMReadIOThread::UpdateWritePtr(size_t size, bool resetstatus)
{
    ALOGV("UpdateWritePtr- %d, r %d, w %d, Cache %d, pos %x, add %d",
          size, mReadPtr,  mWritePtr, mCachedSize, mCurrentPos, mAlignAddSize);
    Mutex::Autolock autoLock(mLock);

    if (resetstatus != mResetStart)
    {
        ALOGD("UpdateReadPtr recieved reset to exit %d<-->%d", resetstatus, mResetStart);
        mResetStart = false;
        return OK;
    }

    if ((mWritePtr + size) < (mInnerBufferSize))
    {
        mWritePtr += size;
    }
    else
    {
        mAlignAddSize = mWritePtr + size - mInnerBufferSize;
        mWritePtr = 0;
    }

    mCurrentPos += size;
    mCachedSize += size;

    if (mCachedSize > (mInnerBufferSize + mAlignAddSize))
    {
        ALOGW("UpdateWritePtr w _error+ %d, r %d, w %d, Cache %d, bsize %d, pos %x, add %d",
              size, mReadPtr,  mWritePtr, mCachedSize, mInnerBufferSize, mCurrentPos, mAlignAddSize);
        assert(0);
    }

    ALOGV("UpdateWritePtr+ %d, r %d, w %d, Cache %d, pos %x, add %d",
          size, mReadPtr,  mWritePtr, mCachedSize, mCurrentPos, mAlignAddSize);
    mBufferFilled.signal();
    
    mResetStart = false;
    
    return OK;
}

size_t MMReadIOThread::GetReadySize()
{
    return mCachedSize;
}

size_t MMReadIOThread::GetEmptySize()
{
    if (mWritePtr > mReadPtr)
    {
        return (mInnerBufferSize - mWritePtr + mReadPtr);
    }
    else
    {
        if (mWritePtr < mReadPtr)
        {
            return (mReadPtr - mWritePtr);
        }
        else
        {
            return (mCachedSize ? 0 : mInnerBufferSize);
        }
    }
}

}

