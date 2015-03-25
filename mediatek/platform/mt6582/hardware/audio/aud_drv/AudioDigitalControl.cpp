#include "AudioDigitalControl.h"
#include "AudioDigitalType.h"
#include "AudioInterConnection.h"
#include "AudioAfeReg.h"
#include "audio_custom_exp.h"
#include <DfoDefines.h>

#define LOG_TAG "AudioDigitalControl"
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

namespace android
{

AudioDigitalControl *AudioDigitalControl::UniqueDigitalInstance = 0;

AudioDigitalControl *AudioDigitalControl::getInstance()
{
    if (UniqueDigitalInstance == 0)
    {
        ALOGD("+UniqueDigitalInstance\n");
        UniqueDigitalInstance = new AudioDigitalControl();
        ALOGD("-UniqueDigitalInstance\n");
    }
    return UniqueDigitalInstance;
}

AudioDigitalControl::AudioDigitalControl()
{
    ALOGD("+%s(), contructor\n", __FUNCTION__);
    mAfeReg = NULL;
    mFd = 0;
    mAfeReg = AudioAfeReg::getInstance();
    if (!mAfeReg)
    {
        ALOGW("mAfeReg init fail ! \n");
    }
    mFd = mAfeReg->GetAfeFd();
    if (mFd == 0)
    {
        ALOGW("mFd  AudioDigitalControl = %d ", mFd);
    }

    for (int i = 0; i < AudioDigitalType::NUM_OF_IRQ_MODE ; i++)
    {
        memset((void *)&mAudioMcuMode[i], 0, sizeof(mAudioMcuMode));
    }
    memset((void *)&mMrgIf, 0, sizeof(AudioMrgIf));
    mMrgIf.Mrg_I2S_SampleRate = AudioMrgIf::MRFIF_I2S_32K;

    // init default for stream attribute
    for (int i = 0; i < AudioDigitalType::NUM_OF_MEM_INTERFACE ; i++)
    {
        mAudioMEMIF[i].mState = AudioMEMIFAttribute::STATE_FREE;
        mAudioMEMIF[i].mMemoryInterFaceType = i;
        mAudioMEMIF[i].mChannels = 0;
        mAudioMEMIF[i].mInterruptSample = 0;
        mAudioMEMIF[i].mBufferSize = 0;
        mAudioMEMIF[i].mMonoSel = -1 ;
        mAudioMEMIF[i].mdupwrite = -1 ;
        mAudioMEMIF[i].mFetchFormatPerSample = AudioMEMIFAttribute::AFE_WLEN_16_BIT;
    }
    mAudioMEMIF[AudioDigitalType::MEM_DL1].mDirection = AudioDigitalType::DIRECTION_OUTPUT;
    mAudioMEMIF[AudioDigitalType::MEM_DL2].mDirection = AudioDigitalType::DIRECTION_OUTPUT;
#if 0
    mAudioMEMIF[AudioDigitalType::MEM_DAI].mDirection = AudioDigitalType::DIRECTION_INPUT;
#endif
    mAudioMEMIF[AudioDigitalType::MEM_AWB].mDirection = AudioDigitalType::DIRECTION_INPUT;
    mAudioMEMIF[AudioDigitalType::MEM_MOD_DAI].mDirection = AudioDigitalType::DIRECTION_INPUT;
    mAudioMEMIF[AudioDigitalType::MEM_VUL].mDirection = AudioDigitalType::DIRECTION_INPUT;

    for (int i = 0; i < AudioDigitalType::NUM_OF_DIGITAL_BLOCK; i++)
    {
        mAudioDigitalBlock[i] = 0;
    }

    mMicInverse = false;

    // init digital control
    mInterConnectionInstance = new AudioInterConnection();

    mAfeReg->SetAfeReg(FPGA_CFG0, 0x00000007, 0xffffffff);   // hopping 32m, MCLK : 3.072M
    mAfeReg->SetAfeReg(FPGA_CFG1, 0x00000000, 0xffffffff);   // hopping 32m, MCLK : 3.072M
    mAfeReg->SetAfeReg(FPGA_CFG2, 0x0300F872, 0xffffffff);   // hopping 32m, MCLK : 3.072M
    mAfeReg->SetAfeReg(FPGA_CFG3, 0x9331802F, 0xffffffff);   // hopping 32m, MCLK : 3.072M
    mUseI2SADCInStatus = false;
    // test loop to test if there is someting wrong
    /*
    for(int i=0 ; i < AudioDigitalType::Num_Input ; i++){
        for(int j=0 ; j < AudioDigitalType::Num_Output ; j++){
            mInterConnectionInstance->SetinputConnection(AudioDigitalType::Connection,(AudioDigitalType::InterConnectionInput)i,(AudioDigitalType::InterConnectionOutput)j);
        }
    }
    mInterConnectionInstance->dump();

    for(int i=0 ; i < AudioDigitalType::Num_Input ; i++){
        for(int j=0 ; j < AudioDigitalType::Num_Output ; j++){
            mInterConnectionInstance->SetinputConnection(AudioDigitalType::DisConnect,(AudioDigitalType::InterConnectionInput)i,(AudioDigitalType::InterConnectionOutput)j);
        }
    }

    mInterConnectionInstance->dump();
    for(int i=0 ; i < AudioDigitalType::Num_Input ; i++){
        for(int j=0 ; j < AudioDigitalType::Num_Output ; j++){
            mInterConnectionInstance->SetinputConnection(AudioDigitalType::ConnectionShift,(AudioDigitalType::InterConnectionInput)i,(AudioDigitalType::InterConnectionOutput)j);
        }
    }
    mInterConnectionInstance->dump();
    for(int i=0 ; i < AudioDigitalType::Num_Input ; i++){
        for(int j=0 ; j < AudioDigitalType::Num_Output ; j++){
            mInterConnectionInstance->SetinputConnection(AudioDigitalType::DisConnect,(AudioDigitalType::InterConnectionInput)i,(AudioDigitalType::InterConnectionOutput)j);
        }
    }
    mInterConnectionInstance->dump();
    */

    // side tone filter
    mSideToneFilterOn = false;

}

status_t AudioDigitalControl::InitCheck()
{
    ALOGD("+%s()\n", __FUNCTION__);
    return NO_ERROR;
}

status_t AudioDigitalControl::SetMemBufferSize(uint32 InterfaceType, uint32 BufferSize)
{
    ALOGV("+%s(), InterfaceType = %d, BufferSize = %d\n", __FUNCTION__, InterfaceType, BufferSize);
    // only State Free can
    if (mAudioMEMIF[InterfaceType].mState == AudioMEMIFAttribute::STATE_FREE)
    {
        mAudioMEMIF[InterfaceType].mBufferSize = BufferSize;
    }
    else if (mAudioMEMIF[InterfaceType].mState == AudioMEMIFAttribute::STATE_FREE)
    {
        ALOGD("MemType = %d mState = %d\n", InterfaceType, mAudioMEMIF[InterfaceType].mState);
        mAudioMEMIF[InterfaceType].mBufferSize = BufferSize;
    }
    else
    {
        // State is executing , cannot set
        ALOGD("MemType = %d mState = %d\n", InterfaceType, mAudioMEMIF[InterfaceType].mState);
        return INVALID_OPERATION;
    }
    return NO_ERROR;
}

uint32 AudioDigitalControl::GetMemBufferSize(uint32 InterfaceType)
{
    ALOGV("+%s(), InterfaceType = %d\n", __FUNCTION__, InterfaceType);
    return mAudioMEMIF[InterfaceType].mBufferSize;
}

status_t AudioDigitalControl::AllocateMemBufferSize(uint32 InterfaceType)
{
    // here to calocate buffer with audio hardware base on mem if
    ALOGD("+%s(), allocate buffer InterfaceType = %d, mBufferSize = %d \n", __FUNCTION__, InterfaceType, mAudioMEMIF[InterfaceType].mBufferSize);
    int ret = 0;
    if (mAudioMEMIF[InterfaceType].mBufferSize && mAudioMEMIF[InterfaceType].mState == AudioMEMIFAttribute::STATE_FREE)
    {
        // todo ::here to tell kernel drvier to do allocate memory
        switch (InterfaceType)
        {
            case AudioDigitalType::MEM_DL1:
            {
                ret = ::ioctl(mFd, ALLOCATE_MEMIF_DL1, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            case AudioDigitalType::MEM_DL2:
            {
                ret =::ioctl(mFd, ALLOCATE_MEMIF_DL2, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            case AudioDigitalType::MEM_AWB:
            {
                ret =::ioctl(mFd, ALLOCATE_MEMIF_AWB, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            case AudioDigitalType::MEM_VUL:
            {
                ret =::ioctl(mFd, ALLOCATE_MEMIF_ADC, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
#if 0
            case AudioDigitalType::MEM_DAI:
            {
                ret =::ioctl(mFd, ALLOCATE_MEMIF_DAI, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
#endif
            case AudioDigitalType::MEM_MOD_DAI:
            {
                ret =::ioctl(mFd, ALLOCATE_MEMIF_MODDAI, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            default:
                ALOGW("no such MEM interface");
                break;
        }
        mAudioMEMIF[InterfaceType].mState = AudioMEMIFAttribute::STATE_STANDBY;
    }
    ALOGD("allocate buffer done");
    return ret;
}

status_t AudioDigitalControl::FreeMemBufferSize(uint32 InterfaceType)
{
    // here to calocate buffer with audio hardware base on mem if
    ALOGD("+%s(), buffer InterfaceType = %d \n", __FUNCTION__, InterfaceType);
    int ret = 0;
    if (mAudioMEMIF[InterfaceType].mBufferSize && mAudioMEMIF[InterfaceType].mState == AudioMEMIFAttribute::STATE_STANDBY)
    {
        switch (InterfaceType)
        {
            case AudioDigitalType::MEM_DL1:
            {
                ret =::ioctl(mFd, FREE_MEMIF_DL1, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            case AudioDigitalType::MEM_DL2:
            {
                ret =::ioctl(mFd, FREE_MEMIF_DL2, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            case AudioDigitalType::MEM_AWB:
            {
                ret =::ioctl(mFd, FREE_MEMIF_AWB, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            case AudioDigitalType::MEM_VUL:
            {
                ret =::ioctl(mFd, FREE_MEMIF_ADC, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
#if 0
            case AudioDigitalType::MEM_DAI:
            {
                ret =::ioctl(mFd, FREE_MEMIF_DAI, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
#endif
            case AudioDigitalType::MEM_MOD_DAI:
            {
                ret = ::ioctl(mFd, FREE_MEMIF_MODDAI, mAudioMEMIF[InterfaceType].mBufferSize);
                break;
            }
            default:
                ALOGW("no such MEM interface");
                break;
        }
        mAudioMEMIF[InterfaceType].mState = AudioMEMIFAttribute::STATE_FREE;
        mAudioMEMIF[InterfaceType].mBufferSize = 0;
    }
    ALOGD("-%s()\n", __FUNCTION__);
    return ret;
}

AudioMEMIFAttribute::SAMPLINGRATE AudioDigitalControl::SampleRateTransform(uint32 SampleRate)
{
    ALOGD("+%s(), SampleRate = %d\n", __FUNCTION__, SampleRate);
    switch (SampleRate)
    {
        case 8000:
            return AudioMEMIFAttribute::AFE_8000HZ;
        case 11025:
            return AudioMEMIFAttribute::AFE_11025HZ;
        case 12000:
            return AudioMEMIFAttribute::AFE_12000HZ;
        case 16000:
            return AudioMEMIFAttribute::AFE_16000HZ;
        case 22050:
            return AudioMEMIFAttribute::AFE_22050HZ;
        case 24000:
            return AudioMEMIFAttribute::AFE_24000HZ;
        case 32000:
            return AudioMEMIFAttribute::AFE_32000HZ;
        case 44100:
            return AudioMEMIFAttribute::AFE_44100HZ;
        case 48000:
            return AudioMEMIFAttribute::AFE_48000HZ;
        default:
            ALOGE("SampleRateTransform no such samplerate matching SampleRate = %d", SampleRate);
            break;
    }
    return AudioMEMIFAttribute::AFE_44100HZ;
}

status_t AudioDigitalControl::SetMemIfFetchFormatPerSample(uint32 InterfaceType, AudioMEMIFAttribute::FETCHFORMATPERSAMPLE eFetchFormat)
{
    return INVALID_OPERATION;
}

AudioMEMIFAttribute::FETCHFORMATPERSAMPLE AudioDigitalControl::GetMemIfFetchFormatPerSample(uint32 InterfaceType)
{
    ALOGD("+%s(), InterfaceType = %d\n", __FUNCTION__, InterfaceType);
    return mAudioMEMIF[InterfaceType].mFetchFormatPerSample;
}


status_t AudioDigitalControl::SetMemIfSampleRate(uint32 InterfaceType, uint32 SampleRate)
{
    mAudioMEMIF[InterfaceType].mSampleRate = SampleRateTransform(SampleRate);
    ALOGD("+%s(), InterfaceType = %d, SampleRate = %d, mAudioMEMIF[InterfaceType].mSampleRate = %d\n"
          , __FUNCTION__, InterfaceType, SampleRate, mAudioMEMIF[InterfaceType].mSampleRate);
    switch (InterfaceType)
    {
        case AudioDigitalType::MEM_DL1:
        {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, mAudioMEMIF[InterfaceType].mSampleRate , 0x0000000f);
            break;
        }
        case AudioDigitalType::MEM_DL2:
        {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, mAudioMEMIF[InterfaceType].mSampleRate << 4 , 0x000000f0);
            break;
        }
        case AudioDigitalType::MEM_I2S:
        {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, mAudioMEMIF[InterfaceType].mSampleRate << 8 , 0x00000f00);
            break;
        }
        case AudioDigitalType::MEM_AWB:
        {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, mAudioMEMIF[InterfaceType].mSampleRate << 12, 0x0000f000);
            break;
        }
        case AudioDigitalType::MEM_VUL:
        {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, mAudioMEMIF[InterfaceType].mSampleRate << 16, 0x000f0000);
            break;
        }
#if 0
        case AudioDigitalType::MEM_DAI:
        {
            if (SampleRate == AudioMEMIFAttribute::AFE_8000HZ)
            {
                mAfeReg->SetAfeReg(AFE_DAC_CON1, 0 << 20 , 1 << 20);
            }
            else if (SampleRate == AudioMEMIFAttribute::AFE_16000HZ)
            {
                mAfeReg->SetAfeReg(AFE_DAC_CON1, 1 << 20 , 1 << 20);
            }
            else
            {
                return INVALID_OPERATION;
            }
            break;
        }
#endif
        case AudioDigitalType::MEM_MOD_DAI:
        {
            if (SampleRate == AudioMEMIFAttribute::AFE_8000HZ)
            {
                mAfeReg->SetAfeReg(AFE_DAC_CON1, 0 << 30, 1 << 30);
            }
            else if (SampleRate == AudioMEMIFAttribute::AFE_16000HZ)
            {
                mAfeReg->SetAfeReg(AFE_DAC_CON1, 1 << 30, 1 << 30);
            }
            else
            {
                return INVALID_OPERATION;
            }
            break;
        }
        default:
            return INVALID_OPERATION;
    }
    return NO_ERROR;
}

uint32 AudioDigitalControl::GetMemIfSampleRate(uint32 InterfaceType)
{
    ALOGD("+%s(), InterfaceType = %d\n", __FUNCTION__, InterfaceType);
    mAudioMEMIF[InterfaceType].mSampleRate;
    return NO_ERROR;
}

status_t AudioDigitalControl::SetMemIfChannelCount(uint32 InterfaceType, uint32 Channels)
{
    ALOGD("+%s(), InterfaceType = %d, Channels = %d\n", __FUNCTION__, InterfaceType, Channels);
    mAudioMEMIF[InterfaceType].mChannels = Channels;
    return NO_ERROR;
}

uint32 AudioDigitalControl::GetMemIfChannelCount(uint32 InterfaceType)
{
    ALOGD("+%s(), InterfaceType = %d\n", __FUNCTION__, InterfaceType);
    int MemType = (int)InterfaceType;
    return mAudioMEMIF[MemType].mChannels;
}

status_t AudioDigitalControl::SetMemIfEnable(uint32 InterfaceType, uint32 State)
{
    ALOGD("+%s(), InterfaceType %d, State %d\n", __FUNCTION__, InterfaceType, State);
    ALOGD("state from %d ==> %d", mAudioMEMIF[InterfaceType].mState, State);
    if (mAudioMEMIF[InterfaceType].mState == AudioMEMIFAttribute::STATE_EXECUTING && State == AudioMEMIFAttribute::STATE_STANDBY)
    {
        //todo disable momory interface
        SetMemoryPathEnable(InterfaceType , 0);

    }
    else if (mAudioMEMIF[InterfaceType].mState == AudioMEMIFAttribute::STATE_STANDBY && State == AudioMEMIFAttribute::STATE_EXECUTING)
    {
        //todo Enable momory interface.
        SetMemoryPathEnable(InterfaceType , 1);
    }
    mAudioMEMIF[InterfaceType].mState = State;
    return NO_ERROR;
}

status_t AudioDigitalControl::SetMemoryPathEnable(uint32 InterfaceType, bool bEnable)
{
    ALOGD("+%s(), InterfaceType = %d, bEnable = %d\n", __FUNCTION__, InterfaceType, bEnable);
    if (bEnable && InterfaceType < AudioDigitalType::NUM_OF_MEM_INTERFACE)
    {
        mAfeReg->SetAfeReg(AFE_DAC_CON0, bEnable << (InterfaceType + 1) , 1 << (InterfaceType + 1));
    }
    else
    {
        mAfeReg->SetAfeReg(AFE_DAC_CON0, bEnable << (InterfaceType + 1), 1 << (InterfaceType + 1));
#ifdef SIDEGEN_ENABLE
        mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x0, 0xffffffff);
#endif
    }
    //mark interface ewnable or disable
    switch (InterfaceType)
    {
        case (AudioDigitalType::MEM_DL1):
            mAudioDigitalBlock[AudioDigitalType::MEM_DL1] = bEnable;
            break;
        case (AudioDigitalType::MEM_DL2):
            mAudioDigitalBlock[AudioDigitalType::MEM_DL2] = bEnable;
            break;
        case (AudioDigitalType::MEM_VUL):
            mAudioDigitalBlock[AudioDigitalType::MEM_VUL] = bEnable;
#ifdef SIDEGEN_ENABLE
            mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0xc4662662, 0xffffffff);
#endif
            break;
#if 0
        case (AudioDigitalType::MEM_DAI):
            mAudioDigitalBlock[AudioDigitalType::MEM_DAI] = bEnable;
            break;
#endif
        case (AudioDigitalType::MEM_AWB):
            mAudioDigitalBlock[AudioDigitalType::MEM_AWB] = bEnable;
            break;
        case (AudioDigitalType::MEM_MOD_DAI):
            mAudioDigitalBlock[AudioDigitalType::MEM_MOD_DAI] = bEnable;
            break;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetMemDuplicateWrite(uint32 InterfaceType, int dupwrite)
{
    ALOGD("+%s(), InterfaceType = %d, dupwrite = %d\n", __FUNCTION__, InterfaceType, dupwrite);
    mAudioMEMIF[InterfaceType].mdupwrite = dupwrite;
    switch (InterfaceType)
    {
#if 0
        case AudioDigitalType::MEM_DAI:
        {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, dupwrite << 29, 1 << 29);
            break;
        }
#endif
        case AudioDigitalType::MEM_MOD_DAI:
        {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, dupwrite << 31, 1 << 31);
            break;
        }
        default:
            ALOGW("%s(), InterfaceType = %d, dupwrite = %d\n", __FUNCTION__, InterfaceType, dupwrite);
            return INVALID_OPERATION;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetMemMonoChannel(uint32 Memory_Interface, bool channel)
{
    ALOGD("+%s(), Memory_Interface = %d, channel = %d\n", __FUNCTION__, Memory_Interface, channel);
    mAudioMEMIF[Memory_Interface].mMonoSel = channel;
    switch (Memory_Interface)
    {
        case AudioDigitalType::MEM_AWB:
        {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, channel << 24, 1 << 24);
            break;
        }
        case AudioDigitalType::MEM_VUL:
        {
            mAfeReg->SetAfeReg(AFE_DAC_CON1, channel << 27, 1 << 27);
            break;
        }
        default:
            ALOGW("%s(), Memory_Interface = %d, channel = %d\n", __FUNCTION__, Memory_Interface, channel);
            return INVALID_OPERATION;
    }
    return NO_ERROR;

}

status_t AudioDigitalControl::SetMemIfInterruptSample(uint32 InterfaceType, uint32 SampleCount)
{
    ALOGD("+%s(), InterfaceType = %d, SampleCount = %d\n", __FUNCTION__, InterfaceType, SampleCount);
    mAudioMEMIF[InterfaceType].mInterruptSample = SampleCount;
    return NO_ERROR;
}

uint32 AudioDigitalControl::GetMemIfInterruptSample(uint32 InterfaceType)
{
    ALOGD("+%s(), InterfaceType = %d\n", __FUNCTION__, InterfaceType);
    return mAudioMEMIF[InterfaceType].mInterruptSample;
}

status_t AudioDigitalControl::SetinputConnection(uint32 ConnectionState, uint32 Input , uint32 Output)
{
    ALOGD("+%s(), ConnectionState = %d, Input = %d, Output = %d\n", __FUNCTION__, ConnectionState, Input, Output);
    if (mInterConnectionInstance)
    {
        return mInterConnectionInstance->SetinputConnection(ConnectionState, Input, Output);
    }
    else
    {
        return INVALID_OPERATION;
    }
}

status_t AudioDigitalControl::SetoutputConnectionFormat(AudioDigitalType::OUTPUT_DATA_FORMAT ConnectionFormat, AudioDigitalType::InterConnectionOutput Output)
{
    return INVALID_OPERATION;
}

AudioDigitalType::OUTPUT_DATA_FORMAT AudioDigitalControl::GetoutputConnectionFormat(AudioDigitalType::InterConnectionOutput Output)
{
    return AudioDigitalType::OUTPUT_DATA_FORMAT_16BIT;
}

status_t AudioDigitalControl::SetIrqMcuEnable(AudioDigitalType::IRQ_MCU_MODE Irqmode, bool bEnable)
{
    ALOGD("+%s(), Irqmode = %d, bEnable = %d\n", __FUNCTION__, Irqmode, bEnable);
    switch (Irqmode)
    {
        case AudioDigitalType::IRQ1_MCU_MODE:
        case AudioDigitalType::IRQ2_MCU_MODE:
        case AudioDigitalType::IRQ3_MCU_MODE:
        {
            mAfeReg->SetAfeReg(AFE_IRQ_MCU_CON, (bEnable << Irqmode), (1 << Irqmode));
            mAudioMcuMode[Irqmode].mStatus = bEnable;
            break;
        }
        default:
            ALOGW("%s(), Irqmode = %d\n", __FUNCTION__, Irqmode);
            break;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetIrqMcuSampleRate(AudioDigitalType::IRQ_MCU_MODE Irqmode, uint32 SampleRate)
{
    ALOGD("+%s(), Irqmode = %d, SampleRate = %d\n", __FUNCTION__, Irqmode, SampleRate);
    switch (Irqmode)
    {
        case AudioDigitalType::IRQ1_MCU_MODE:
        {
            mAfeReg->SetAfeReg(AFE_IRQ_MCU_CON, (SampleRateTransform(SampleRate) << 4), 0x000000f0);
            mAudioMcuMode[Irqmode].mSampleRate = SampleRate;
            break;
        }
        case AudioDigitalType::IRQ2_MCU_MODE:
        {
            mAfeReg->SetAfeReg(AFE_IRQ_MCU_CON, (SampleRateTransform(SampleRate) << 8), 0x00000f00);
            mAudioMcuMode[Irqmode].mSampleRate = SampleRate;
            break;
        }
        case AudioDigitalType::IRQ3_MCU_MODE:
        default:
            ALOGW("%s(), Irqmode = %d\n", __FUNCTION__, Irqmode);
            break;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetIrqMcuCounter(AudioDigitalType::IRQ_MCU_MODE Irqmode, uint32 Counter)
{
    ALOGD("+%s(), Irqmode = %d, Counter = %d\n", __FUNCTION__, Irqmode, Counter);
    switch (Irqmode)
    {
        case AudioDigitalType::IRQ1_MCU_MODE:
        {
            mAfeReg->SetAfeReg(AFE_IRQ_MCU_CNT1, Counter, 0xffffffff);
            mAudioMcuMode[Irqmode].mIrqMcuCounter = Counter;
            break;
        }
        case AudioDigitalType::IRQ2_MCU_MODE:
        {
            mAfeReg->SetAfeReg(AFE_IRQ_MCU_CNT2, Counter, 0xffffffff);
            mAudioMcuMode[Irqmode].mIrqMcuCounter = Counter;
            break;
        }
        case AudioDigitalType::IRQ3_MCU_MODE:
        default:
            ALOGW("%s(), Irqmode = %d\n", __FUNCTION__, Irqmode);
            break;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetMicinputInverse(bool bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    mMicInverse = bEnable;
    return NO_ERROR;
}


status_t AudioDigitalControl::GetIrqStatus(AudioDigitalType::IRQ_MCU_MODE Irqmode, AudioIrqMcuMode *Mcumode)
{
    switch (Irqmode)
    {
        case AudioDigitalType::IRQ1_MCU_MODE:
        case AudioDigitalType::IRQ2_MCU_MODE:
        case AudioDigitalType::IRQ3_MCU_MODE:
            memcpy((void *)Mcumode, (const void *)&mAudioMcuMode[Irqmode], sizeof(AudioIrqMcuMode));
            break;
        default:
            ALOGE("%s(), Irqmode = %d\n", __FUNCTION__, Irqmode);
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetAfeEnable(bool  bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    bool AfeRunning = GetAfeDigitalStatus();
    ALOGD("SetAfeEnable bEnable = %d", bEnable);
    if (AfeRunning && (!bEnable))
    {
        ALOGW("SetAfeEnable disable but digital still running");
        return INVALID_OPERATION;
    }
    mAfeReg->SetAfeReg(AFE_DAC_CON0, bEnable, 0x1); // AFE_DAC_CON0[0]: AFE_ON
    return NO_ERROR;
}



bool AudioDigitalControl::GetAfeEnable(bool  bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    uint32 AFE_DAC_CON0_reg  = mAfeReg->GetAfeReg(AFE_DAC_CON0);
    return (AFE_DAC_CON0_reg & 0x1);
}

int AudioDigitalControl::GetDigitalBlockState(int block)
{
    ALOGD("+%s(), block = %d\n", __FUNCTION__, block);
    if (block == AudioDigitalType::NUM_OF_DIGITAL_BLOCK)
    {
        ALOGW("%s(), block = %d\n", __FUNCTION__, block);
        return false;
    }
    else
    {
        return mAudioDigitalBlock[block];
    }
}

bool AudioDigitalControl::GetAfeDigitalStatus()
{
    ALOGD("+%s()\n", __FUNCTION__);
    for (int i = 0 ; i < AudioDigitalType::NUM_OF_DIGITAL_BLOCK ; i++)
    {
        if (mAudioMEMIF[i].mState == AudioMEMIFAttribute::STATE_EXECUTING || mAudioDigitalBlock[i] != 0)
        {
            ALOGW("GetAfeDigitalStatus mAudioMEMIF[%d] state = %d, mAudioDigitalBlock = %d", i, mAudioMEMIF[i].mState, mAudioDigitalBlock[i]);
            return true;
        }
    }
    return false;
}

status_t AudioDigitalControl::SetI2SDacOutAttribute(uint32_t sampleRate)
{
    ALOGD("+%s(), sampleRate = %d\n", __FUNCTION__, sampleRate);
    mDacI2SOut.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
    mDacI2SOut.mI2S_SLAVE = AudioDigtalI2S::MASTER_MODE;
    mDacI2SOut.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
    mDacI2SOut.mI2S_FMT = AudioDigtalI2S::I2S;
    mDacI2SOut.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
    mDacI2SOut.mI2S_SAMPLERATE = sampleRate;
    SetI2SDacOut(&mDacI2SOut);
    return NO_ERROR;
}

status_t AudioDigitalControl::SetI2SDacOut(AudioDigtalI2S *mDigitalI2S)
{
    ALOGD("+%s()\n", __FUNCTION__);
    mAfeReg->SetAfeReg(AFE_PREDIS_CON0, 0, AFE_MASK_ALL);
    mAfeReg->SetAfeReg(AFE_PREDIS_CON1, 0, AFE_MASK_ALL);
    uint32 AfeAddaDLSrc2Con0, AfeAddaDLSrc2Con1;
    memcpy((void *)&mDacI2SOut, (void *)mDigitalI2S, sizeof(AudioDigtalI2S));
    ALOGD("mDacI2SOut.mI2S_SAMPLERATE [%d]\n", mDacI2SOut.mI2S_SAMPLERATE);
    if (mDacI2SOut.mI2S_SAMPLERATE == 8000)
    {
        AfeAddaDLSrc2Con0 = 0;
    }
    else if (mDacI2SOut.mI2S_SAMPLERATE == 11025)
    {
        AfeAddaDLSrc2Con0 = 1;
    }
    else if (mDacI2SOut.mI2S_SAMPLERATE == 12000)
    {
        AfeAddaDLSrc2Con0 = 2;
    }
    else if (mDacI2SOut.mI2S_SAMPLERATE == 16000)
    {
        AfeAddaDLSrc2Con0 = 3;
    }
    else if (mDacI2SOut.mI2S_SAMPLERATE == 22050)
    {
        AfeAddaDLSrc2Con0 = 4;
    }
    else if (mDacI2SOut.mI2S_SAMPLERATE == 24000)
    {
        AfeAddaDLSrc2Con0 = 5;
    }
    else if (mDacI2SOut.mI2S_SAMPLERATE == 32000)
    {
        AfeAddaDLSrc2Con0 = 6;
    }
    else if (mDacI2SOut.mI2S_SAMPLERATE == 44100)
    {
        AfeAddaDLSrc2Con0 = 7;
    }
    else if (mDacI2SOut.mI2S_SAMPLERATE == 48000)
    {
        AfeAddaDLSrc2Con0 = 8;
    }
    else
    {
        AfeAddaDLSrc2Con0 = 7;    //Default 44100
    }
    //ASSERT(0);
    if (AfeAddaDLSrc2Con0 == 0 || AfeAddaDLSrc2Con0 == 3) //8k or 16k voice mode
    {
        AfeAddaDLSrc2Con0 = (AfeAddaDLSrc2Con0 << 28) | (0x03 << 24) | (0x03 << 11) | (0x01 << 5);
    }
    else
    {
        AfeAddaDLSrc2Con0 = (AfeAddaDLSrc2Con0 << 28) | (0x03 << 24) | (0x03 << 11);
    }
    //SA suggest apply -0.3db to audio/speech path
    AfeAddaDLSrc2Con0 = AfeAddaDLSrc2Con0 | (0x01 << 1); //2013.02.22 for voice mode degrade 0.3 db
    AfeAddaDLSrc2Con1 = 0xf74f0000;

    ALOGD("%s(), AfeAddaDLSrc2Con0[0x%x] = 0x%x\n", __FUNCTION__, AFE_ADDA_DL_SRC2_CON0, AfeAddaDLSrc2Con0);
    ALOGD("%s(), AfeAddaDLSrc2Con1[0x%x] = 0x%x\n", __FUNCTION__, AFE_ADDA_DL_SRC2_CON1, AfeAddaDLSrc2Con1);

    mAfeReg->SetAfeReg(AFE_ADDA_DL_SRC2_CON0, AfeAddaDLSrc2Con0, AFE_MASK_ALL);
    mAfeReg->SetAfeReg(AFE_ADDA_DL_SRC2_CON1, AfeAddaDLSrc2Con1, AFE_MASK_ALL);

    uint32 Audio_I2S_Dac = 0;
    Audio_I2S_Dac |= (mDacI2SOut.mLR_SWAP << 31);
    Audio_I2S_Dac |= (SampleRateTransform(mDacI2SOut.mI2S_SAMPLERATE) << 8);
    Audio_I2S_Dac |= (mDacI2SOut.mINV_LRCK << 5);
    Audio_I2S_Dac |= (mDacI2SOut.mI2S_FMT << 3);
    Audio_I2S_Dac |= (mDacI2SOut.mI2S_WLEN << 1);
    ALOGD("%s(), Audio_I2S_Dac[0x%x] = 0x%x\n", __FUNCTION__, AFE_I2S_CON1, Audio_I2S_Dac);
    mAfeReg->SetAfeReg(AFE_I2S_CON1, Audio_I2S_Dac, AFE_MASK_ALL);
    return NO_ERROR;
}

status_t AudioDigitalControl::GetI2SDacOut(AudioDigtalI2S *mDigitalI2S)
{
    memcpy((void *)mDigitalI2S, (void *)&mDacI2SOut, sizeof(AudioDigtalI2S));
    return NO_ERROR;
}

status_t AudioDigitalControl::SetI2SDacEnable(bool bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    mAfeReg->SetAfeReg(AFE_ADDA_DL_SRC2_CON0, bEnable ? 1 : 0, 0x01);
    mDacI2SOut.mI2S_EN = bEnable;
    mAfeReg->SetAfeReg(AFE_I2S_CON1, bEnable, 0x1); // TODO: AFE_I2S_CON1 defined both in "AudioAfeReg.h" & AudioAnalogReg.h
    mAudioDigitalBlock[AudioDigitalType::I2S_OUT_DAC] = bEnable;

    if (bEnable == true)
    {
        mAfeReg->SetAfeReg(AFE_ADDA_UL_DL_CON0, 0x0001, 0x0001);
    }
    else if (mAudioDigitalBlock[AudioDigitalType::I2S_IN_ADC] == false) // if I2S ADC is not used, too, then can turn off ADDA UL/DL
    {
        mAfeReg->SetAfeReg(AFE_ADDA_UL_DL_CON0, 0x0000, 0x0001);
    }

    if (true == bEnable)
    {
        mAfeReg->SetAfeReg(FPGA_CFG1, 0, 0x10);    //For FPGA Pin the same with DAC
    }
    else
    {
        mAfeReg->SetAfeReg(FPGA_CFG1, 1 << 4, 0x10);    //For FPGA Pin the same with DAC
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::Set2ndI2SOutAttribute(uint32_t sampleRate)
{
    ALOGD("+%s(), sampleRate = %d\n", __FUNCTION__, sampleRate);
    m2ndI2S.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
    m2ndI2S.mI2S_SLAVE = AudioDigtalI2S::MASTER_MODE;
    m2ndI2S.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
    m2ndI2S.mI2S_FMT = AudioDigtalI2S::I2S;
#if defined(I2S_LOW_JITTER_MODE)
    m2ndI2S.mI2S_WLEN = AudioDigtalI2S::WLEN_32BITS;
    m2ndI2S.mI2S_HD_EN = AudioDigtalI2S::LOW_JITTER_CLOCK;
#else
    m2ndI2S.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
    m2ndI2S.mI2S_HD_EN = AudioDigtalI2S::NORMAL_CLOCK;
#endif
    m2ndI2S.mI2S_SAMPLERATE = sampleRate;
    Set2ndI2SOut(&m2ndI2S);
    return NO_ERROR;
}

status_t AudioDigitalControl::Set2ndI2SOut(AudioDigtalI2S *mDigitalI2S)
{
    ALOGD("+%s()\n", __FUNCTION__);
    memcpy((void *)&m2ndI2S, (void *)mDigitalI2S, sizeof(AudioDigtalI2S));
    uint32 u32AudioI2S = 0;
    // set 2nd samplerate to AFE_ADC_CON1
    //SetMemIfSampleRate(AudioDigitalType::MEM_I2S, m2ndI2S.mI2S_SAMPLERATE);
    u32AudioI2S = I2SSampleRateTransform(mDigitalI2S->mI2S_SAMPLERATE);
    u32AudioI2S = (u32AudioI2S << 8);
    u32AudioI2S |= (m2ndI2S.mINV_LRCK << 5);
    u32AudioI2S |= (m2ndI2S.mI2S_FMT << 3);
    u32AudioI2S |= (m2ndI2S.mI2S_WLEN << 1);
    ALOGD("Set2ndI2SOut u32AudioI2S= 0x%x", u32AudioI2S);
    mAfeReg->SetAfeReg(AFE_I2S_CON3, u32AudioI2S, AFE_MASK_ALL);
    return NO_ERROR;
}

status_t AudioDigitalControl::Get2ndI2SOut(AudioDigtalI2S *mDigitalI2S)
{
    ALOGD("+%s()\n", __FUNCTION__);
    memcpy((void *)mDigitalI2S, (void *)&m2ndI2S, sizeof(AudioDigtalI2S));
    return NO_ERROR;
}

status_t AudioDigitalControl::SetI2SASRCConfig(bool bIsUseASRC, unsigned int dToSampleRate)
{
    ALOGD("+%s() bIsUseASRC [%d] dToSampleRate [%d]\n", __FUNCTION__, bIsUseASRC, dToSampleRate);
    if (true == bIsUseASRC)
    {
        ASSERT(dToSampleRate == 44100 || dToSampleRate == 48000);
        mAfeReg->SetAfeReg(AFE_CONN4, 0, 1 << 30);
        SetMemIfSampleRate(AudioDigitalType::MEM_I2S, dToSampleRate);//To target sample rate
        mAfeReg->SetAfeReg(AFE_ASRC_CON13, 0, 1 << 16); //0:Stereo 1:Mono
        if (dToSampleRate == 44100)
        {
            mAfeReg->SetAfeReg(AFE_ASRC_CON14, 0xDC8000, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON15, 0xA00000, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON17, 0x1FBD, AFE_MASK_ALL);
        }
        else
        {
            mAfeReg->SetAfeReg(AFE_ASRC_CON14, 0x600000, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON15, 0x400000, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON17, 0xCB2, AFE_MASK_ALL);
        }

        mAfeReg->SetAfeReg(AFE_ASRC_CON16, 0x00075987, AFE_MASK_ALL);//Calibration setting
        mAfeReg->SetAfeReg(AFE_ASRC_CON20, 0x00001b00, AFE_MASK_ALL);//Calibration setting
    }
    else
    {
        mAfeReg->SetAfeReg(AFE_CONN4, 1 << 30, 1 << 30);
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetI2SASRCEnable(bool bEnable)
{
    if (true == bEnable)
    {
        mAfeReg->SetAfeReg(AFE_ASRC_CON0, ((1 << 6) | (1 << 0)), ((1 << 6) | (1 << 0)));
    }
    else
    {
        uint32 dNeedDisableASM  = (mAfeReg->GetAfeReg(AFE_ASRC_CON0) & 0x0030) ? 1 : 0;
        mAfeReg->SetAfeReg(AFE_ASRC_CON0, 0, (1 << 6 | dNeedDisableASM));
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::Set2ndI2SInConfig(unsigned int sampleRate, bool bIsSlaveMode)
{
    AudioDigtalI2S _2ndI2SIn_attribute;
    memset((void *)&_2ndI2SIn_attribute, 0, sizeof(_2ndI2SIn_attribute));
    _2ndI2SIn_attribute.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
    _2ndI2SIn_attribute.mI2S_SLAVE = bIsSlaveMode;
    _2ndI2SIn_attribute.mI2S_SAMPLERATE = sampleRate;
    _2ndI2SIn_attribute.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
    _2ndI2SIn_attribute.mI2S_FMT = AudioDigtalI2S::I2S;
    _2ndI2SIn_attribute.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
    Set2ndI2SIn(&_2ndI2SIn_attribute);
    return NO_ERROR;
}

status_t AudioDigitalControl::Set2ndI2SIn(AudioDigtalI2S *mDigitalI2S)
{
    memcpy((void *)&m2ndI2S, (void *)mDigitalI2S, sizeof(AudioDigtalI2S));
    uint32 Audio_I2S_Adc = 0;
    if (!m2ndI2S.mI2S_SLAVE) //Master setting SampleRate only
    {
        SetMemIfSampleRate(AudioDigitalType::MEM_I2S, m2ndI2S.mI2S_SAMPLERATE);
    }
    Audio_I2S_Adc |= (m2ndI2S.mINV_LRCK << 5);
    Audio_I2S_Adc |= (m2ndI2S.mI2S_FMT << 3);
    Audio_I2S_Adc |= (m2ndI2S.mI2S_SLAVE << 2);
    Audio_I2S_Adc |= (m2ndI2S.mI2S_WLEN << 1);
    Audio_I2S_Adc |= (m2ndI2S.mI2S_IN_PAD_SEL << 28);
    Audio_I2S_Adc |= 1 << 31;//Default enable phase_shift_fix for better quality
    ALOGD("Set2ndI2SIn Audio_I2S_Adc= 0x%x", Audio_I2S_Adc);
    mAfeReg->SetAfeReg(AFE_I2S_CON, Audio_I2S_Adc, 0xfffffffe);
    if (!m2ndI2S.mI2S_SLAVE)
    {
        mAfeReg->SetAfeReg(FPGA_CFG1, 1 << 8, 0x0100);
    }
    else
    {
        mAfeReg->SetAfeReg(FPGA_CFG1, 0, 0x0100);
    }

    return NO_ERROR;
}
status_t AudioDigitalControl::Set2ndI2SInEnable(bool bEnable)
{
    ALOGD("Set2ndI2SInEnable bEnable = %d", bEnable);
    m2ndI2S.mI2S_EN = bEnable;
    mAfeReg->SetAfeReg(AFE_I2S_CON, bEnable, 0x1);
    mAudioDigitalBlock[AudioDigitalType::I2S_IN_2] = bEnable;
    return NO_ERROR;
}
status_t AudioDigitalControl::Set2ndI2SOutEnable(bool bEnable)
{
    ALOGD("Set2ndI2SOutEnable bEnable = %d", bEnable);
    m2ndI2S.mI2S_EN = bEnable;
    mAfeReg->SetAfeReg(AFE_I2S_CON3, bEnable, 0x1);
    mAudioDigitalBlock[AudioDigitalType::I2S_OUT_2] = bEnable;
    if (true == bEnable)
    {
        mAfeReg->SetAfeReg(FPGA_CFG1, 1 << 4, 0x10);    //For FPGA Pin the same with DAC
    }
    else
    {
        mAfeReg->SetAfeReg(FPGA_CFG1, 0, 0x10);    //For FPGA Pin the same with DAC
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::Set2ndI2SEnable(bool bEnable)
{
    ALOGD(" doesn't support Set2ndI2SEnable");
    return NO_ERROR;
}

status_t AudioDigitalControl::SetI2SSoftReset(bool bEnable)
{
    ALOGD("%s(), flag= %d\n", __FUNCTION__, bEnable );
    uint32 audio_top_control_1 = 0;
    if(bEnable)
       audio_top_control_1 |= 1 << 1;
    mAfeReg->SetAfeReg(AUDIO_TOP_CON1, audio_top_control_1, 0x2);

    return NO_ERROR;
}



status_t AudioDigitalControl::SetI2SAdcIn(AudioDigtalI2S *mDigitalI2S)
{
    ALOGD("+%s()\n", __FUNCTION__);
    memcpy((void *)&mAdcI2SIn, (void *)mDigitalI2S, sizeof(AudioDigtalI2S));

    if (false == mUseI2SADCInStatus)
    {

        AudioMEMIFAttribute::SAMPLINGRATE eSamplingRate = SampleRateTransform(mAdcI2SIn.mI2S_SAMPLERATE);
        uint32 dVoiceModeSelect = 0;
        mAfeReg->SetAfeReg(AFE_ADDA_TOP_CON0, 0, 0x1); //Using Internal ADC

        if (eSamplingRate == AudioMEMIFAttribute::AFE_8000HZ)
        {
            dVoiceModeSelect = 0;
        }
        else if (eSamplingRate == AudioMEMIFAttribute::AFE_16000HZ)
        {
            dVoiceModeSelect = 1;
        }
        else if (eSamplingRate == AudioMEMIFAttribute::AFE_32000HZ)
        {
            dVoiceModeSelect = 2;
        }
        else if (eSamplingRate == AudioMEMIFAttribute::AFE_48000HZ)
        {
            dVoiceModeSelect = 3;
        }
        else
        {
            ASSERT(0);
        }

        mAfeReg->SetAfeReg(AFE_ADDA_UL_SRC_CON0, ((dVoiceModeSelect << 2) | dVoiceModeSelect) << 17, 0x001E0000);
        mAfeReg->SetAfeReg(AFE_ADDA_NEWIF_CFG0, 0x03F87201, 0xFFFFFFFF); // up8x txif sat on
        mAfeReg->SetAfeReg(AFE_ADDA_NEWIF_CFG1, ((dVoiceModeSelect < 3) ? 1 : 3) << 10, 0x00000C00);

    }
    else
    {
        mAfeReg->SetAfeReg(AFE_ADDA_TOP_CON0, 1, 0x1); //Using External ADC
        uint32 Audio_I2S_Adc = 0;
        Audio_I2S_Adc |= (mAdcI2SIn.mLR_SWAP << 31);
        Audio_I2S_Adc |= (mAdcI2SIn.mBuffer_Update_word << 24);
        Audio_I2S_Adc |= (mAdcI2SIn.mINV_LRCK << 23);
        Audio_I2S_Adc |= (mAdcI2SIn.mFpga_bit_test << 22);
        Audio_I2S_Adc |= (mAdcI2SIn.mFpga_bit << 21);
        Audio_I2S_Adc |= (mAdcI2SIn.mloopback << 20);
        Audio_I2S_Adc |= (SampleRateTransform(mAdcI2SIn.mI2S_SAMPLERATE) << 8);
        Audio_I2S_Adc |= (mAdcI2SIn.mI2S_FMT << 3);
        Audio_I2S_Adc |= (mAdcI2SIn.mI2S_WLEN << 1);
        ALOGD("%s Audio_I2S_Adc = 0x%x", __FUNCTION__, Audio_I2S_Adc);
        mAfeReg->SetAfeReg(AFE_I2S_CON2, Audio_I2S_Adc, AFE_MASK_ALL);
    }

    return NO_ERROR;
}

status_t AudioDigitalControl::GetI2SAdcIn(AudioDigtalI2S *mDigitalI2S)
{
    ALOGD("+%s()\n", __FUNCTION__);
    memcpy((void *)mDigitalI2S, (void *)&mAdcI2SIn, sizeof(AudioDigtalI2S));
    return NO_ERROR;
}

status_t AudioDigitalControl::SetI2SAdcEnable(bool bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    mAdcI2SIn.mI2S_EN = bEnable;
    if (false == mUseI2SADCInStatus)
    {

        mAfeReg->SetAfeReg(AFE_ADDA_UL_SRC_CON0, bEnable ? 1 : 0, 0x01);

        if (bEnable == true)
        {
            mAfeReg->SetAfeReg(AFE_ADDA_UL_DL_CON0, 0x0001, 0x0001);
        }
        else if (mAudioDigitalBlock[AudioDigitalType::I2S_OUT_DAC] == false)  // if I2S DAC out is not used, too
        {
            mAfeReg->SetAfeReg(AFE_ADDA_UL_DL_CON0, 0x0000, 0x0001);
        }
    }
    else
    {
        mAfeReg->SetAfeReg(AFE_I2S_CON2, bEnable, 0x1);
    }

    mAudioDigitalBlock[AudioDigitalType::I2S_IN_ADC] = bEnable;
    return NO_ERROR;
}

AudioDigtalI2S::I2S_SAMPLERATE AudioDigitalControl::I2SSampleRateTransform(unsigned int SampleRate)
{
    ALOGD("+%s(), SampleRate = %d\n", __FUNCTION__, SampleRate);
    switch (SampleRate)
    {
        case 8000:
            return AudioDigtalI2S::I2S_8K;
        case 11025:
            return AudioDigtalI2S::I2S_11K;
        case 12000:
            return AudioDigtalI2S::I2S_12K;
        case 16000:
            return AudioDigtalI2S::I2S_16K;
        case 22050:
            return AudioDigtalI2S::I2S_22K;
        case 24000:
            return AudioDigtalI2S::I2S_24K;
        case 32000:
            return AudioDigtalI2S::I2S_32K;
        case 44100:
            return AudioDigtalI2S::I2S_44K;
        case 48000:
            return AudioDigtalI2S::I2S_48K;
        default:
            ALOGE("I2SSampleRateTransform no such samplerate matching SampleRate = %d", SampleRate);
            break;
    }
    return AudioDigtalI2S::I2S_44K;
}

status_t AudioDigitalControl::SetMrgI2SEnable(bool bEnable, unsigned int sampleRate)
{
    ALOGW(" unsupport SetMrgI2SEnable");
    return NO_ERROR;
}

status_t AudioDigitalControl::ResetFmChipMrgIf(void)
{
    ALOGD("+%s(), Reset MergeIf HW of FM Chip\n", __FUNCTION__);
    Mutex::Autolock _l(mLock);
    ::ioctl(mFd, AUDDRV_RESET_FMCHIP_MERGEIF);
    return NO_ERROR;
}

status_t AudioDigitalControl::SetDAIBBT(AudioDigitalDAIBT *DAIBT)
{
    ALOGW(" unsupport SetDAIBBT");
    return NO_ERROR;
}

status_t AudioDigitalControl::GetDAIBTOut(AudioDigitalDAIBT *DAIBT)
{
    ALOGW(" unsupport GetDAIBTOut");
    return NO_ERROR;
}

status_t AudioDigitalControl::SetDAIBTEnable(bool bEnable)
{
    ALOGW(" unsupport SetDAIBTEnable");
    return NO_ERROR;
}

status_t AudioDigitalControl::SetHwDigitalGainMode(AudioDigitalType::Hw_Digital_Gain GainType, AudioMEMIFAttribute::SAMPLINGRATE SampleRate,  uint32 SamplePerStep)
{
    ALOGD("+%s(), GainType = %d, SampleRate = %d, SamplePerStep= %d\n", __FUNCTION__, GainType, SampleRate, SamplePerStep);
    uint32 value = 0;
    value = SamplePerStep << 8 | SampleRate << 4;
    switch (GainType)
    {
        case AudioDigitalType::HW_DIGITAL_GAIN1:
            mAfeReg->SetAfeReg(AFE_GAIN1_CON0, value, 0xfff0);
            break;
        case AudioDigitalType::HW_DIGITAL_GAIN2:
            mAfeReg->SetAfeReg(AFE_GAIN2_CON0, value, 0xfff0);
            break;
        default:
            ALOGW("%s with no match type\n", __FUNCTION__);
            return INVALID_OPERATION;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetHwDigitalGainEnable(AudioDigitalType::Hw_Digital_Gain GainType, bool Enable)
{
    ALOGD("+%s(), GainType = %d, Enable = %d\n", __FUNCTION__, GainType, Enable);
    switch (GainType)
    {
        case AudioDigitalType::HW_DIGITAL_GAIN1:
            if (Enable)
            {
                mAfeReg->SetAfeReg(AFE_GAIN1_CUR, 0, 0xFFFFFFFF);    //Let current gain be 0 to ramp up
            }
            mAfeReg->SetAfeReg(AFE_GAIN1_CON0, Enable, 0x1);
            break;
        case AudioDigitalType::HW_DIGITAL_GAIN2:
            if (Enable)
            {
                mAfeReg->SetAfeReg(AFE_GAIN2_CUR, 0, 0xFFFFFFFF);    //Let current gain be 0 to ramp up
            }
            mAfeReg->SetAfeReg(AFE_GAIN2_CON0, Enable, 0x1);
            break;
        default:
            ALOGW("%s with no match type\n", __FUNCTION__);
            return INVALID_OPERATION;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetHwDigitalGainEnableByPassRampUp(AudioDigitalType::Hw_Digital_Gain GainType, bool Enable)
{
    ALOGD("+%s(), GainType = %d, Enable = %d\n", __FUNCTION__, GainType, Enable);
    switch (GainType)
    {
        case AudioDigitalType::HW_DIGITAL_GAIN1:
            mAfeReg->SetAfeReg(AFE_GAIN1_CON0, Enable, 0x1);
            break;
        case AudioDigitalType::HW_DIGITAL_GAIN2:
            mAfeReg->SetAfeReg(AFE_GAIN2_CON0, Enable, 0x1);
            break;
        default:
            ALOGW("%s with no match type\n", __FUNCTION__);
            return INVALID_OPERATION;
    }
    return NO_ERROR;
}

uint32 AudioDigitalControl::DlPolicyByDevice(uint32_t Device)
{
    ALOGV("+%s(), Device = %d\n", __FUNCTION__, Device);
    if (Device & AUDIO_DEVICE_OUT_BLUETOOTH_SCO ||
        Device & AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET ||
        Device & AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT)
    {
        return AudioDigitalType::DAI_BT;
    }
    else if (Device & AUDIO_DEVICE_OUT_AUX_DIGITAL ||
             Device & AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET ||
             Device & AUDIO_DEVICE_OUT_FM_TX)
    {
        return AudioDigitalType::I2S_OUT_2;
    }
    else
    {
        return AudioDigitalType::I2S_OUT_DAC;
    }
}


bool AudioDigitalControl::CheckDlDigitalChange(uint32_t PreDevice, uint32_t NewDevice)
{
    uint32 OuputPreDevice = 0 , OutPutNewDevice = 0;
    ALOGD("+%s(), PreDevice = 0x%x, NewDevice = %d\n", __FUNCTION__, PreDevice, NewDevice);
    OuputPreDevice = DlPolicyByDevice(PreDevice);
    OutPutNewDevice = DlPolicyByDevice(NewDevice);
    if (OuputPreDevice != OutPutNewDevice)
    {
        return true;
    }
    else
    {
        return false;
    }
}

status_t AudioDigitalControl::SetHwDigitalGain(uint32 Gain , AudioDigitalType::Hw_Digital_Gain GainType)
{
    ALOGD("+%s(), Gain = 0x%x, gain type = %d\n", __FUNCTION__, Gain, GainType);
    switch (GainType)
    {
        case AudioDigitalType::HW_DIGITAL_GAIN1:
            mAfeReg->SetAfeReg(AFE_GAIN1_CON1, Gain, 0xffffffff);
            break;
        case AudioDigitalType::HW_DIGITAL_GAIN2:
            mAfeReg->SetAfeReg(AFE_GAIN2_CON1, Gain, 0xffffffff);
            break;
        default:
            ALOGW("%s with no match type\n", __FUNCTION__);
            return INVALID_OPERATION;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetHwDigitalCurrentGain(uint32 Gain , AudioDigitalType::Hw_Digital_Gain GainType)
{
    ALOGD("+%s(), Gain = 0x%x, gain type = %d\n", __FUNCTION__, Gain, GainType);
    switch (GainType)
    {
        case AudioDigitalType::HW_DIGITAL_GAIN1:
            mAfeReg->SetAfeReg(AFE_GAIN1_CUR, Gain, 0xffffffff);
            break;
        case AudioDigitalType::HW_DIGITAL_GAIN2:
            mAfeReg->SetAfeReg(AFE_GAIN2_CUR, Gain, 0xffffffff);
            break;
        default:
            ALOGW("%s with no match type\n", __FUNCTION__);
            return INVALID_OPERATION;
    }
    return NO_ERROR;
}
status_t AudioDigitalControl::SetModemPcmConfig(modem_index_t modem_index, AudioDigitalPCM *p_modem_pcm_attribute)
{
    ALOGD("+%s(), modem_index = %d\n", __FUNCTION__, modem_index);
    if (modem_index == MODEM_1) // MODEM_1 use PCM2_INTF_CON (0x53C) !!!
    {
        memcpy((void *)&mModemPcm1, (void *)p_modem_pcm_attribute, sizeof(AudioDigitalPCM));

        uint32 reg_pcm2_intf_con = 0;
        reg_pcm2_intf_con |= (mModemPcm1.mPcmSyncInInv           & 0x01) << 15;
        reg_pcm2_intf_con |= (mModemPcm1.mPcmBckInInv             & 0x01) << 14;
        reg_pcm2_intf_con |= (mModemPcm1.mTxLchRepeatSel     & 0x1) << 13;
        reg_pcm2_intf_con |= (mModemPcm1.mVbt16kModeSel      & 0x1) << 12;
        reg_pcm2_intf_con |= (mModemPcm1.mSingelMicSel       & 0x1) << 7;
        reg_pcm2_intf_con |= (mModemPcm1.mPcmWordLength      & 0x1) << 4;
        reg_pcm2_intf_con |= (mModemPcm1.mPcmModeWidebandSel & 0x1) << 3;
        reg_pcm2_intf_con |= (mModemPcm1.mPcmFormat          & 0x3) << 1;

        ALOGD("%s(), PCM2_INTF_CON(0x%x) = 0x%x\n", __FUNCTION__, PCM2_INTF_CON, reg_pcm2_intf_con);
        mAfeReg->SetAfeReg(PCM2_INTF_CON, reg_pcm2_intf_con, AFE_MASK_ALL);
    }
    else if (modem_index == MODEM_2 || modem_index == MODEM_EXTERNAL)    // MODEM_2 use PCM_INTF_CON1 (0x530) !!!
    {
        memcpy((void *)&mModemPcm2, (void *)p_modem_pcm_attribute, sizeof(AudioDigitalPCM));
        ALOGD("+%s(), modem_index = %d\n", __FUNCTION__, modem_index);

        // config ASRC for modem 2
        if (mModemPcm2.mPcmModeWidebandSel == AudioDigitalPCM::PCM_MODE_8K)
        {
            mAfeReg->SetAfeReg(AFE_ASRC_CON1, 0x00001964, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON2, 0x00400000, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON3, 0x00400000, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON4, 0x00001964, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON7, 0x00000CB2, AFE_MASK_ALL);
        }
        else if (mModemPcm2.mPcmModeWidebandSel == AudioDigitalPCM::PCM_MODE_16K)
        {
            mAfeReg->SetAfeReg(AFE_ASRC_CON1, 0x00000cb2, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON2, 0x00400000, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON3, 0x00400000, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON4, 0x00000cb2, AFE_MASK_ALL);
            mAfeReg->SetAfeReg(AFE_ASRC_CON7, 0x00000659, AFE_MASK_ALL);
        }

        // config modem 2
        uint32 reg_pcm_intf_con1 = 0;
        reg_pcm_intf_con1 |= (mModemPcm2.mPcmSyncOutInv          & 0x01) << 23;
        reg_pcm_intf_con1 |= (mModemPcm2.mPcmBckOutInv            & 0x01) << 22;
        reg_pcm_intf_con1 |= (mModemPcm2.mPcmSyncInInv            & 0x01) << 21;
        reg_pcm_intf_con1 |= (mModemPcm2.mPcmBckInInv              & 0x01) << 20;
        reg_pcm_intf_con1 |= (mModemPcm2.mTxLchRepeatSel       & 0x01) << 19;
        reg_pcm_intf_con1 |= (mModemPcm2.mVbt16kModeSel        & 0x01) << 18;
        reg_pcm_intf_con1 |= (mModemPcm2.mExtModemSel          & 0x01) << 17;
        reg_pcm_intf_con1 |= (mModemPcm2.mExtendBckSyncLength  & 0x1F) << 9;
        reg_pcm_intf_con1 |= (mModemPcm2.mExtendBckSyncTypeSel & 0x01) << 8;
        reg_pcm_intf_con1 |= (mModemPcm2.mSingelMicSel         & 0x01) << 7;
        reg_pcm_intf_con1 |= (mModemPcm2.mAsyncFifoSel         & 0x01) << 6;
        reg_pcm_intf_con1 |= (mModemPcm2.mSlaveModeSel         & 0x01) << 5;
        reg_pcm_intf_con1 |= (mModemPcm2.mPcmWordLength        & 0x01) << 4;
        reg_pcm_intf_con1 |= (mModemPcm2.mPcmModeWidebandSel   & 0x01) << 3;
        reg_pcm_intf_con1 |= (mModemPcm2.mPcmFormat            & 0x03) << 1;

        ALOGD("%s(), PCM_INTF_CON1(0x%x) = 0x%x", __FUNCTION__, PCM_INTF_CON1, reg_pcm_intf_con1);
        mAfeReg->SetAfeReg(PCM_INTF_CON1, reg_pcm_intf_con1, AFE_MASK_ALL);
    }
    else
    {
        ALOGE("%s(), no such modem_index: %d!!", __FUNCTION__, modem_index);
        return INVALID_OPERATION;
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetModemPcmEnable(modem_index_t modem_index, bool modem_pcm_on)
{
    ALOGD("+%s(), modem_index = %d, modem_pcm_on = %d\n", __FUNCTION__, modem_index, modem_pcm_on);

    if (modem_index == MODEM_1) // MODEM_1 use PCM2_INTF_CON (0x53C) !!!
    {
        mModemPcm1.mModemPcmOn = modem_pcm_on;
        mAfeReg->SetAfeReg(PCM2_INTF_CON, modem_pcm_on, 0x1);
        mAudioDigitalBlock[AudioDigitalType::MODEM_PCM_1_O] = modem_pcm_on;
    }
    else if (modem_index == MODEM_2 || modem_index == MODEM_EXTERNAL) // MODEM_2 use PCM_INTF_CON1 (0x530) !!!
    {
        mModemPcm2.mModemPcmOn = modem_pcm_on;
        if (modem_pcm_on == true) // turn on ASRC before Modem PCM on
        {
            mAfeReg->SetAfeReg(AFE_ASRC_CON6, 0x0001183F, AFE_MASK_ALL); // pre ver. 0x0001188F
            mAfeReg->SetAfeReg(AFE_ASRC_CON0, 0x06003031, 0xFFFFFFBF);
            mAfeReg->SetAfeReg(PCM_INTF_CON1, 0x1, 0x1);
        }
        else if (modem_pcm_on == false) // turn off ASRC after Modem PCM off
        {
            mAfeReg->SetAfeReg(PCM_INTF_CON1, 0x0, 0x1);
            mAfeReg->SetAfeReg(AFE_ASRC_CON6, 0x00000000, AFE_MASK_ALL);
            uint32 dNeedDisableASM  = (mAfeReg->GetAfeReg(AFE_ASRC_CON0) & 0x0040) ? 1 : 0;
            mAfeReg->SetAfeReg(AFE_ASRC_CON0, 0, (1 << 4 | 1 << 5 | dNeedDisableASM));
            mAfeReg->SetAfeReg(AFE_ASRC_CON0, 0x0, 0x1);
        }
        mAudioDigitalBlock[AudioDigitalType::MODEM_PCM_2_O] = modem_pcm_on;

        if (MTK_ENABLE_MD5 == true)
        {
            // modem external (MT6290): AudDrv AUDDRV_MOD_PCM_GPIO(43~46) input pull high/low
            ::ioctl(mFd, AUDDRV_MOD_PCM_GPIO, modem_pcm_on);
        }
    }
    else
    {
        ALOGW("%s(), no such modem_index: %d!!", __FUNCTION__, modem_index);
        return INVALID_OPERATION;
    }

    return NO_ERROR;
}

static const uint16_t kSideToneCoefficientTable16k[] =
{
    0x049C, 0x09E8, 0x09E0, 0x089C,
    0xFF54, 0xF488, 0xEAFC, 0xEBAC,
    0xfA40, 0x17AC, 0x3D1C, 0x6028,
    0x7538
};

static const uint16_t kSideToneCoefficientTable32k[] =
{
    0xff58, 0x0063, 0x0086, 0x00bf,
    0x0100, 0x013d, 0x0169, 0x0178,
    0x0160, 0x011c, 0x00aa, 0x0011,
    0xff5d, 0xfea1, 0xfdf6, 0xfd75,
    0xfd39, 0xfd5a, 0xfde8, 0xfeea,
    0x005f, 0x0237, 0x0458, 0x069f,
    0x08e2, 0x0af7, 0x0cb2, 0x0df0,
    0x0e96
};

status_t AudioDigitalControl::EnableSideToneFilter(bool stf_on)
{
    ALOGD("+%s(), stf_on = %d", __FUNCTION__, stf_on);

    if (stf_on == mSideToneFilterOn)
    {
        ALOGD("-%s(), stf_on(%d) == mSideToneFilterOn(%d), return", __FUNCTION__, stf_on, mSideToneFilterOn);
        return NO_ERROR;
    }

    mSideToneFilterOn = stf_on;


    // MD max support 16K sampling rate
    const uint8_t kSideToneHalfTapNum = sizeof(kSideToneCoefficientTable16k) / sizeof(uint16_t);

    if (stf_on == false)
    {
        // bypass STF result & disable
        const bool bypass_stf_on = true;
        uint32_t reg_value = (bypass_stf_on << 31) | (stf_on << 8);
        mAfeReg->SetAfeReg(AFE_SIDETONE_CON1, reg_value, AFE_MASK_ALL);
        ALOGD("%s(), AFE_SIDETONE_CON1[0x%x] = 0x%x", __FUNCTION__, AFE_SIDETONE_CON1, reg_value);

        // set side tone gain = 0
        mAfeReg->SetAfeReg(AFE_SIDETONE_GAIN, 0, AFE_MASK_ALL);
        ALOGD("%s(), AFE_SIDETONE_GAIN[0x%x] = 0x%x", __FUNCTION__, AFE_SIDETONE_GAIN, 0);
    }
    else
    {
        // set side tone gain
        mAfeReg->SetAfeReg(AFE_SIDETONE_GAIN, 0, AFE_MASK_ALL);
        ALOGD("%s(), AFE_SIDETONE_GAIN[0x%x] = 0x%x", __FUNCTION__, AFE_SIDETONE_GAIN, 0);

        // using STF result & enable & set half tap num
        const bool bypass_stf_on = false;
        uint32_t write_reg_value = (bypass_stf_on << 31) | (stf_on << 8) | kSideToneHalfTapNum;
        mAfeReg->SetAfeReg(AFE_SIDETONE_CON1, write_reg_value, AFE_MASK_ALL);
        ALOGD("%s(), AFE_SIDETONE_CON1[0x%x] = 0x%x", __FUNCTION__, AFE_SIDETONE_CON1, write_reg_value);

        // set side tone coefficient
        const bool enable_read_write = true; // enable read/write side tone coefficient
        const bool read_write_sel = true;    // for write case
        const bool sel_ch2 = false;          // using uplink ch1 as STF input

        uint32_t   read_reg_value = mAfeReg->GetAfeReg(AFE_SIDETONE_CON0);
        for (size_t coef_addr = 0; coef_addr < kSideToneHalfTapNum; coef_addr++)
        {
            bool old_write_ready = (read_reg_value >> 29) & 0x1;
            write_reg_value = enable_read_write << 25 |
                              read_write_sel    << 24 |
                              sel_ch2           << 23 |
                              coef_addr         << 16 |
                              kSideToneCoefficientTable16k[coef_addr];
            mAfeReg->SetAfeReg(AFE_SIDETONE_CON0, write_reg_value, 0x39FFFFF);
            ALOGD("%s(), AFE_SIDETONE_CON0[0x%x] = 0x%x", __FUNCTION__, AFE_SIDETONE_CON0, write_reg_value);

            // wait until flag write_ready changed (means write done)
            for (int try_cnt = 0; try_cnt < 10; try_cnt++)  // max try 10 times
            {
                usleep(3);
                read_reg_value = mAfeReg->GetAfeReg(AFE_SIDETONE_CON0);
                bool new_write_ready = (read_reg_value >> 29) & 0x1;
                if (new_write_ready != old_write_ready) // flip => ok
                {
                    break;
                }
                else
                {
                    ::ioctl(mFd, AUDDRV_LOG_PRINT, 0);
                    ALOGW("%s(), AFE_SIDETONE_CON0[0x%x] = 0x%x, old_write_ready = %d, new_write_ready = %d",
                          __FUNCTION__, AFE_SIDETONE_CON0, read_reg_value, old_write_ready, new_write_ready);
#if 1
                    ASSERT(new_write_ready != old_write_ready);
                    return UNKNOWN_ERROR;
#endif
                }
            }
        }
    }

    ALOGD("-%s(), stf_on = %d", __FUNCTION__, stf_on);
    return NO_ERROR;
}

status_t AudioDigitalControl::EnableSideToneHw(uint32 connection , bool direction  , bool  Enable)
{
    ALOGD("+%s(), connection = %d, direction = %d, Enable= %d\n", __FUNCTION__, connection, direction, Enable);
    if (Enable && direction)
    {
        switch (connection)
        {
            case AudioDigitalType::I00:
            case AudioDigitalType::I01:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x04662662, 0xffffffff);
                break;
            case AudioDigitalType::I02:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x14662662, 0xffffffff);
                break;
            case AudioDigitalType::I03:
            case AudioDigitalType::I04:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x24662662, 0xffffffff);
                break;
            case AudioDigitalType::I05:
            case AudioDigitalType::I06:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x34662662, 0xffffffff);
                break;
            case AudioDigitalType::I07:
            case AudioDigitalType::I08:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x44662662, 0xffffffff);
                break;
            case AudioDigitalType::I09:
            case AudioDigitalType::I10:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x54662662, 0xffffffff);
                break;
            case AudioDigitalType::I11:
            case AudioDigitalType::I12:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x64662662, 0xffffffff);
                break;
            default:
                ALOGW("EnableSideToneHw fail with conenction connection");
                break;
        }
    }
    else if (Enable)
    {
        switch (connection)
        {
            case AudioDigitalType::O00:
            case AudioDigitalType::O01:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x746c26c2, 0xffffffff);
                break;
            case AudioDigitalType::O02:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x846c26c2, 0xffffffff);
                break;
            case AudioDigitalType::O03:
            case AudioDigitalType::O04:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x946c26c2, 0xffffffff);
                break;
            case AudioDigitalType::O05:
            case AudioDigitalType::O06:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0xa46c26c2, 0xffffffff);
                break;
            case AudioDigitalType::O07:
            case AudioDigitalType::O08:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0xb46c26c2, 0xffffffff);
                break;
            case AudioDigitalType::O09:
            case AudioDigitalType::O10:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0xc46c26c2, 0xffffffff);
                break;
            case AudioDigitalType::O11:
                mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0xd46c26c2, 0xffffffff);
                break;
            case AudioDigitalType::O12:
                if (AudioMEMIFAttribute::AFE_8000HZ == mAudioMEMIF[AudioDigitalType::MEM_MOD_DAI].mSampleRate) //MD connect BT Verify (8K Sampling Rate)
                {
                    mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0xe40e80e8, 0xffffffff);
                }
                else if (AudioMEMIFAttribute::AFE_16000HZ == mAudioMEMIF[AudioDigitalType::MEM_MOD_DAI].mSampleRate)
                {
                    mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0xe40f00f0, 0xffffffff);
                }
                else
                {
                    mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0xe46c26c2, 0xffffffff);    //Default
                }
                break;
            default:
                ALOGW("EnableSideToneHw fail with conenction connection");
        }
    }
    else
    {
        //don't set [31:28] as 0 when disable sinetone HW, because it will repalce i00/i01 input with sine gen output.
        //Set 0xf is correct way to disconnect sinetone HW to any I/O.
        mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0xf0000000 , 0xffffffff);
    }
    return NO_ERROR;
}

status_t AudioDigitalControl::SetSinetoneOutputLR(bool bLR)
{
    ALOGD("+%s(), bLR = %d\n", __FUNCTION__, bLR);
    if(bLR)
        mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x01000000, 0x03000000); //AFE_SGEN_CON0, [24] Mute CH1, [25] Mute CH2
    else
        mAfeReg->SetAfeReg(AFE_SGEN_CON0, 0x02000000, 0x03000000);
	return NO_ERROR;
}

void AudioDigitalControl::BT_SCO_SetMode(uint32_t mode)
{
    if (mode == 0)
    {
        BT_mode = 0;
    }
    else
    {
        BT_mode = 1;
    }
    ALOGD("BT_SCO_SetMode, mode=%d, BTmode=%d", mode, BT_mode);
}

uint32_t AudioDigitalControl::BT_SCO_GetMode(void)
{
    ALOGD("BT_SCO_GetMode,BTmode=%d", BT_mode);
    return BT_mode;
}

}//namespace android
