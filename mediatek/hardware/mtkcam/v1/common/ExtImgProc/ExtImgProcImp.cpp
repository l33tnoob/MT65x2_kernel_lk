#define LOG_TAG "MtkCam/ExtImgProc"
//
#include <utils/threads.h>
//
using namespace android;
//
#include <mtkcam/Log.h>
#include <mtkcam/BuiltinTypes.h>
//
#include <mtkcam/v1/ExtImgProc/IExtImgProc.h>
#include <mtkcam/v1/ExtImgProc/ExtImgProc.h>
#include <ExtImgProcImp.h>
//-----------------------------------------------------------------------------
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s]"             fmt, ::gettid(), __FUNCTION__,           ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s]WRN(%5d):"    fmt, ::gettid(), __FUNCTION__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s]ERR(%5d):"    fmt, ::gettid(), __FUNCTION__, __LINE__, ##arg)
//
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }
//
#define FUNCTION_NAME               MY_LOGD("");
#define FUNCTION_IN                 MY_LOGD("+")
#define FUNCTION_OUT                MY_LOGD("-")
//-----------------------------------------------------------------------------
ExtImgProc*
ExtImgProc::
createInstance(void)
{
    return ExtImgProcImp::getInstance();
}
//-----------------------------------------------------------------------------
ExtImgProc*
ExtImgProcImp::
getInstance(void)
{
    static ExtImgProcImp Singleton;
    return &Singleton;
}
//-----------------------------------------------------------------------------
void
ExtImgProcImp::
destroyInstance(void) 
{
}
//-----------------------------------------------------------------------------
ExtImgProcImp::
ExtImgProcImp()
{
    FUNCTION_NAME;
    //Set which img buf you want to process.
    //For example: mImgMask = BufType_Display|BufType_Record;
    mImgMask = 0;
}
//-----------------------------------------------------------------------------
ExtImgProcImp::
~ExtImgProcImp()
{
    FUNCTION_NAME;
}
//-----------------------------------------------------------------------------
MBOOL
ExtImgProcImp::
init(void)
{
    MBOOL Result = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUser == 0)
    {
        MY_LOGD("First user(%d)",mUser);
    }
    else
    {
        MY_LOGD("More user(%d)",mUser);
        android_atomic_inc(&mUser);
        goto EXIT;
    }
    //Add init code
    //[BEGIN]

    //[END]
    //
    android_atomic_inc(&mUser);
    //
    EXIT:
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL
ExtImgProcImp::
uninit(void)
{
    MBOOL Result = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUser <= 0)
    {
        MY_LOGW("No user(%d)",mUser);
        goto EXIT;
    }
    //
    android_atomic_dec(&mUser);
    //
    if(mUser == 0)
    {
        MY_LOGD("Last user(%d)",mUser);
    }
    else
    {
        MY_LOGD("More user(%d)",mUser);
        goto EXIT;
    }
    //Add uninit code
    //[BEGIN]

    //[END]
    EXIT:
    return Result;
}
//-----------------------------------------------------------------------------
MUINT32
ExtImgProcImp::
getImgMask()
{
    Mutex::Autolock lock(mLock);
    //
    return mImgMask;
}
//-----------------------------------------------------------------------------
MBOOL
ExtImgProcImp::
doImgProc(ImgInfo& img)
{
    MBOOL Result = MTRUE;
    //
    {
        Mutex::Autolock lock(mLock);
        //
        if(mUser <= 0)
        {
            MY_LOGW("No user");
            Result = MFALSE;
            goto EXIT;
        }
    }
    //Doubel check to avoid user send buffer here but they don't need extra image process.
    if((mImgMask & img.bufType) != img.bufType)
    {
        Result = MFALSE;
        goto EXIT;
    }
    //
    MY_LOGD("Img(0x%08X,%s,%dx%d,%d/%d/%d,0x%08X,%d)",
            img.bufType,
            img.format,
            img.width,
            img.height,
            img.stride[0],
            img.stride[1],
            img.stride[2],
            img.virtAddr,
            img.bufSize);
    //
    //Add image process code
    switch(img.bufType)
    {
        case BufType_Display:
        {
            //[BEGIN]

            //[END]
            break;
        }
        case BufType_PreviewCB:
        {
            //[BEGIN]

            //[END]
            break;
        }
        case BufType_Record:
        {
            //[BEGIN]

            //[END]
            break;
        }
        default:
        {
            MY_LOGW("Unknown bufType(0x%08X)",img.bufType);
            break;
        }
    }
    //
    EXIT:
    return Result;
}


