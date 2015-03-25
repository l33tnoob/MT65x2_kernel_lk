#include <utils/Trace.h>
#include <cutils/xlog.h>

#include <mediatek/gem/thread.h>


#define INIT_FRAMES  (3)
#define INIT_TIMEOUT ((uint64_t)1000 * 1000 * 1000)
#define WAIT_TIMEOUT ((uint64_t)  50 * 1000 * 1000)


namespace android
{


// for thread naming count
uint32_t GEMThread::sCount = 0;


const char *GEMThread::mapRunModeToString(RUN_MODE mode)
{
    static const char *strings[] =
    {
        "SYNC",
        "ASYNC",
        "ASYNC_GL",
    };
    return strings[mode];
}


const char *GEMThread::mapStateToString(STATE state)
{
    static const char *strings[] =
    {
        "NONE",
        "CREATE_EGL",
        "DESTROY_EGL",
        "DESTORY_PROGRAM",
        "RUN",
        "DESTROY",
    };
    return strings[state];
}


GEMThread::GEMThread()
    : mRunMode(RUN_MODE_SYNC)
    , mState(STATE_NONE)
    , mCount(0)
    , mProgram(NULL)
    , mAdapter(NULL)
{
    sCount += 1;
    XLOGI("[%s] %p (sCount:%d)", __func__, this, sCount);

    mName = String8::format("GEMThread #%d", sCount);
}


GEMThread::~GEMThread()
{    
    sCount -= 1;
    XLOGI("[%s] %p (sCount:%d)", __func__, this, sCount);
}


status_t GEMThread::createEGLLocked()
{
    ATRACE_CALL();

    // check if already created
    EGLInfo *i = &mEGLInfo;
    if ((EGL_NO_DISPLAY != i->mDisplay) &&
        (EGL_NO_SURFACE != i->mSurface) &&
        (EGL_NO_CONTEXT != i->mContext))
    {
        return INVALID_OPERATION;
    }

    XLOGI("[%s] %p", __func__, this);

    // start to create pbuffer context
    // config surface pixel format
    const EGLint config_attribs[] =
    {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_NONE
    };

    // for the dummy pbuffer
    const EGLint pbuffer_attribs[] =
    {                   
        EGL_WIDTH, 1,                           // just set to min size
        EGL_HEIGHT, 1,
        //EGL_LARGEST_PBUFFER, EGL_FALSE,
        //EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE,
        //EGL_TEXTURE_TARGET, EGL_NO_TEXTURE,
        EGL_NONE
    };

    const EGLint context_attribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    i->mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_NO_DISPLAY == i->mDisplay)
    {
        XLOGE("    * eglGetDisplay failed");
        return INVALID_OPERATION;
    }
    eglInitialize(i->mDisplay, &(i->mMajorVersion), &(i->mMinorVersion));

    EGLint num;
    EGLConfig eglConfig;
    eglChooseConfig(i->mDisplay, config_attribs, &eglConfig, 1, &num);  // get usable config
    if (0 == num)
    {
        XLOGE("    * eglChooseConfig failed");
        return INVALID_OPERATION;
    }
    
	i->mSurface = eglCreatePbufferSurface(i->mDisplay, eglConfig, pbuffer_attribs);
    if (EGL_NO_SURFACE == i->mSurface)
    {
       XLOGE("    * eglCreatePbufferSurface failed");
       return INVALID_OPERATION;
    }

	i->mContext = eglCreateContext(i->mDisplay, eglConfig, NULL, context_attribs);
    if (EGL_NO_CONTEXT == i->mContext)
    {
        XLOGE("    * eglCreateContext failed");
        return INVALID_OPERATION;
    }
    eglMakeCurrent(i->mDisplay, i->mSurface, i->mSurface, i->mContext);

    XLOGI("    * done create GL context with pBuffer");
    XLOGI("        eglDisplay: %p", i->mDisplay);
    XLOGI("        eglSurface: %p", i->mSurface);
    XLOGI("        eglContext: %p", i->mContext);

    return OK;
}


