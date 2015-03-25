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

#ifndef MM_READIO_THREAD_H_

#define MM_READIO_THREAD_H_

#include <media/stagefright/MediaErrors.h>
#include <utils/threads.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaBufferGroup.h>

namespace android
{
/********************************************************
class MMReadIOThread  to build Rio table
********************************************************/


class MMReadIOThread
{
public:
    enum
    {
        RIO_TABLE_SIZE = 256,
        RIO_TABLE_INTERVAL_INTIAL = 32
    };
    struct TableEntry
    {
        Vector<off_t> RioPos;
        Vector<int64_t> RioTS;
        uint32_t size;
    };

    MMReadIOThread();
    virtual ~MMReadIOThread();
	
	uint32_t GetBestMinFrameCount(uint32_t frameSize);
    void startRIOThread(const sp<DataSource> &dataSource, off_t firstFramePos, uint32_t frameSize, uint32_t cacheCount = 6);
    void stopRIOThread();

    status_t ResetReadioPtr(off_t frameStartPos);
    ssize_t ReadBitsteam(void *data, size_t size);
    status_t UpdateReadPtr(size_t size);

private:
    off_t mNextFilePos;
    off_t mRioIntervalLeft;
    uint32_t mFrameNum;
    bool mRunning;
    pthread_t mThread;
    bool mStopped;
    uint32_t mRioFrameSize;
    uint32_t mRioFrameCnt;
    sp<DataSource> mDataSource;
    off_t mCurrentPos;

    bool mRioComplete;
    bool mResetStart;
    Mutex mLock;

    Mutex mFilledLock;
    Mutex mEmptyedLock;
    Condition mBufferFilled;
    Condition mBufferEmptyed;
    status_t mRioStat;

    int64_t mDuration;
    int64_t mRioTimeUs;
    off_t mFirstFramePos;

    MediaBufferGroup *mInnerGroup;
    MediaBuffer *mInnerBuffer;
    uint32_t mInnerBufferSize;
    uint32_t mAlignReadSize;
    uint32_t mAlignAddSize;
    uint32_t mCachedSize;

    off_t mReadPtr;
    off_t mWritePtr;

    size_t GetReadySize();
    size_t GetEmptySize();
    static void *RioThreadWrapper(void *me);
    void RioThreadEntry();
    status_t UpdateWritePtr(size_t size, bool resetstatus);


};

}

#endif
