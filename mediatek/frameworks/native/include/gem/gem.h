#ifndef GEM_H
#define GEM_H

#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <utils/KeyedVector.h>

#include <ui/Rect.h>

// GEM utils
#include "queue.h"
#include "program.h"
#include "thread.h"


namespace android
{


// display infomation gives more hints for conversion
// often used when buffer is for SF::Layer
class DisplayLayerInfo
{
public:
    // buffer info
    Rect     mBufferCrop;
    uint32_t mBufferTransform;   // as rotation
    uint32_t mUsage;
    uint32_t mUsageEx;

    // layer info
    Rect     mLayerCrop;
    uint32_t mLayerTransform;    // as transform
    uint32_t mFlags;
    uint32_t mFlagsEx;

    // device info
    uint32_t mDisplayOrientation; // as orientaion

    // display info
    Rect     mDisplayRect;
    uint32_t mFinalTransform;

    // reserved space, and try to keep total size as 32 * 4bytes
    uint32_t mReserved[12];

    DisplayLayerInfo();
};


// main entry class
// GEM stands for Graphic Enhancement Module
class GEM : public LightRefBase<GEM>
{
public:
    // max programs will be cached in the pool
    enum { NUM_MAX_PROGRAMS = 3 };

    GEM();
    ~GEM();

    // get current using program
    GEMProgram* getProgram() const { return mProgram; }

    // set program to use
    status_t setProgram(const String8& libname, const String8& progname);

    // give the display info
    // for chance to do some optimization
    // return OK if value changed
    status_t setDisplayLayerInfo(const DisplayLayerInfo* info);

    // input to given the input buffer, and output for conversion result
    // check return status if sucess or not
    status_t process(const BufferSlot* input, BufferSlot** output);

private:
    // thread safe
    Mutex mLock;

    // display info for chance of look ahead optimization
    DisplayLayerInfo mDisplayLayerInfo;

    // GEM worker thread
    sp<GEMThread> mThread;

    // buffer queue to control buffer and processed result
    sp<GEMQueue> mQueue;

    // current program
    GEMProgram* mProgram;

    // pool for programs, sould keep size no more than MAX_KEEP_PROGRAMS
    DefaultKeyedVector<String8, GEMProgram*> mProgramPool;

    // current buffer slot
    BufferSlot* mAcquiredBufferSlot;

    // data adpater to program
    GEMProgram::Adapter mAdapter;

    // to destroy given program in worker thread
    status_t destroyProgramByThread(GEMProgram *pgm);
};


} // namespace android
#endif