status_t GEMThread::destroyEGLLocked()
{
    ATRACE_CALL();

    // check if already destroyed
    EGLInfo *i = &mEGLInfo;
    if ((EGL_NO_DISPLAY == i->mDisplay) &&
        (EGL_NO_SURFACE == i->mSurface) &&
        (EGL_NO_CONTEXT == i->mContext))
    {
        return INVALID_OPERATION;
    }

    XLOGI("[%s] %p", __func__, this);
    XLOGI("    * egl env going to be destroyed");
    XLOGI("        eglDisplay: %p", i->mDisplay);
    XLOGI("        eglSurface: %p", i->mSurface);
    XLOGI("        eglContext: %p", i->mContext);

    // release with egl apis
    eglMakeCurrent(i->mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(i->mDisplay, i->mContext);
    eglDestroySurface(i->mDisplay, i->mSurface);
    eglTerminate(i->mDisplay);

    i->Reset();

    return OK;
}


void GEMThread::onFirstRef()
{
    Mutex::Autolock l(mLock);

    // init semaphore to wait for trigger event
    sem_init(&mEvent, 0, 0);

    // start thread
    run(mName);
}


status_t GEMThread::readyToRun()
{
    return OK;
}


bool GEMThread::threadLoop()
{
    sem_wait(&mEvent);

    Mutex::Autolock l(mLock);

    bool ret = true;
    switch(mState)
    {
    case STATE_CREATE_EGL:
        createEGLLocked();
        break;

    case STATE_DESTORY_EGL:
        destroyEGLLocked();
        break;

    case STATE_DESTORY_PROGRAM:
        delete mProgram;
        mProgram = NULL;
        break;

    case STATE_RUN:
        mProgram->process(mAdapter);
        break;

    case STATE_DESTORY:
        destroyEGLLocked();
        ret = false;
        break;

    default:
        XLOGW("[%s] do nothing for unexpected state(%s)",
            __func__, mapStateToString(mState));
    }
    mCondition.signal();

    return ret;
}


const GEMThread::EGLInfo* GEMThread::getEGL()
{
    Mutex::Autolock l(mLock);

    return &mEGLInfo;
}


status_t GEMThread::setProgram(GEMProgram *pgm, GEMProgram::Adapter *adpr)
{
    Mutex::Autolock l(mLock);

    if (mProgram != pgm)
    {
        mProgram = pgm;
        mCount = 0;
    }
    mAdapter = adpr;

    return OK;
}

status_t GEMThread::setRunMode(RUN_MODE to_mode)
{
    Mutex::Autolock l(mLock);

    mRunMode = to_mode;
    if (RUN_MODE_ASYNC_GL == mRunMode)
    {
        triggerLocked(STATE_CREATE_EGL);
    }

    return OK;
}


status_t GEMThread::trigger(STATE to_state)
{
    Mutex::Autolock l(mLock);

    return triggerLocked(to_state);
}


status_t GEMThread::triggerLocked(STATE to_state)
{
    ATRACE_CALL();

    uint64_t timeout = (mCount < INIT_FRAMES)
                     ? INIT_TIMEOUT
                     : WAIT_TIMEOUT;

    if (STATE_RUN == to_state)
    {
        if (RUN_MODE_SYNC == mRunMode)
        {
            // just as run in original thread instead of in thread loop
            mProgram->process(mAdapter);
        }
        else
        {
            mState = to_state;
            sem_post(&mEvent);
        
            // TODO: use fence sync to wait later instead this sync wait
            if (NO_ERROR != mCondition.waitRelative(mLock, timeout))
            {
                XLOGW("[%s] wait sync timeout for state:%s, time:%d(ms)",
                    __func__, mapStateToString(mState), timeout);
            }
        }
    }
    else
    {
        mState = to_state;
        sem_post(&mEvent);
    
        // need to wait sync in other thread loop state operations
        if (NO_ERROR != mCondition.waitRelative(mLock, timeout))
        {
            XLOGW("[%s] wait sync timeout for state:%s, time:%d(ms)",
                __func__, mapStateToString(mState), timeout);
        }
    }
    mCount += 1;

    return OK;
}


}; // namespace android
