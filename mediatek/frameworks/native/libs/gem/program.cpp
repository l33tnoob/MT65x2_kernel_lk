#include <dlfcn.h>

#include <utils/Trace.h>
#include <cutils/xlog.h>

#include <mediatek/gem/program.h>


namespace android
{


String8 makeFullProgramName(const String8& libname, const String8& progname)
{
    return libname + " : " + progname;
}


GEMProgram::GEMProgram()
    : mName("UN-INITED")
    , mLib(NULL)
    , mData(NULL)
    , mInit(NULL)
    , mSetvalue(NULL) 
    , mGetvalue(NULL) 
    , mProcess(NULL) 
    , mReset(NULL) 
    , mTeardown(NULL) 
{
    XLOGI("[%s] %p", __FUNCTION__, this);
}


GEMProgram::~GEMProgram()
{
    XLOGI("[%s] %p [%s]", __FUNCTION__, this, mName.string());

    teardown();
    if (NULL != mLib)
    {
        dlclose(mLib);
    }
}


status_t GEMProgram::openProgram(const String8& libname, const String8& progname)
{
    // open lib first
    if (NULL == mLib)
    {
        mLib = dlopen(libname.string(), RTLD_NOW);
        if (NULL == mLib)
        {
            XLOGE("[%s] Error opening lib: %s: %s",
                __FUNCTION__, libname.string(), dlerror());
            return BAD_VALUE;
        }
    }
    XLOGI("[%s] open lib: %s [OK] %p",
        __FUNCTION__, libname.string(), mLib);

    String8 symname;

    // hook init function
    symname = progname + "_init";
    mInit = reinterpret_cast<initFP>(dlsym(mLib, symname.string()));
    (NULL != mInit)
        ? XLOGI("    hook init() [OK] %p", mInit)
        : XLOGW("    hook init() [FAILED]");

    // hook setvalue function
    symname = progname + "_setvalue";
    mSetvalue = reinterpret_cast<setvalueFP>(dlsym(mLib, symname.string()));
    (NULL != mSetvalue)
        ? XLOGI("    hook setvalue() [OK] %p", mSetvalue)
        : XLOGW("    hook setvalue() [FAILED]");

    // hook getvalue function
    symname = progname + "_getvalue";
    mGetvalue = reinterpret_cast<getvalueFP>(dlsym(mLib, symname.string()));
    (NULL != mGetvalue)
        ? XLOGI("    hook getvalue() [OK] %p", mGetvalue)
        : XLOGW("    hook getvalue() [FAILED]");

    // hook process function
    symname = progname + "_process";
    mProcess = reinterpret_cast<processFP>(dlsym(mLib, symname.string()));
    (NULL != mProcess)
        ? XLOGI("    hook process() [OK] %p", mProcess)
        : XLOGW("    hook process() [FAILED]");

    // hook reset function
    symname = progname + "_reset";
    mReset = reinterpret_cast<resetFP>(dlsym(mLib, symname.string()));
    (NULL != mReset)
        ? XLOGI("    hook reset() [OK] %p", mReset)
        : XLOGW("    hook reset() [FAILED]");

    // hook reset function
    symname = progname + "_teardown";
    mTeardown = reinterpret_cast<teardownFP>(dlsym(mLib, symname.string()));
    (NULL != mTeardown)
        ? XLOGI("    hook teardown() [OK] %p", mTeardown)
        : XLOGW("    hook teardown() [FAILED]");

    // check must-have functions
    if ((NULL == mInit) || (NULL == mProcess) || (NULL == mTeardown))
    {
        XLOGE("    FAILED: %p lost impl", this);
        return BAD_VALUE;
    }

    // set full program symbol name
    mName = makeFullProgramName(libname, progname);
    XLOGI("    SUCCESS: %p hooked as [%s]", this, mName.string());

    return OK;
}


status_t GEMProgram::init()
{
    if ((NULL != mInit) && (NULL == mData))
    {
        mInit(&mData);
        return OK;
    }
    return BAD_VALUE;
}


status_t GEMProgram::setvalue(const String8& key, const String8& value)
{
    if ((NULL != mSetvalue) && (NULL != mData))
    {
        mSetvalue(key.string(), value.string(), mData);
        return OK;
    }
    return BAD_VALUE;
}


String8 GEMProgram::getvalue(const String8& key)
{
    if ((NULL != mGetvalue) && (NULL != mData))
    {
        static const int buffer_size = 32;
        char result[buffer_size];
        result[buffer_size - 1] = '\0';
        mGetvalue(key.string(), result, buffer_size, mData);
        return String8(result);
    }
    return String8();
}


status_t GEMProgram::process(const Adapter* adapter)
{
    ATRACE_CALL();

    if ((NULL != mProcess) && (NULL != mData))
    {
        int r = mProcess((const char**)&(adapter->mInputs[0]),
                         (const int*)&(adapter->mInputSizes[0]),
                         adapter->mInputs.size(),
                         (char*)(adapter->mOutput),
                         adapter->mOutputSize,
                         mData);
        return (0 != r ) ? OK : INVALID_OPERATION;
    }

    return BAD_VALUE;
}


status_t GEMProgram::reset()
{
    if ((NULL != mReset) && (NULL != mData))
    {
        mReset(mData);
        return OK;
    }
    return BAD_VALUE;
}


status_t GEMProgram::teardown()
{
    if ((NULL != mTeardown) && (NULL != mData))
    {
        mTeardown(mData);
        mData = NULL;
        return OK;
    }
    return BAD_VALUE;
}


}; // namespace android
