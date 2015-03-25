#include <utils/Trace.h>
#include <cutils/xlog.h>
#include <gui/IGraphicBufferProducer.h>
#include <private/gui/LayerState.h>
#include <Transform.h>

#include <mediatek/gem/gem.h>


namespace android
{


DisplayLayerInfo::DisplayLayerInfo()
    : mBufferCrop(0, 0)
    , mBufferTransform(Transform::ROT_INVALID)
    , mUsage(0x00000000)
    , mUsageEx(EXTRA_USAGE_INIT_VALUE)
    , mLayerCrop(0, 0)
    , mLayerTransform(Transform::ROT_INVALID)
    , mFlags(0x00000000)
    , mFlagsEx(layer_state_t::eExInitValue)
    , mDisplayOrientation(DisplayState::eOrientationDefault)
    , mDisplayRect(0, 0)
    , mFinalTransform(Transform::ROT_INVALID)
{
    for (uint32_t i = 0; i < (sizeof(mReserved) / sizeof(mReserved[0])); i++)
    {
        mReserved[i] = 0xdeadbeef;
    }
}


GEM::GEM()
    : mThread(NULL)
    , mQueue(NULL)
    , mProgram(NULL)
    , mProgramPool(NULL)
    , mAcquiredBufferSlot(NULL)
{
    // GEM internal init sequence should be
    // (thread) -> (queue) -> (program)
}


GEM::~GEM()
{
    // GEM internal deinit sequence should be
    // (program) -> (queue) -> (thread)

    Mutex::Autolock l(mLock);

    // delete program units
    for (uint32_t i = 0; i < mProgramPool.size(); i++)
    {
        destroyProgramByThread(mProgramPool[i]);
    }

    // releaes queue staff
    if (mQueue != NULL)
    {
        mQueue->releaseBuffer(mAcquiredBufferSlot);
        mQueue.clear();
    }

    // stop and leave the worker thread
    if (mThread != NULL)
    {
        mThread->requestExit();
        mThread.clear();
    }
}


status_t GEM::setProgram(const String8& libname, const String8& progname)
{
    Mutex::Autolock l(mLock);

    // use wanted program as current
    String8 name = makeFullProgramName(libname, progname);
    GEMProgram *p = mProgramPool.valueFor(name);
    if (NULL == p)
    {
        // try to hook a new program
        p = new GEMProgram();
        status_t err = p->openProgram(libname, progname);
        if (OK != err)
        {
            delete p;
            return err;
        }

        // prepare context
        p->init();

        // swap the new created program into pool
        if (NUM_MAX_PROGRAMS == mProgramPool.size())
        {
            destroyProgramByThread(mProgramPool[NUM_MAX_PROGRAMS - 1]);
            mProgramPool.removeItemsAt(NUM_MAX_PROGRAMS - 1);
        }
        mProgramPool.add(name, p);
    }

    // set current program and log if changed
    if (mProgram != p)
    {
        mProgram = p;

        XLOGI("[%s] %d program in pool", __FUNCTION__, mProgramPool.size());
        for (uint32_t i = 0; i < mProgramPool.size(); i++)
        {
            String8 s = String8::format(
                "    %s[%d] %s",
                (p == mProgramPool[i]) ? ">" : " ",
                i, mProgramPool[i]->getName().string());
            XLOGD("%s", s.string());
        }
    }

    // we are going to use program, make the worker thread and buffer queue ready
    if (mThread == NULL)
    {
        mThread = new GEMThread();
    }

    if (mQueue == NULL)
    {
        mQueue = new GEMQueue();
        mQueue->setGEMThread(mThread);
    }

    // get run mode given from program by getvalue, and set to the worker thread
    GEMThread::RUN_MODE m =
        (GEMThread::RUN_MODE)atoi(mProgram->getvalue(String8("RunMode")).string());
    mThread->setRunMode(m);
    if (GEMThread::RUN_MODE_ASYNC_GL == m)
    {
        // the thread also have to provide egl env to program
        mProgram->setvalue(
            String8("EGLInfo"),
            String8::format("%d", (uint32_t)mThread->getEGL()));
    }

    return OK;
}


// give the display info, return OK if info updated
status_t GEM::setDisplayLayerInfo(const DisplayLayerInfo* info)
{
    Mutex::Autolock l(mLock);

    status_t ret = INVALID_OPERATION;

    // check if info changed
    if (0 != memcmp(info, &mDisplayLayerInfo, sizeof(DisplayLayerInfo)))
    {
        mDisplayLayerInfo = *info;
        ret = OK;
    }

    return ret;
}


// do the conversion work
status_t GEM::process(const BufferSlot* input, BufferSlot** output)
{
    Mutex::Autolock l(mLock);

    if (NULL == mProgram) {
        XLOGE("[%s] mProgram is NULL", __FUNCTION__);
    }

    uint32_t w, h;
    if (true == mDisplayLayerInfo.mDisplayRect.isEmpty())
    {
        // set to as original buffer
        w = input->mGraphicBuffer->width;
        h = input->mGraphicBuffer->height;
    }
    else
    {
        // set to display required
        w = mDisplayLayerInfo.mDisplayRect.width();
        h = mDisplayLayerInfo.mDisplayRect.height();
    }
    
    // buffer data process unit
    BufferSlot *buf;

    // get dst buffer from queue for the program to process
    mQueue->dequeueBuffer(&buf, w, h);
    {
        // set program and data adapter to thread first
        mThread->setProgram(mProgram, &mAdapter);

        // default setup basic display info
        mProgram->setvalue(
            String8("DisplayLayerInfo"),
            String8::format("%d", (uint32_t)&mDisplayLayerInfo));

        // clear old
        mAdapter.mInputs.clear();
        mAdapter.mInputSizes.clear();

        // setup input
        mAdapter.mInputs.add(input);
        mAdapter.mInputSizes.add(MQBS);

        // setup output
        mAdapter.mOutput = buf;
        mAdapter.mOutputSize = MQBS;

        // start process and check result
        status_t err = mThread->trigger(GEMThread::STATE_RUN);
        if (OK != err)
        {
            XLOGE("[%s] buffer process failed, err:%s", __FUNCTION__, strerror(err));
            mQueue->cancelBuffer(buf);
            return err;
        }
    }
    mQueue->queueBuffer(buf);
    
    // acquire the new buffer and release the previous
    mQueue->acquireBuffer(&buf);
    mQueue->releaseBuffer(mAcquiredBufferSlot);

    // set current buffer to the new acquired one
    *output = mAcquiredBufferSlot = buf;

    return OK;
}


status_t GEM::destroyProgramByThread(GEMProgram *pgm)
{
    mThread->setProgram(pgm, NULL);
    mThread->trigger(GEMThread::STATE_DESTORY_PROGRAM);

    return OK;
}


}; // namespace android
