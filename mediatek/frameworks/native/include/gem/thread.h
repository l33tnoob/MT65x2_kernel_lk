#ifndef GEM_THREAD_H
#define GEM_THREAD_H

#include <semaphore.h>
#include <utils/Thread.h>
#include <utils/String8.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

// GEM utils
#include "program.h"


namespace android
{

// thread impl as GEM worker and egl env provider
class GEMThread : public Thread
{
public:
    struct EGLInfo
    {
        EGLDisplay mDisplay;
        EGLSurface mSurface;
        EGLContext mContext;
        EGLint     mMajorVersion;
        EGLint     mMinorVersion;

        void Reset()
        {
            mDisplay = EGL_NO_DISPLAY;
            mSurface = EGL_NO_SURFACE;
            mContext = EGL_NO_CONTEXT;
            mMajorVersion = mMinorVersion = -1;
        }

        EGLInfo()
        {
            Reset();        
        }
    };

    enum RUN_MODE
    {
        RUN_MODE_SYNC = 0,
        RUN_MODE_ASYNC,
        RUN_MODE_ASYNC_GL,
    };
    inline const char *mapRunModeToString(RUN_MODE mode);

    enum STATE
    {
        STATE_NONE = 0,
        STATE_CREATE_EGL,
        STATE_DESTORY_EGL,
        STATE_DESTORY_PROGRAM,
        STATE_RUN,
        STATE_DESTORY,
    };
    inline const char *mapStateToString(STATE state);

private:
    // for thread naming count
    static uint32_t sCount;

    // utils for making thread control in orders
    sem_t     mEvent;
    Mutex     mLock;
    Condition mCondition;

    // class internal info
    String8  mName;
    RUN_MODE mRunMode;
    STATE    mState;
    uint32_t mCount;

    // for algo program process
    GEMProgram          *mProgram;
    GEMProgram::Adapter *mAdapter;

    // for using egl
    EGLInfo mEGLInfo;

    // egl realted operation
    status_t createEGLLocked();
    status_t destroyEGLLocked();

    // trigger in locked region
    status_t triggerLocked(STATE to_state);

public:
    GEMThread();
    virtual ~GEMThread();

    // thread interface
    virtual void     onFirstRef();
    virtual status_t readyToRun();
    virtual bool     threadLoop();
    virtual void     requestExit() { trigger(STATE_DESTORY); };

    // provide egl env info
    const EGLInfo *getEGL();

    // for algo program setup
    status_t setProgram(GEMProgram *pgm, GEMProgram::Adapter *adpr);

    // for task handling behavior
    status_t setRunMode(RUN_MODE to_mode);

    // trigger thread loop to run the given state
    status_t trigger(STATE to_state);
};


}; // namespace android
#endif
