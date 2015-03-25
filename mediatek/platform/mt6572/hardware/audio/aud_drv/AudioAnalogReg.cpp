#include "AudioAnalogReg.h"
#include <stdio.h>

#ifndef ANDROID_DEFAULT_CODE
#include <cutils/xlog.h>
#ifdef ALOGE
#undef ALOGE
#endif
#ifdef ALOGW
#undef ALOGW
#endif ALOGI
#undef ALOGI
#ifdef ALOGD
#undef ALOGD
#endif
#ifdef ALOGV
#undef ALOGV
#endif
#define ALOGE XLOGE
#define ALOGW XLOGW
#define ALOGI XLOGI
#define ALOGD XLOGD
#define ALOGV XLOGV
#else
#include <utils/Log.h>
#endif

#define LOG_TAG "AudioAnalogReg"

namespace android
{

AudioAnalogReg *AudioAnalogReg::UniqueAnalogRegInstance = 0;

AudioAnalogReg *AudioAnalogReg::getInstance()
{
    if (UniqueAnalogRegInstance == 0)
    {
        printf("+UniqueAfeRegInstance\n");
        UniqueAnalogRegInstance = new AudioAnalogReg();
        printf("-UniqueAfeRegInstance\n");
    }
    return UniqueAnalogRegInstance;
}

bool AudioAnalogReg::CheckAnaRegRange(uint32 offset)
{
    return true;
}

AudioAnalogReg::AudioAnalogReg()
{
    ALOGD("AudioAnalogReg contructor\n");
    mFd = 0;
    // here open audio hardware for register setting
    mFd = ::open(kAudioDeviceName, O_RDWR);
    if (mFd == 0)
    {
        ALOGE("AudioAfeReg open mFd fail");
    }
}

status_t AudioAnalogReg::SetAnalogReg(uint32 offset, uint32 value, uint32 mask)
{
    Register_Control Reg_Data;
    // here need to set reg to analog part
    if (!CheckAnaRegRange(offset))
    {
        return INVALID_OPERATION;
    }
    Reg_Data.offset = offset;
    Reg_Data.value = value;
    Reg_Data.mask = mask | 0xffff0000;
    ALOGV("SetAnalogReg offet = 0x%x value = 0x%x mask = 0x%x", offset, value, Reg_Data.mask);
    ::ioctl(mFd, SET_ANAAFE_REG, &Reg_Data);
    return NO_ERROR;
}

uint32_t AudioAnalogReg::GetAnalogReg(uint32 offset)
{
    // here need to get afe ref base on address
    Register_Control Reg_Data;
    Reg_Data.offset = offset;
    Reg_Data.value = 0;
    Reg_Data.mask = 0xffffffff;
    ::ioctl(mFd, GET_ANAAFE_REG, &Reg_Data);
    ALOGV("GetAnalogReg offet = 0x%x value = 0x%x", offset, Reg_Data.value);
    return Reg_Data.value;
}

}