#ifndef GEM_QUEUE_H
#define GEM_QUEUE_H

#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <utils/Vector.h>

#include <ui/GraphicBuffer.h>
#include <ui/Fence.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

// GEM utils
#include "thread.h"


namespace android
{


// const define
enum { INVALID_BUFFER_SLOT = -1 };
enum { NO_BUFFER_AVAILABLE = -1 };


// for both input and output
class BufferSlot
{
public:
    // buffer index in queue (need no reset)
    int mIndex;

    // frame dequeued count
    uint32_t mFrameNumber;

    // state for operations
    enum State
    {
        FREE = 0,
        DEQUEUED = 1,
        QUEUED = 2,
        ACQUIRED = 3
    };
    State mState;

    // native data and sync
    sp<GraphicBuffer> mGraphicBuffer;
    sp<Fence>         mFence;

    // egl data for G3D usage
    EGLDisplay  mEglDisplay;
    EGLImageKHR mEglImage;
    EGLSyncKHR  mEglFence;

    BufferSlot();
    ~BufferSlot();

    // clear up
    void release();
};


// simplified buffer queue for buffer convert control
class GEMQueue : public LightRefBase<GEMQueue>
{
public:
    // max buffer slots will be cached in the queue
    enum { NUM_MAX_SLOTS = 3 };

    // set graphic buffer properties
    enum { BUFFER_FORMAT = HAL_PIXEL_FORMAT_RGBA_8888 };
    enum { BUFFER_USAGE = GRALLOC_USAGE_SW_READ_RARELY | GRALLOC_USAGE_SW_WRITE_RARELY |
                          GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_HW_RENDER |
                          GRALLOC_USAGE_HW_COMPOSER };

    GEMQueue();
    ~GEMQueue();

    // producer get buffer from queue
    status_t dequeueBuffer(BufferSlot** buffer, uint32_t w, uint32_t h);

    // producer return buffer back into queue
    // and the buffer is going to be used
    status_t queueBuffer(BufferSlot* buffer);

    // producer return buffer back into queue
    // but the buffer will not be processed
    status_t cancelBuffer(BufferSlot* buffer);

    // consumer acquire the next pending buffer by consumer
    status_t acquireBuffer(BufferSlot** buffer);

    // consumer releases a buffer slot from the consumer back
    status_t releaseBuffer(BufferSlot* buffer);

    // setup GEMThread to use egl env info
    status_t setGEMThread(const sp<GEMThread>& thread);

    // dump internal states
    status_t dump();

private:
    // thread safe
    mutable Mutex mMutex;

    // buffer storage
    BufferSlot mSlots[NUM_MAX_SLOTS];

    // FIFO mQueue is for consumer to take
    typedef Vector<int> Fifo;
    Fifo mQueue;

    // inc with buffer queue
    // and used to find oldest buffer when dequeue
    uint32_t mFrameCounter;

    // wait condition when no buffer to dequeue
    mutable Condition mDequeueCondition;

    // use GEMThread as egl provider
    sp<GEMThread> mThread;

    // check if buffer is valid
    // check buffer index range and expected state
    status_t validateBuffer(const BufferSlot* buffer, BufferSlot::State state);

    // create egl image for the buffer slot
    status_t createImage(BufferSlot* buffer);
};


} // namespace android
#endif
