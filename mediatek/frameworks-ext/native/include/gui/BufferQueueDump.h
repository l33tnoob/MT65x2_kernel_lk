/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_GUI_BUFFERQUEUEDUMP_H
#define ANDROID_GUI_BUFFERQUEUEDUMP_H

#include <ui/Fence.h>
#include <ui/GraphicBuffer.h>

#include <utils/Vector.h>

#include <gui/mediatek/RingBuffer.h>

namespace android {
// ----------------------------------------------------------------------------

#ifdef USE_DP

class BufferQueue;
class AcquiredBuffer;
class BackupBuffer;
class BackupBufPusher;
class BackupBufDumper;

class BufferQueueDump : public virtual RefBase {
private:
    mutable RingBuffer< sp<BackupBuffer> > mBackupBuf;
    mutable sp<BackupBufPusher> mBackupBufPusher;
    mutable sp<BackupBufDumper> mBackupBufDumper;
    mutable bool mBackupBufInited;

    String8 mName;

    Vector< sp<AcquiredBuffer> > mAcquiredBufs;

    void updateBuffer(const int slot);

public:
    BufferQueueDump();
    void setName(String8& name);
    void dumpBuffer() const;
    int checkBackupCount() const;
    void onAcquireBuffer(const int slot, const sp<GraphicBuffer>& buffer, const sp<Fence>& fence);
    void onReleaseBuffer(const int slot);
    void onFreeBuffer(const int slot);
    static void getDumpFileName(String8& fileName, const String8& name);
};

class BackupBufPusher : public RingBuffer< sp<BackupBuffer> >::Pusher {
public:
    BackupBufPusher(RingBuffer< sp<BackupBuffer> >& rb) :
        RingBuffer< sp<BackupBuffer> >::Pusher(rb) {}
    virtual bool push(const sp<BackupBuffer>& in);
};

class BackupBufDumper : public RingBuffer< sp<BackupBuffer> >::Dumper {
public:
    BackupBufDumper(RingBuffer< sp<BackupBuffer> >& rb) :
        RingBuffer< sp<BackupBuffer> >::Dumper(rb),
        mName("") {}
    BackupBufDumper(RingBuffer< sp<BackupBuffer> >& rb, const String8& name) :
        RingBuffer< sp<BackupBuffer> >::Dumper(rb),
        mName(name) {}
    virtual void dump();
    void setName(String8& name) { mName = name; }

private:
    String8 mName;
};

class AcquiredBuffer : public virtual RefBase {
public:
    AcquiredBuffer();
    AcquiredBuffer(const int slot, const sp<GraphicBuffer> buffer,
                    const sp<Fence>& fence, const nsecs_t timestamp = 0);

private:
    int mSlot;
    sp<GraphicBuffer> mGraphicBuffer;
    sp<Fence> mFence;
    nsecs_t mTimeStamp;

    friend class BufferQueueDump;
};

class BackupBuffer : public virtual RefBase {
public:
    BackupBuffer() : mTimeStamp(0) {}
    BackupBuffer(const sp<GraphicBuffer> buffer, const nsecs_t timestamp) :
        mTimeStamp(timestamp),
        mGraphicBuffer(buffer) {}

private:
    sp<GraphicBuffer> mGraphicBuffer;
    nsecs_t mTimeStamp;

    friend class BackupBufPusher;
    friend class BackupBufDumper;
};

#else // nodef USE_DP

// dummy implement for no dpframework
// the public member must be consistent with original implement
class BufferQueueDump : public virtual RefBase {
public:
    BufferQueueDump() {};
    void setName(String8& name) {};
    void dumpBuffer() const {};
    int checkBackupCount() const { return 0; };
    void onAcquireBuffer(const int slot, const sp<GraphicBuffer>& buffer, const sp<Fence>& fence) {};
    void onReleaseBuffer(const int slot) {};
    void onFreeBuffer(const int slot) {};
};

#endif // USE_DP

// ----------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_GUI_BUFFERQUEUEDUMP_H
