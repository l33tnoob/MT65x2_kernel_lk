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

#ifndef ANDROID_GUI_RINGBUFFER_H
#define ANDROID_GUI_RINGBUFFER_H

#include <utils/Vector.h>
#include <utils/Mutex.h>
#include <utils/StrongPointer.h>
#include <utils/RefBase.h>

#include <cutils/xlog.h>

namespace android {
// ----------------------------------------------------------------------------

#define DEFAULT_RING_BUFFER_SIZE 10

template <typename TYPE>
class RingBuffer : public virtual RefBase {
public:

    // for custom data push
    class Pusher : public virtual RefBase {
    protected:
        RingBuffer<TYPE>& mRingBuffer;

        // use friend privillege to access necessary data
        TYPE& editHead();

    public:
        Pusher(RingBuffer<TYPE>& rb) : mRingBuffer(rb) {}
        virtual bool push(const TYPE& in) = 0;
    };

    // for custom data dump
    class Dumper : public virtual RefBase {
    protected:
        RingBuffer<TYPE>& mRingBuffer;

        // use friend privillege to access necessary data
        const TYPE& getItem(uint32_t idx);

    public:
        Dumper(RingBuffer<TYPE>& rb) : mRingBuffer(rb) {}
        virtual void dump() = 0;
    };

private:
    Vector<TYPE> mBuffer;

    uint32_t mHead;
    uint32_t mSize;
    uint32_t mCount;

    Mutex mLock;

    sp<Pusher> mPusher;
    sp<Dumper> mDumper;

public:
    RingBuffer(uint32_t size) { resize(size, true); }
    RingBuffer() { resize(DEFAULT_RING_BUFFER_SIZE, true); }
    ~RingBuffer() {}

    uint32_t getSize() { return mSize; }
    uint32_t getCount() { return mCount; }
    uint32_t getValidSize() { return (mSize < mCount) ? mSize : mCount; }
    Mutex getLock() { return mLock; }

    // reset buffers and counters
    void resize(uint32_t size, bool force = false);

    // push data to head, and return the count
    uint32_t push(const TYPE& in);

    // just print some info if no dumper assigned
    void dump();

    void setPusher(sp<Pusher>& pusher);
    void setDumper(sp<Dumper>& dumper);

    friend class Pusher;
    friend class Dumper;
};

// ----------------------------------------------------------------------------
template <typename TYPE>
TYPE& RingBuffer<TYPE>::Pusher::editHead() {
    return mRingBuffer.mBuffer.editItemAt(mRingBuffer.mHead);
}

template <typename TYPE>
const TYPE& RingBuffer<TYPE>::Dumper::getItem(uint32_t idx) {
    uint32_t oldest = (mRingBuffer.mCount > mRingBuffer.mSize)
                    ? mRingBuffer.mHead
                    : 0;
    idx = (oldest + idx) % mRingBuffer.mSize;
    return mRingBuffer.mBuffer[idx];
}

template <typename TYPE>
void RingBuffer<TYPE>::resize(uint32_t size, bool force) {
    Mutex::Autolock lock(mLock);

    if ((mSize != size) || force) {
        mSize = size;
        mBuffer.clear();
        mBuffer.resize(mSize);
        mHead = 0;
        mCount = 0;

        XLOGI("[%s] %p resize to %u (force=%s)",
                __func__, this, mSize, force ? "true" : "false");
    }
}

template <typename TYPE>
uint32_t RingBuffer<TYPE>::push(const TYPE& in) {
    Mutex::Autolock lock(mLock);

    if (mPusher != NULL) {
        if (!mPusher->push(in))
            return mCount;
    } else {
        mBuffer.replaceAt(in, mHead);
    }

    // Update pointer
    mHead++;
    if (mHead >= mSize)
        mHead = 0;

    return mCount++;
}

template <typename TYPE>
void RingBuffer<TYPE>::dump() {
    Mutex::Autolock lock(mLock);

    XLOGI("[%s] %p (mHead:%d, mSize:%d, mCount:%d)",
        __func__, this, mHead, mSize, mCount);

    if (mDumper != NULL) {
        mDumper->dump();
    }
}

template <typename TYPE>
void RingBuffer<TYPE>::setPusher(sp<Pusher>& pusher) {
    Mutex::Autolock lock(mLock);
    mPusher = pusher;
}

template <typename TYPE>
void RingBuffer<TYPE>::setDumper(sp<Dumper>& dumper) {
    Mutex::Autolock lock(mLock);
    mDumper = dumper;
}


// ----------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_GUI_RINGBUFFER_H
