#include <utils/Trace.h>
#include <cutils/xlog.h>

#include <mediatek/gem/queue.h>


namespace android
{


BufferSlot::BufferSlot()
    : mIndex(INVALID_BUFFER_SLOT)
    , mFrameNumber(0)
    , mState(FREE)
    , mGraphicBuffer(NULL)
    , mFence(NULL)
    , mEglDisplay(EGL_NO_DISPLAY)
    , mEglImage(EGL_NO_IMAGE_KHR)
    , mEglFence(EGL_NO_SYNC_KHR)
{
}


BufferSlot::~BufferSlot()
{
}


void BufferSlot::release()
{
    // reset state
    mFrameNumber = 0;
    mState = FREE;

    // release sp
    mGraphicBuffer = NULL;
    mFence = NULL;

    // release egl
    if ((mEglImage != EGL_NO_IMAGE_KHR) && (mEglDisplay != EGL_NO_DISPLAY))
    {
        eglDestroyImageKHR(mEglDisplay, mEglImage);
    }
    mEglDisplay = EGL_NO_DISPLAY;
    mEglImage = EGL_NO_IMAGE_KHR;
    if (mEglFence != EGL_NO_SYNC_KHR)
    {
        eglDestroySyncKHR(mEglDisplay, mEglFence);
    }
    mEglFence = EGL_NO_SYNC_KHR;
}


GEMQueue::GEMQueue()
    : mFrameCounter(0)
    , mThread(NULL)
{
    for (uint32_t i = 0; i < NUM_MAX_SLOTS; i++)
    {
        mSlots[i].mIndex = i;
    }
}


GEMQueue::~GEMQueue()
{
    for (uint32_t i = 0; i < NUM_MAX_SLOTS; i++)
    {
        mSlots[i].release();
    }
}


status_t GEMQueue::dequeueBuffer(BufferSlot** outBuf, uint32_t w, uint32_t h)
{
    // invalid size
    if ((0 == w) || (0 == h))
    {
        XLOGE("[%s] invalid size: w=%u, h=%u", __FUNCTION__, w, h);
        return BAD_VALUE;
    }

    Mutex::Autolock l(mMutex);

    // get the buffer index to dequeue
    int found = INVALID_BUFFER_SLOT;
    bool tryAgain = true;
    while (tryAgain)
    {
        found = INVALID_BUFFER_SLOT;
        for (uint32_t i = 0; i < NUM_MAX_SLOTS; i++)
        {
            if (BufferSlot::FREE == mSlots[i].mState)
            {
                // return the oldest of the free buffers to avoid
                // stalling the producer if possible.
                if ((INVALID_BUFFER_SLOT == found) ||
                    (mSlots[i].mFrameNumber < mSlots[found].mFrameNumber))
                {
                    found = i;
                }
            }
        }

        // if no buffer is found, wait for a buffer to be released
        tryAgain = (INVALID_BUFFER_SLOT == found);
        if (tryAgain)
        {
            XLOGW("[%s] wait for buffer and try again", __FUNCTION__);
            mDequeueCondition.wait(mMutex);
        }
    }
    BufferSlot& buf = mSlots[found];

    // realloc buffer if needed
    sp<GraphicBuffer> sg = buf.mGraphicBuffer;
    if ((sg == NULL) ||
        (uint32_t(sg->width)  != w) ||
        (uint32_t(sg->height) != h))
    {
        XLOGI("new GraphicBuffer needed");
        if (sg != NULL)
        {
            XLOGI("    [OLD] gb=%p, handle=%p, w=%d, h=%d, s=%d",
                sg.get(), sg->handle,
                sg->width, sg->height, sg->stride);

            // reset old buffer data
            buf.release();
        }
        else
        {
            XLOGI("    [OLD] gb:NULL");
        }

        // re-create buffer
        sg = new GraphicBuffer(w, h, BUFFER_FORMAT, BUFFER_USAGE);
        if ((sg == NULL) || (sg->handle == NULL))
        {
            XLOGE("    create GraphicBuffer FAILED");
            return BAD_VALUE;
        }
        else
        {
            XLOGI("    [NEW] gb=%p, handle=%p, w=%d, h=%d, s=%d",
                sg.get(), sg->handle,
                sg->width, sg->height, sg->stride);
        }        
        buf.mGraphicBuffer = sg;
    }

    // try to create eglimage will not always create ok, for current qualification
    // it still OK since we do not always need it for egl env always
    if (EGL_NO_IMAGE_KHR == buf.mEglImage)
    {
        createImage(&buf);
    }

    // FREE -> DEQUEUED
    buf.mState = BufferSlot::DEQUEUED;
    *outBuf = &(buf);

    return OK;
}


status_t GEMQueue::queueBuffer(BufferSlot *buffer)
{
    status_t err = validateBuffer(buffer, BufferSlot::DEQUEUED);
    if (OK != err) {
        return err;
    }

    Mutex::Autolock l(mMutex);

    // DEQUEUED -> QUEUED
    mQueue.push_back(buffer->mIndex);
    buffer->mState = BufferSlot::QUEUED;
    buffer->mFrameNumber = (++mFrameCounter);

    mDequeueCondition.broadcast();
    return OK;
}


status_t GEMQueue::cancelBuffer(BufferSlot* buffer)
{
    status_t err = validateBuffer(buffer, BufferSlot::DEQUEUED);
    if (OK != err) {
        return err;
    }

    AutoMutex l(mMutex);

    // DEQUEUED -> FREE
    buffer->mState = BufferSlot::FREE;
    buffer->mFrameNumber = 0;

    mDequeueCondition.broadcast();
    return OK;
}


status_t GEMQueue::acquireBuffer(BufferSlot** buffer)
{
    AutoMutex l(mMutex);

    if (!mQueue.empty())
    {
        Fifo::iterator front(mQueue.begin());
        BufferSlot& buf = mSlots[*front];

        // QUEDED -> ACQUIRED
        buf.mState = BufferSlot::ACQUIRED;
        *buffer = &(buf);

        mQueue.erase(front);
        mDequeueCondition.broadcast();
    }
    else
    {
        return NO_BUFFER_AVAILABLE;
    }

    return OK;
}


status_t GEMQueue::releaseBuffer(BufferSlot* buffer)
{
    status_t err = validateBuffer(buffer, BufferSlot::ACQUIRED);
    if (OK != err) {
        return err;
    }

    AutoMutex l(mMutex);

    // ACQUIRED -> FREE
    buffer->mState = BufferSlot::FREE;

    mDequeueCondition.broadcast();
    return OK;
}


status_t GEMQueue::setGEMThread(const sp<GEMThread>& thread)
{
    AutoMutex l(mMutex);

    mThread = thread;

    return OK;
}


status_t GEMQueue::dump()
{
    return OK;
}


status_t GEMQueue::validateBuffer(const BufferSlot* buffer, BufferSlot::State state)
{
    if (NULL == buffer)
    {
        return BAD_VALUE;
    }

    const int idx = buffer->mIndex;

    if ((INVALID_BUFFER_SLOT == idx) || (NUM_MAX_SLOTS <= idx))
    {
        XLOGE("[%s] index out of range [0, %d]: %d",
            __FUNCTION__, NUM_MAX_SLOTS, idx);
        return BAD_VALUE;
    }
    else if (state != buffer->mState)
    {
        XLOGE("[%s] buffer %d state is diff from expected [%d]: %d)",
            __FUNCTION__, idx, state, buffer->mState);
        return INVALID_OPERATION;
    }

    return OK;
}


status_t GEMQueue::createImage(BufferSlot* buffer)
{
    EGLDisplay d = EGL_NO_DISPLAY;

    // try to get a valid eglDisplay from worker thread first
    if ((mThread != NULL) )
    {
        d = mThread->getEGL()->mDisplay;
        if (EGL_NO_SURFACE != d)
        {
            XLOGI("    (got display from GEMThread)");
        }
    }

    // try to get a valid eglDisplay from process
    if (EGL_NO_DISPLAY == d)
    {
        d = eglGetCurrentDisplay();
        if (EGL_NO_SURFACE != d)
        {
            XLOGI("    (got display from process env)");
        }
    }

    // cannot get valid display in current process, return
    if (EGL_NO_DISPLAY == d)
    {
        return INVALID_OPERATION;
    }

    // reset to display newly got
    buffer->mEglDisplay = d;

    // reset image
    EGLint attrs[] =
    {
        EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
        EGL_NONE,
    };
    EGLImageKHR image = eglCreateImageKHR(
        buffer->mEglDisplay,
        EGL_NO_CONTEXT,
        EGL_NATIVE_BUFFER_ANDROID,
        buffer->mGraphicBuffer->getNativeBuffer(),
        attrs);

    // check if image created
    if (image == EGL_NO_IMAGE_KHR)
    {
        EGLint error = eglGetError();
        XLOGE("    => error creating EGLImage: %#x, dpy=%d", error, buffer->mEglDisplay);
        return INVALID_OPERATION;
    }
    else
    {
        buffer->mEglImage = image;
        XLOGD("    => EGLImage: dpy=%p, img=%p", buffer->mEglDisplay, buffer->mEglImage);
    }

    return OK;
}


}; // namespace android
