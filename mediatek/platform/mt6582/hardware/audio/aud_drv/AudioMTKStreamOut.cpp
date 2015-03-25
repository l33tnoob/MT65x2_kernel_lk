#include "AudioMTKStreamOut.h"
#include "AudioResourceFactory.h"
#include "AudioResourceManagerInterface.h"
#include "AudioIoctl.h"
#include "AudioDigitalType.h"
#include "SpeechDriverFactory.h"
#include "SpeechBGSPlayer.h"
#include "SpeechPhoneCallController.h"
#include "AudioBTCVSDControl.h"
#include "LoopbackManager.h"
#include "AudioLoopbackController.h"

#include "AudioFMController.h"
#include "AudioMATVController.h"

#include "audio_custom_exp.h"
#include "AudioVUnlockDL.h"

#include "AudioCustParam.h"
#include "CFG_AUDIO_File.h"
#include "AudioUtility.h"

#include"AudioMTKFilter.h"
#include"AudioType.h"

#include "WCNChipController.h"

#include "AudioFtm.h"

#define MAX_NUM_FILE (20)
#define MAX_FILE_LENGTH (50000000)
#define NO_REMAIN_DATA_INFO (-1)

#define LOG_TAG  "AudioMTKStreamOut"
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

//#define VERY_VERBOSE_LOGGING
#ifdef VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

namespace android
{

AudioMTKStreamOut::AudioMTKStreamOut()
{
    ALOGD("+AudioMTKStreamOut default constructor");
    mPDacPCMDumpFile = NULL;
    mPFinalPCMDumpFile = NULL;
    //#if defined(MTK_VIBSPK_SUPPORT)
    mVIBsignalDumpFile = NULL;
    mLoudNotchDumpFile = NULL;
    //#endif

    mSteroToMono = false;
    mForceStandby = false;
    mFilters = NULL;
    DumpFileNum = 0;

    ALOGD("-AudioMTKStreamOut default constructor");
}

AudioMTKStreamOut::AudioMTKStreamOut(uint32_t devices, int *format, uint32_t *channels, uint32_t *sampleRate, status_t *status)
{
    mFd = 0;

    //if (WCNChipController::GetInstance()->BTUseCVSDRemoval() == true)
    {
        mFd2 = 0;
        mFd2 = ::open(kBTDeviceName, O_RDWR);
        if (mFd2 <= 0)
        {
            ALOGE("AudioMTKStreamOut open mFd2 fail");
        }
        ALOGD("+%s(), open cvsd kernel, mFd2: %d, AP errno: %d", __FUNCTION__, mFd2, errno);

        mAudioBTCVSDControl = AudioBTCVSDControl::getInstance();
        if (!mAudioBTCVSDControl)
        {
            ALOGE("AudioBTCVSDControl::getInstance() fail");
        }

        mAudioBTCVSDControl->BT_SCO_SET_TXState(BT_SCO_TXSTATE_IDLE);
        mAudioBTCVSDControl->BT_SCO_SET_RXState(BT_SCO_RXSTATE_IDLE);
    }

    mPDacPCMDumpFile = NULL;
    mPFinalPCMDumpFile = NULL;
    //#if defined(MTK_VIBSPK_SUPPORT)
    mVIBsignalDumpFile = NULL;
    mLoudNotchDumpFile = NULL;
    //#endif
    DumpFileNum = 0;
    // here open audio hardware for register setting
    mFd = ::open(kAudioDeviceName, O_RDWR);
    if (mFd == 0)
    {
        ALOGE("AudioMTKStreamOut open mFd fail");
    }

    ALOGD("+AudioMTKStreamOut constructor devices = %x format = %x channels = %x sampleRate = %d",
          devices, *format, *channels, *sampleRate);
    mDL1Attribute = new AudioStreamAttribute();
    mDL1Out = new AudioDigtalI2S();
    m2ndI2SOut = new AudioDigtalI2S();
    mDaiBt = new AudioDigitalDAIBT();
    mAudioDigitalControl = AudioDigitalControlFactory::CreateAudioDigitalControl();
    mAudioAnalogControl = AudioAnalogControlFactory::CreateAudioAnalogControl();
    mAudioResourceManager = AudioResourceManagerFactory::CreateAudioResource();
    ASSERT(mAudioResourceManager != NULL);

    mAudioSpeechEnhanceInfoInstance = AudioSpeechEnhanceInfo::getInstance();
    ASSERT(mAudioSpeechEnhanceInfoInstance != NULL);


    mAudioSpeechEnhanceInfoInstance->SetStreamOutPointer(this);
    mEcho_reference = NULL;

    mDL1Attribute->mdevices = devices;
    ALOGD("format = %d, channels = %d, rate = %d", *format, *channels, *sampleRate);

    if (*format == AUDIO_FORMAT_PCM_16_BIT)
    {
        mDL1Attribute->mFormat = AUDIO_FORMAT_PCM_16_BIT;
    }
    else if (*format == AUDIO_FORMAT_PCM_8_BIT)
    {
        mDL1Attribute->mFormat = AUDIO_FORMAT_PCM_8_BIT;
    }
    else
    {
        ALOGE("Format is not a valid number");
        mDL1Attribute->mFormat = AUDIO_FORMAT_PCM_16_BIT;
    }

    if (*channels == AUDIO_CHANNEL_OUT_MONO)
    {
        mDL1Attribute->mChannels = 1;
    }
    else if (*channels == AUDIO_CHANNEL_OUT_STEREO)
    {
        mDL1Attribute->mChannels = 2;
    }
    else
    {
        ALOGE("Channelsis not a valid number");
        mDL1Attribute->mChannels = 2;
    }
    mDL1Attribute->mSampleRate = *sampleRate;

    // save the original sample rate & channels before SRC
    mSourceSampleRate  = *sampleRate;
    mSourceChannels    = mDL1Attribute->mChannels;

    mStarting = false;
    mSuspend = 0;
    mSteroToMono = false;
    mDL1Attribute->mBufferSize = 8192;
    mHwBufferSize = mAudioDigitalControl->GetMemBufferSize(AudioDigitalType::MEM_DL1);
    mLatency = (mHwBufferSize * 1000) / (mDL1Attribute->mSampleRate * mDL1Attribute->mChannels *
                                         (mDL1Attribute->mFormat == AUDIO_FORMAT_PCM_8_BIT ? 1 : 2));
    AudioVUnlockDL *VUnlockhdl = AudioVUnlockDL::getInstance();
    if (VUnlockhdl != NULL)
    {
        VUnlockhdl->GetStreamOutLatency(mLatency);
    }
    calInterrupttime();

    //#if defined(MTK_VIBSPK_SUPPORT)
    mVibSpk      = AudioVIBSPKControl::getInstance();
    mVibSpkFreq = AudioFtm::getInstance()->GetVibSpkCalibrationStatus();
    ALOGD("VibSpkReadFrequency:%x", mVibSpkFreq);
    if (mVibSpkFreq == 0)
    {
        SetVibSpkDefaultParam();//It should be set before creating AudioMTKFilterManager
        mVibSpkFreq = VIBSPK_DEFAULT_FREQ;
    }
    mVibSpk->setParameters(44100, mVibSpkFreq, MOD_FREQ, DELTA_FREQ);
    mVibSpkEnable = false;
    //#endif

    mBliSrc = new BliSrc();
    ASSERT(mBliSrc != NULL);

    mSwapBufferTwo = new uint8_t[bufferSize()];
    ASSERT(mSwapBufferTwo != NULL);

    mFilters = new AudioMTKFilterManager(mDL1Attribute->mSampleRate, mDL1Attribute->mChannels,
                                         mDL1Attribute->mFormat, mDL1Attribute->mBufferSize);
    ASSERT(mFilters != NULL);

    mBliSrcVoIP = NULL;
    mBliSrcVoIP = new BliSrc();

    mSwapBufferVoIP = NULL;
    mSwapBufferVoIP = new uint8_t[bufferSize()];
    if (mSwapBufferVoIP == NULL)
    {
        ALOGE("mSwapBufferVoIP for BliSRCVoIP allocate fail!!! \n");
    }
    
    mPresentedBytes = 0;
    mForceStandby = false;
    *status = NO_ERROR;
    ALOGD("-AudioMTKStreamOut constructor");
}

AudioMTKStreamOut::~AudioMTKStreamOut()
{
    ALOGD("AudioMTKStreamOut desstructor");

    if (mFilters)
    {
        mFilters->stop();
        delete mFilters;
        mFilters = NULL;
    }
    if (mBliSrc)
    {
        mBliSrc->close();
        delete mBliSrc;
        mBliSrc = NULL;
    }
    if (mSwapBufferTwo)
    {
        delete []mSwapBufferTwo;
        mSwapBufferTwo = NULL;
    }
    if (mEcho_reference)
    {
        mEcho_reference = NULL;
    }

    if (mBliSrcVoIP)
    {
        mBliSrcVoIP->close();
        delete mBliSrcVoIP;
        mBliSrcVoIP = NULL;
    }
    if (mSwapBufferVoIP)
    {
        delete []mSwapBufferVoIP;
        mSwapBufferVoIP = NULL;
    }

    if (mFd2 > 0)
    {
        ::close(mFd2);
        mFd2 = 0;
    }
}

uint32_t AudioMTKStreamOut::calInterrupttime()
{
    int SampleCount = mHwBufferSize / mDL1Attribute->mChannels;
    ALOGD("calInterrupttime bufferSize = %d mDL1Attribute->mChannels = %d", bufferSize(), mDL1Attribute->mChannels);

    if (mDL1Attribute->mFormat == AUDIO_FORMAT_PCM_16_BIT)
    {
        SampleCount = SampleCount >> 1;
    }

    mDL1Attribute->mInterruptSample =  SampleCount >> 1;
    ALOGD("calInterrupttime mInterruptCounter = %d", mDL1Attribute->mInterruptSample);
    return  mDL1Attribute->mInterruptSample ;
}

status_t AudioMTKStreamOut::SetMEMIFAttribute(AudioDigitalType::Digital_Block Mem_IF, AudioStreamAttribute *Attribute)
{
    ALOGD("SetMEMIFAttribute Mem_IF = %d sampleRate = %d mInterruptSample = %d", Mem_IF, Attribute->mSampleRate, Attribute->mInterruptSample);

    mAudioDigitalControl->SetMemIfSampleRate(Mem_IF, Attribute->mSampleRate);
    mAudioDigitalControl->SetMemIfChannelCount(Mem_IF, Attribute->mChannels);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::SetMEMIFEnable(AudioDigitalType::Digital_Block Mem_IF, bool bEnable)
{
    ALOGD("SetMEMIFEnable Mem_IF = %d bEnable = %d", Mem_IF, bEnable);
    mAudioDigitalControl->SetMemIfEnable(Mem_IF, bEnable);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::SetI2SOutDACAttribute()
{
    ALOGD("SetI2SOutDACAttribute");
    mDL1Out->mLR_SWAP = AudioDigtalI2S::NO_SWAP;
    mDL1Out->mI2S_SLAVE = AudioDigtalI2S::MASTER_MODE;
    mDL1Out->mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
    mDL1Out->mI2S_FMT = AudioDigtalI2S::I2S;
    mDL1Out->mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
    mDL1Out->mI2S_SAMPLERATE = sampleRate();
    mAudioDigitalControl->SetI2SDacOut(mDL1Out);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::Set2ndI2SOutAttribute()
{
    ALOGD("Set2ndI2SOutAttribute");
    m2ndI2SOut->mLR_SWAP = AudioDigtalI2S::NO_SWAP;
    m2ndI2SOut->mI2S_SLAVE = AudioDigtalI2S::MASTER_MODE;
    m2ndI2SOut->mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
    m2ndI2SOut->mI2S_FMT = AudioDigtalI2S::I2S;
    m2ndI2SOut->mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
    m2ndI2SOut->mI2S_SAMPLERATE = sampleRate();
    mAudioDigitalControl->Set2ndI2SOut(m2ndI2SOut);
    return NO_ERROR;
}


status_t AudioMTKStreamOut::Set2ndI2SOutAttribute(
    AudioDigtalI2S::LR_SWAP LRswap ,
    AudioDigtalI2S::I2S_SRC mode ,
    AudioDigtalI2S::INV_LRCK inverse,
    AudioDigtalI2S::I2S_FORMAT format,
    AudioDigtalI2S::I2S_WLEN Wlength,
    int samplerate)
{
    ALOGD("Set2ndI2SOutAttribute with dedicated define");
    m2ndI2SOut->mLR_SWAP = LRswap;
    m2ndI2SOut->mI2S_SLAVE = mode;
    m2ndI2SOut->mINV_LRCK = inverse;
    m2ndI2SOut->mI2S_FMT = format;
    m2ndI2SOut->mI2S_WLEN = Wlength;
    m2ndI2SOut->mI2S_SAMPLERATE = samplerate;
    mAudioDigitalControl->Set2ndI2SOut(m2ndI2SOut);
    return NO_ERROR;
}


status_t AudioMTKStreamOut::SetI2SDACOut(bool bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    mAudioDigitalControl->SetI2SDacEnable(bEnable);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::Set2ndI2SOut(bool bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
#if 1
    mAudioDigitalControl->Set2ndI2SOutEnable(bEnable);
#else
    mAudioDigitalControl->Set2ndI2SEnable(bEnable);
#endif
    return NO_ERROR;
}

status_t AudioMTKStreamOut::SetDAIBTAttribute()
{
    // fix me , ned to base on actual situation
    if (WCNChipController::GetInstance()->IsBTMergeInterfaceSupported() == true)
    {
        mDaiBt->mUSE_MRGIF_INPUT = AudioDigitalDAIBT::FROM_MGRIF;
    }
    else
    {
        mDaiBt->mUSE_MRGIF_INPUT = AudioDigitalDAIBT::FROM_BT;
    }
    mDaiBt->mDAI_BT_MODE = (WCNChipController::GetInstance()->BTChipSamplingRate()) ? (AudioDigitalDAIBT::Mode16K) : (AudioDigitalDAIBT::Mode8K);
    mDaiBt->mDAI_DEL = AudioDigitalDAIBT::HighWord;
    mDaiBt->mBT_LEN  = WCNChipController::GetInstance()->BTChipSyncLength();
    mDaiBt->mDATA_RDY = true;
    mDaiBt->mBT_SYNC = WCNChipController::GetInstance()->BTChipSyncFormat();
    mDaiBt->mBT_ON = true;
    mDaiBt->mDAIBT_ON = false;
    mAudioDigitalControl->SetDAIBBT(mDaiBt);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::SetDAIBTOut(bool bEnable)
{
    mAudioDigitalControl->SetDAIBTEnable(bEnable);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::SetIMcuIRQ(AudioDigitalType::IRQ_MCU_MODE IRQ_mode, AudioStreamAttribute *Attribute)
{
    ALOGD("SetIMcuIRQ1 IRQ_mode = %d sampleRate = %d mInterruptSample = %d", IRQ_mode, Attribute->mSampleRate, Attribute->mInterruptSample);
    mAudioDigitalControl->SetIrqMcuSampleRate(IRQ_mode, Attribute->mSampleRate);
    mAudioDigitalControl->SetIrqMcuCounter(IRQ_mode , Attribute->mInterruptSample);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::EnableIMcuIRQ(AudioDigitalType::IRQ_MCU_MODE IRQ_mode , bool bEnable)
{
    ALOGD("EnableIMcuIRQ1 IRQ_mode = %d bEnable = %d", IRQ_mode, bEnable);
    mAudioDigitalControl->SetIrqMcuEnable(IRQ_mode, bEnable);
    return NO_ERROR;
}

uint32_t AudioMTKStreamOut::sampleRate() const
{
    return mDL1Attribute->mSampleRate;
}

size_t AudioMTKStreamOut::bufferSize() const
{
    return mDL1Attribute->mBufferSize;
}

uint32_t AudioMTKStreamOut::channels() const // TODO(Harvey): WHY? 1 & 2 ??? or 0x1 & 0x3 ???
{
    if (mDL1Attribute->mChannels == 1)
    {
        return AUDIO_CHANNEL_OUT_MONO;
    }
    else
    {
        return AUDIO_CHANNEL_OUT_STEREO;
    }
    return AUDIO_CHANNEL_OUT_STEREO;
}

int AudioMTKStreamOut::format() const
{
    return mDL1Attribute->mFormat;
}

uint32_t AudioMTKStreamOut::latency() const
{
    return mLatency;
}

status_t AudioMTKStreamOut::setVolume(float left, float right)
{
    return NO_ERROR;
}

status_t AudioMTKStreamOut::RequesetPlaybackclock()
{
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
    return NO_ERROR;
}
status_t AudioMTKStreamOut::ReleasePlaybackclock()
{
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
    mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);
    return NO_ERROR;
}

status_t AudioMTKStreamOut::SetPlayBackPinmux()
{
    ALOGD("SetPlayBackPinmux set audio pinmux");

    uint32 DlDevice = mAudioResourceManager->getDlOutputDevice();
    if ((DlDevice & AUDIO_DEVICE_OUT_WIRED_HEADSET) || (DlDevice & AUDIO_DEVICE_OUT_WIRED_HEADPHONE))
    {
        mAudioAnalogControl->AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::MUX_AUDIO);
        mAudioAnalogControl->AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETL, AudioAnalogType::MUX_AUDIO);
        mAudioAnalogControl->AnalogSetMux(AudioAnalogType::DEVICE_OUT_SPEAKERR, AudioAnalogType::MUX_AUDIO);
        mAudioAnalogControl->AnalogSetMux(AudioAnalogType::DEVICE_OUT_SPEAKERL, AudioAnalogType::MUX_AUDIO);
    }
    if (DlDevice & AUDIO_DEVICE_OUT_EARPIECE)
    {
        mAudioAnalogControl->AnalogSetMux(AudioAnalogType::DEVICE_OUT_EARPIECEL, AudioAnalogType::MUX_VOICE);
    }

    return NO_ERROR;
}

bool AudioMTKStreamOut::DoStereoMonoConvert(void *buffer, size_t byte)
{
    //ALOGD("DoStereoMonoConvert mSteroToMono = %d",mSteroToMono);
    if (mSteroToMono == true)
    {
        short *Sample = (short *)buffer;
        int FinalValue  = 0;
        while (byte > 0)
        {
            FinalValue = ((*Sample) + (*(Sample + 1)));
            *Sample++ = FinalValue >> 1;
            *Sample++ = FinalValue >> 1;
            byte -= 4;
        }
    }
    return true;
}

status_t AudioMTKStreamOut::SetStereoToMonoFlag(int device)
{
    if ((device & AUDIO_DEVICE_OUT_SPEAKER) && (IsStereoSpeaker() == false))
    {
        mSteroToMono = true;
    }
    else
    {
        mSteroToMono = false;
    }
    return NO_ERROR;
}

void  AudioMTKStreamOut::OpenPcmDumpFile()
{
    char Buf[10];
    sprintf(Buf, "%d.pcm", DumpFileNum);
    if (mPDacPCMDumpFile == NULL)
    {
        DumpFileName = String8(streamout);
        DumpFileName.append((const char *)Buf);
        ALOGD("mPDacPCMDumpFile DumpFileName = %s", DumpFileName.string());
        mPDacPCMDumpFile = AudioOpendumpPCMFile(DumpFileName.string(), streamout_propty);
    }
    if (mPFinalPCMDumpFile == NULL)
    {
        DumpFileName = String8(streamoutfinal);
        DumpFileName.append((const char *)Buf);
        ALOGD("mPFinalPCMDumpFile DumpFileName = %s", DumpFileName.string());
        mPFinalPCMDumpFile = AudioOpendumpPCMFile(DumpFileName.string(), streamout_propty);
    }
    //#if defined(MTK_VIBSPK_SUPPORT)
    if (mVIBsignalDumpFile == NULL)
    {
        DumpFileName = String8(streamout_vibsignal);
        DumpFileName.append((const char *)Buf);
        ALOGD("mVIBsignalDumpFile DumpFileName = %s", DumpFileName.string());
        mVIBsignalDumpFile = AudioOpendumpPCMFile(DumpFileName.string(), streamout_propty);
    }
    if (mLoudNotchDumpFile == NULL)
    {
        DumpFileName = String8(streamout_notch);
        DumpFileName.append((const char *)Buf);
        ALOGD("mLoudNotchDumpFile DumpFileName = %s", DumpFileName.string());
        mLoudNotchDumpFile = AudioOpendumpPCMFile(DumpFileName.string(), streamout_propty);
    }
    //#endif

    DumpFileNum++;
    DumpFileNum %= MAX_NUM_FILE;
}

void  AudioMTKStreamOut::ClosePcmDumpFile()
{
    AudioCloseDumpPCMFile(mPDacPCMDumpFile);
    AudioCloseDumpPCMFile(mPFinalPCMDumpFile);
    mPDacPCMDumpFile = NULL;
    mPFinalPCMDumpFile = NULL;
    //#if defined(MTK_VIBSPK_SUPPORT)
    AudioCloseDumpPCMFile(mVIBsignalDumpFile);
    AudioCloseDumpPCMFile(mLoudNotchDumpFile);
    mVIBsignalDumpFile = NULL;
    mLoudNotchDumpFile = NULL;
    //#endif

}

int AudioMTKStreamOut::dumpPcm(FILE *fp, const void *buffer, size_t bytes)
{
#if 0
    int written_data = 0;
    if (fp)
    {
        long int position = 0;
        position = ftell(fp);
        ALOGD("position = %d", position);
        if (position > MAX_FILE_LENGTH)
        {
            rewind(fp);
        }
        written_data = fwrite((void *)buffer, 1, bytes, fp);
    }
    return written_data;
#else
    if (fp)
    {
        AudioDumpPCMData((void *)buffer, (uint32_t)bytes, fp);
        return (int)bytes;
    }
    else
    {
        return 0;
    }
#endif
}


bool AudioMTKStreamOut::IsStereoSpeaker()
{
#ifdef ENABLE_STEREO_SPEAKER
    return true;
#else
    return false;
#endif
}

timespec AudioMTKStreamOut::GetSystemTime(bool print)
{
    struct timespec systemtime;
    int rc;
    rc = clock_gettime(CLOCK_MONOTONIC, &systemtime);
    if (rc != 0)
    {
        systemtime.tv_sec  = 0;
        systemtime.tv_nsec = 0;
        ALOGD("clock_gettime error");
    }
    if (print == true)
    {
        ALOGD("GetSystemTime, sec %ld nsec %ld", systemtime.tv_sec, systemtime.tv_nsec);
    }

    return systemtime;
}

bool AudioMTKStreamOut::NeedAFEDigitalAnalogControl(uint32 DigitalPart)
{
#if defined(BTCVSD_ENC_DEC_LOOPBACK) || defined(BTCVSD_KERNEL_LOOPBACK)
    // BTCVSD TEST: turn on irq 1 INTERRUPT_IRQ1_MCU for simulate BT interrupt
    // AFE_IRQ_CON does not support 64000 samplerate, use 1080/48000=22.5ms to simulate BTCVSD interrupt
    mDL1Attribute->mSampleRate = 48000;
    mDL1Attribute->mInterruptSample = 1080;
#endif

#if 0
#if defined(BTCVSD_ENC_DEC_LOOPBACK) || defined(BTCVSD_KERNEL_LOOPBACK) || defined(BTCVSD_LOOPBACK_WITH_CODEC)
    return false; // if test by AFE IRQ (TEST_USE_AFE_IRQ in kernel ), need to enable this condition
#else
    if ((WCNChipController::GetInstance()->BTUseCVSDRemoval() == true) &&
        (DigitalPart == AudioDigitalType::DAI_BT)) //no need to enable IRQ1 , AFE, MemIntf, analogDev  in BTCVSD case
    {
        return false;
    }
#endif
#else
    if (AudioLoopbackController::GetInstance()->IsAPBTLoopbackWithCodec() == true)
    {
        return false;
    }

    if ((WCNChipController::GetInstance()->BTUseCVSDRemoval() == true) &&
        (DigitalPart == AudioDigitalType::DAI_BT)) //no need to enable IRQ1 , AFE, MemIntf, analogDev  in BTCVSD case
    {
        return false;
    }

#endif

    return true;
}

ssize_t AudioMTKStreamOut::write(const void *buffer, size_t bytes)
{
    int ret = 0;
    ALOGV("%s(), buffer = %p bytes = %d mLatency = %d", __FUNCTION__, buffer, bytes, mLatency);
    // here need write data to hardware
    ssize_t WrittenBytes = 0;
    void *outbuffer = const_cast<void *>(buffer);

    if (mSuspend || (LoopbackManager::GetInstance()->GetLoopbackType() != NO_LOOPBACK && LoopbackManager::GetInstance()->GetLoopbackType() != AP_BT_LOOPBACK))
    {
        usleep(mLatency * 1000);//slee for a while
        ALOGD("%s() suspend write", __FUNCTION__);
        return bytes;
    }
    AudioVUnlockDL *VUnlockhdl = AudioVUnlockDL::getInstance();
    // need lock first
    ret =  mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
    if (ret)
    {
        ALOGW("write EnableAudioLock  AUDIO_HARDWARE_LOCK fail");
        usleep(50 * 1000);
        return bytes;
    }
    ret = mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
    if (ret)
    {
        mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
        ALOGW("write EnableAudioLock AUDIO_STREAMOUT_LOCK fail");
        usleep(50 * 1000);
        return bytes;
    }

    audio_mode_t Mode = mAudioResourceManager->GetAudioMode();


    //staring DL digital part.
    if (GetStreamRunning() == false)
    {

        ALOGD("write mStarting == false , first start");
#ifndef EXTMD_LOOPBACK_TEST
        RequesetPlaybackclock();
        OpenPcmDumpFile();
        SetStreamRunning(true);
        UpdateLine(__LINE__);
        switch (Mode)
        {
            case AUDIO_MODE_NORMAL:
            case AUDIO_MODE_RINGTONE:
            case AUDIO_MODE_IN_COMMUNICATION:
            {
                ::ioctl(mFd, START_MEMIF_TYPE, AudioDigitalType::MEM_DL1); // fp for write indentify
                uint32 DigitalPart = mAudioDigitalControl->DlPolicyByDevice(mDL1Attribute->mdevices);
                SetAnalogFrequency(DigitalPart);
                SetPlayBackPinmux();

                DoStereoMonoConvert((void *)outbuffer, bytes);

                if (DigitalPart == AudioDigitalType::DAI_BT)
                {
                    UpdateLine(__LINE__);
                    if (WCNChipController::GetInstance()->BTUseCVSDRemoval() != true)
                    {
                        WrittenBytes = WriteDataToBTSCOHW(outbuffer, bufferSize());
                    }
                    else
                    {
                        if (NeedToDoBTSCOProcess(DigitalPart))
                        {
                            mAudioBTCVSDControl->BTCVSD_Init(mFd2, mSourceSampleRate, mSourceChannels);
                            mCVSDTXoutDumpFile = NULL;
                            mCVSDTXoutDumpFile = AudioOpendumpPCMFile("/sdcard/mtklog/audio_dump/cvsdtxout.pcm", "cvsdtxout.pcm.dump");
                            WrittenBytes = WriteDataToBTSCOHW(outbuffer, bytes);
                        }
                    }
                }
                else
                {
                    UpdateLine(__LINE__);
#ifdef EARLY_PORTING_VERIFY_USE
                    WrittenBytes =::write(mFd, outbuffer, bufferSize());
#else
                    WrittenBytes = WriteDataToAudioHW(outbuffer, bufferSize());
#endif

                    ALOGD("playback first write finish time");
                    GetSystemTime(true);


                    if (VUnlockhdl != NULL)
                    {
                        VUnlockhdl->SetInputStandBy(false);
                        VUnlockhdl-> GetSRCInputParameter(mDL1Attribute->mSampleRate, mDL1Attribute->mChannels);
                        VUnlockhdl->GetStreamOutLatency(mLatency);
                    }
                    //WrittenBytes =::write(mFd, buffer, bufferSize());
                }

                if (NeedAFEDigitalAnalogControl(DigitalPart))
                {
                    TurnOnAfeDigital(DigitalPart);
                    mAudioDigitalControl->SetAfeEnable(true);

                    mAudioResourceManager->StartOutputDevice();  // open analog device

                    // turn on irq 1 INTERRUPT_IRQ1_MCU
                    SetIMcuIRQ(AudioDigitalType::IRQ1_MCU_MODE, mDL1Attribute);
                    SetMEMIFAttribute(AudioDigitalType::MEM_DL1, mDL1Attribute);

                    SetMEMIFEnable(AudioDigitalType::MEM_DL1, true);
                    EnableIMcuIRQ(AudioDigitalType::IRQ1_MCU_MODE, true);
                }                
                mAudioSpeechEnhanceInfoInstance->GetDownlinkIntrStartTime();
                UpdateLine(__LINE__);

                if (VUnlockhdl != NULL)
                {
                    VUnlockhdl->GetFirstDLTime();
                }
                mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK);
                mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
                return WrittenBytes;
            }
            case AUDIO_MODE_IN_CALL:
            case AUDIO_MODE_IN_CALL_2:
            case AUDIO_MODE_IN_CALL_EXTERNAL:
            {
                SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();
                BGSPlayer *pBGSPlayer = BGSPlayer::GetInstance();
                pBGSPlayer->mBGSMutex.lock();
                pBGSPlayer->CreateBGSPlayBuffer(mSourceSampleRate , mSourceChannels, format()); // TODO(Harvey): use channels() // Set target sample rate = 16000 Hz
                pBGSPlayer->Open(pSpeechDriver, 0x0, 0xFF);
                pBGSPlayer->mBGSMutex.unlock();
                break;
            }
            default:
            {
                break;
            }
        }
#else
        BTCVSD_ExtMDLoopBackTest_Init();
#endif
        ALOGD("AudioMTKStreamOut::write first running (-)");
    }
    else
    {

    }
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
#ifndef EXTMD_LOOPBACK_TEST
    dumpPcm(mPDacPCMDumpFile, outbuffer, bytes);
    UpdateLine(__LINE__);
    switch (Mode)
    {
        case AUDIO_MODE_NORMAL:
        case AUDIO_MODE_RINGTONE:
        case AUDIO_MODE_IN_COMMUNICATION:
        {
            uint32 DigitalPart = mAudioDigitalControl->DlPolicyByDevice(mDL1Attribute->mdevices);

            DoStereoMonoConvert((void *)outbuffer, bytes);
            if (WCNChipController::GetInstance()->BTUseCVSDRemoval() != true)
            {
                if (DigitalPart == AudioDigitalType::DAI_BT)
                {
                    WrittenBytes = WriteDataToBTSCOHW(outbuffer, bufferSize());
                }
                else
                {
                    //WrittenBytes =::write(mFd, buffer, bufferSize());
                    VUnlockhdl->SetInputStandBy(false);
                    WrittenBytes = WriteDataToAudioHW(outbuffer, bufferSize());
                }
            }
            else
            {
                if (NeedToDoBTSCOProcess(DigitalPart))
                {
                    WrittenBytes = WriteDataToBTSCOHW(outbuffer, bytes);
                }
                else
                {
                    //WrittenBytes =::write(mFd, buffer, bufferSize());
                    VUnlockhdl->SetInputStandBy(false);
                    WrittenBytes = WriteDataToAudioHW(outbuffer, bufferSize());
                }
            }
            break;
        }
        case AUDIO_MODE_IN_CALL:
        case AUDIO_MODE_IN_CALL_2:
        case AUDIO_MODE_IN_CALL_EXTERNAL:
        {
            WrittenBytes = BGSPlayer::GetInstance()->Write(const_cast<void *>(outbuffer), bytes);
            break;
        }
        default:
        {
            break;
        }
    }
#else
    usleep(50 * 1000);
    WrittenBytes = bytes;
#endif
    UpdateLine(__LINE__);
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK);
    ALOGD("AudioMTKStreamOut::write (-)");

    return WrittenBytes;
}

ssize_t AudioMTKStreamOut::WriteDataToBTSCOHW(const void *buffer, size_t bytes)
{
    ssize_t WrittenBytes = 0;
    size_t outputSize = 0;
    uint8_t *outbuffer, *inbuf, *workbuf, i;
    uint32_t insize, outsize, workbufsize, total_outsize, src_fs_s, original_insize;
    original_insize = bytes;

    if (WCNChipController::GetInstance()->BTUseCVSDRemoval() == true)
    {
        inbuf = (uint8_t *)buffer;
        struct timespec TimeInfo = GetSystemTime(false);
        writeDataToEchoReference(buffer, bytes, NO_REMAIN_DATA_INFO, TimeInfo);
        do
        {
            outbuffer = mAudioBTCVSDControl->BT_SCO_TX_GetCVSDOutBuf();
            outsize = SCO_TX_ENCODE_SIZE;
            insize = bytes;
            workbuf = mAudioBTCVSDControl->BT_SCO_TX_GetCVSDWorkBuf();
            workbufsize = SCO_TX_PCM64K_BUF_SIZE;
            src_fs_s = mSourceSampleRate;//source sample rate for SRC
            total_outsize = 0;
            i = 0;
            do
            {
#if defined(__MSBC_CODEC_SUPPORT__)
                if (mAudioBTCVSDControl->BT_SCO_isWideBand())
                {
                    mAudioBTCVSDControl->btsco_process_TX_MSBC(inbuf, &insize, outbuffer, &outsize, workbuf, workbufsize, src_fs_s); //return insize is consumed size
                    ALOGD("WriteDataToBTSCOHW, do mSBC encode outsize=%d, consumed size=%d, bytes=%d", outsize, insize, bytes);
                }
                else
#endif
                {
                    mAudioBTCVSDControl->btsco_process_TX_CVSD(inbuf, &insize, outbuffer, &outsize, workbuf, workbufsize, src_fs_s); //return insize is consumed size
                    ALOGVV("WriteDataToBTSCOHW, do CVSD encode outsize=%d, consumed size=%d, bytes=%d", outsize, insize, bytes);
                }

                outbuffer += outsize;
                inbuf += insize;
                bytes -= insize;
                insize = bytes;
                ASSERT(bytes >= 0);
                total_outsize += outsize;
                i++;
            }
            while ((total_outsize < BTSCO_CVSD_TX_OUTBUF_SIZE) && (outsize != 0));

#if defined(BTCVSD_ENC_DEC_LOOPBACK) && !defined(BTCVSD_KERNEL_LOOPBACK) //TEST CODE:write user space TX encode data to RX
            mAudioBTCVSDControl->BTCVSD_Test_UserSpace_TxToRx(total_outsize);
#else //BTCVSD normal path, write to kernel
            if (mCVSDTXoutDumpFile)
            {
                fwrite((void *)(mAudioBTCVSDControl->BT_SCO_TX_GetCVSDOutBuf()), 1, total_outsize, mCVSDTXoutDumpFile);
            }
            ALOGD("WriteDataToBTSCOHW write to kernel(+) total_outsize=%d", total_outsize);
            if (total_outsize)
            {
                WrittenBytes =::write(mFd2, mAudioBTCVSDControl->BT_SCO_TX_GetCVSDOutBuf(), total_outsize);  //total_outsize should be BTSCO_CVSD_TX_OUTBUF_SIZE!!!
            }
            ALOGD("WriteDataToBTSCOHW write to kernel(-) remaining bytes=%d", bytes);
#endif

        }
        while (bytes > 0);
        return original_insize;
    }
    else
    {
        outputSize = DoBTSCOSRC(buffer, bytes, (void **)&outbuffer);
        WrittenBytes =::write(mFd, outbuffer, outputSize);
        struct timespec TimeInfo = GetSystemTime(false);
        writeDataToEchoReference(buffer, bytes, NO_REMAIN_DATA_INFO, TimeInfo);
        return WrittenBytes;
    }
}

ssize_t AudioMTKStreamOut::DoBTSCOSRC(const void *buffer, size_t bytes, void **outbuffer)
{
    size_t outputSize = 0;

    if (mBliSrc)
    {
        if (mBliSrc->initStatus() != OK)
        {
            // 6628 only support 8k BTSCO
            ALOGD("DoBTSCOSRC Init BLI_SRC,mDL1Attribute->mSampleRate=%d, target=8000", mDL1Attribute->mSampleRate);
            mBliSrc->init(44100, mDL1Attribute->mChannels, 8000, mDL1Attribute->mChannels);
        }
        *outbuffer = mSwapBufferTwo;
        outputSize = mBliSrc->process(buffer, bytes, *outbuffer);
        if (outputSize <= 0)
        {
            outputSize = bytes;
            *outbuffer = (void *)buffer;
        }
        return outputSize;
    }
    else
    {
        ALOGW("DoBTSCOSRC() mBliSrc=NULL!!!");
        *outbuffer = (void *)buffer;
        return bytes;
    }
}

ssize_t AudioMTKStreamOut::DoVoIPSRC(const void *buffer, size_t bytes, void **outbuffer)
{
    size_t outputSize = 0;

    if (mBliSrcVoIP)
    {
        if (mBliSrcVoIP->initStatus() != OK)
        {
            // VoIP only support 16k mono downlink data
            ALOGD("DoVoIPSRC Init BLI_SRC,mDL1Attribute->mSampleRate=%d, target=16000, stereo", mDL1Attribute->mSampleRate);
            mBliSrcVoIP->init(44100, mDL1Attribute->mChannels, 16000, 1);
        }
        *outbuffer = mSwapBufferVoIP;
        outputSize = mBliSrcVoIP->process(buffer, bytes, *outbuffer);
        if (outputSize <= 0)
        {
            outputSize = bytes;
            *outbuffer = (void *)buffer;
        }
        return outputSize;
    }
    else
    {
        ALOGD("DoVoIPSRC() mBliSrcVoIP=NULL!!!");
        *outbuffer = (void *)buffer;
        return bytes;
    }
}

status_t AudioMTKStreamOut::setForceStandby(bool bEnable)
{
    ALOGD("+%s(), bEnable = %d\n", __FUNCTION__, bEnable);
    mForceStandby = bEnable;
    return NO_ERROR;
}

status_t AudioMTKStreamOut::standby()
{
    ALOGD("+AudioMTKStreamOut standby");
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
    ALOGD("+AudioMTKStreamOut standby() EnableAudioLock AUDIO_STREAMOUT_LOCK");
    AudioVUnlockDL *VUnlockhdl = AudioVUnlockDL::getInstance();
    //staring DL digital part.
    //mPresentedBytes = 0;
    if (GetStreamRunning() == true)
    {
        SetStreamRunning(false);
        ClosePcmDumpFile();
#ifndef EXTMD_LOOPBACK_TEST
        switch (mAudioResourceManager->GetAudioMode())
        {
            case AUDIO_MODE_NORMAL:
            case AUDIO_MODE_RINGTONE:
            case AUDIO_MODE_IN_COMMUNICATION:
            {
                if (AudioFMController::GetInstance()->GetFmEnable() == true ||
                    AudioMATVController::GetInstance()->GetMatvEnable() == true)
                {
                    ALOGD("%s(), FM/mATV is on, do nothing!!!!!!", __FUNCTION__);
                }
                else
                {
                    mAudioResourceManager->StopOutputDevice();
                }
                uint32 DigitalPart = mAudioDigitalControl->DlPolicyByDevice(mAudioResourceManager->getDlOutputDevice());

                if (WCNChipController::GetInstance()->BTUseCVSDRemoval() == true)
                {
                    if (NeedToDoBTSCOProcess(DigitalPart))
                    {
                        if (mCVSDTXoutDumpFile)
                        {
                            AudioCloseDumpPCMFile(mCVSDTXoutDumpFile);
                            ALOGD("ClosePcmDumpFile mCVSDTXoutDumpFile");
                        }
                        mAudioBTCVSDControl->BTCVSD_StandbyProcess(mFd2);
                    }
                    else
                    {
                        TurnOffAfeDigital(DigitalPart, AudioFMController::GetInstance()->GetFmEnable() || AudioMATVController::GetInstance()->GetMatvEnable());
                        SetMEMIFEnable(AudioDigitalType::MEM_DL1, false); // disable irq 1
                        EnableIMcuIRQ(AudioDigitalType::IRQ1_MCU_MODE, false);
                        if (AudioFMController::GetInstance()->GetFmEnable() == false &&
                            AudioMATVController::GetInstance()->GetMatvEnable() == false)
                        {
                            mAudioDigitalControl->SetAfeEnable(false);
                        }
                        ::ioctl(mFd, STANDBY_MEMIF_TYPE, AudioDigitalType::MEM_DL1); // disable mem interface mem1
                        if (VUnlockhdl != NULL)
                        {
                            VUnlockhdl->SetInputStandBy(true);
                        }
                    }
                }
                else
                {
                    TurnOffAfeDigital(DigitalPart, AudioFMController::GetInstance()->GetFmEnable() || AudioMATVController::GetInstance()->GetMatvEnable());
                    SetMEMIFEnable(AudioDigitalType::MEM_DL1, false); // disable irq 1
                    EnableIMcuIRQ(AudioDigitalType::IRQ1_MCU_MODE, false);
                    if (AudioFMController::GetInstance()->GetFmEnable() == false &&
                        AudioMATVController::GetInstance()->GetMatvEnable() == false)
                    {
                        mAudioDigitalControl->SetAfeEnable(false);
                    }
                    ::ioctl(mFd, STANDBY_MEMIF_TYPE, AudioDigitalType::MEM_DL1); // disable mem interface mem1
                    if (VUnlockhdl != NULL)
                    {
                        VUnlockhdl->SetInputStandBy(true);
                    }
                }
                break;
            }

            case AUDIO_MODE_IN_CALL:
            case AUDIO_MODE_IN_CALL_2:
            case AUDIO_MODE_IN_CALL_EXTERNAL:
            {
                SpeechDriverInterface *pSpeechDriver = SpeechDriverFactory::GetInstance()->GetSpeechDriver();
                BGSPlayer *pBGSPlayer = BGSPlayer::GetInstance();
                pBGSPlayer->mBGSMutex.lock();
                pBGSPlayer->Close();
                pBGSPlayer->DestroyBGSPlayBuffer();
                pBGSPlayer->mBGSMutex.unlock();
                break;
            }
        }
#if defined(BTCVSD_KERNEL_LOOPBACK)
        usleep(30 * 1000); // add delay to avoid audio afe clk is turn off before last ISR comes!(used in 6589 early porting)
#endif
        ReleasePlaybackclock();
        if (mFilters) { mFilters->stop(); }
        if (mBliSrc)
        {
            mBliSrc->close();
        }

        if (mBliSrcVoIP)
        {
            mBliSrcVoIP->close();
        }

        StopWriteDataToEchoReference();

#else
        BTCVSD_ExtMDLoopBackTest_Stadby();
#endif
    }
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK);
    ALOGD("-AudioMTKStreamOut standby");
    return NO_ERROR;
}


status_t AudioMTKStreamOut::SetAnalogFrequency(uint32 AfeDigital)
{
    ALOGD("SetAnalogFrequency AfeDigital = %d", AfeDigital);
    switch (AfeDigital)
    {
        case (AudioDigitalType::DAI_BT):
        {
            break;
        }
        case (AudioDigitalType::I2S_OUT_DAC):
        {
            mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_OUT_DAC, sampleRate());
            break;
        }
        case (AudioDigitalType::I2S_OUT_2):
        {
            mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_OUT_DAC, sampleRate());
            break;
        }
        default:
        {
            ALOGD("Turn on default I2S out DAC part");
        }
    }
    return NO_ERROR;
}

status_t AudioMTKStreamOut::TurnOnAfeDigital(uint32 AfeDigital)
{
    ALOGD("TurnOnAfeDigital AfeDigital = %d", AfeDigital);
    switch (AfeDigital)
    {
        case (AudioDigitalType::DAI_BT):
        {
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I05, AudioDigitalType::O02);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I06, AudioDigitalType::O02);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::ConnectionShift, AudioDigitalType::I05, AudioDigitalType::O02);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::ConnectionShift, AudioDigitalType::I06, AudioDigitalType::O02);
            SetDAIBTAttribute();
            SetDAIBTOut(true);
            // turn on dai_out
            break;
        }
        case (AudioDigitalType::I2S_OUT_DAC):
        {
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I05, AudioDigitalType::O03);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I06, AudioDigitalType::O04);
            SetMEMIFEnable(AudioDigitalType::I2S_OUT_DAC, true);
            // turn on DAC_I2S out
            SetI2SOutDACAttribute();
            SetI2SDACOut(true);
            break;
        }
        case (AudioDigitalType::I2S_OUT_2):
        {
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I05, AudioDigitalType::O00);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I06, AudioDigitalType::O01);
            SetMEMIFEnable(AudioDigitalType::I2S_OUT_2, true);
            // turn on 2nd_I2S out
#if defined(HDMI_2NDI2S_32BIT)
            // MT8193 need 64*fs , wordlength =32 bits.
            Set2ndI2SOutAttribute(AudioDigtalI2S::NO_SWAP, AudioDigtalI2S::MASTER_MODE, AudioDigtalI2S::NO_INVERSE, AudioDigtalI2S::I2S, AudioDigtalI2S::WLEN_32BITS, sampleRate());
#else
            Set2ndI2SOutAttribute();
#endif
            Set2ndI2SOut(true);
            break;
        }
        default:
        {
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I05, AudioDigitalType::O03);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I06, AudioDigitalType::O04);
            SetI2SOutDACAttribute();
            SetI2SDACOut(true);
            ALOGD("Turn on default I2S out DAC part");
        }
    }
    return NO_ERROR;
}

status_t AudioMTKStreamOut::TurnOffAfeDigital(uint32 AfeDigital, bool keepDacOpen)
{
    ALOGD("TurnOffAfeDigital AfeDigital = %d", AfeDigital);
    switch (AfeDigital)
    {
        case (AudioDigitalType::DAI_BT):
        {
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I05, AudioDigitalType::O02);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I06, AudioDigitalType::O02);
            SetDAIBTOut(false);
            // turn off dai_out
            break;
        }
        case (AudioDigitalType::I2S_OUT_2):
        {
            SetMEMIFEnable(AudioDigitalType::I2S_OUT_2, false);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I05, AudioDigitalType::O00);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I06, AudioDigitalType::O01);
            Set2ndI2SOut(false);
            break;
        }
        case (AudioDigitalType::I2S_OUT_DAC):
        {
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I05, AudioDigitalType::O03);
            mAudioDigitalControl->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I06, AudioDigitalType::O04);
            SetMEMIFEnable(AudioDigitalType::I2S_OUT_DAC, false);
            if (!keepDacOpen)
            {
                SetI2SDACOut(false);
            }
            break;
        }
        default:
        {
            ALOGD("TurnOffAfeDigital no setting is available");
        }
    }
    return NO_ERROR;
}

void AudioMTKStreamOut::dokeyRouting(uint32_t new_device)
{
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_MODE_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS); // handle for output new_device change , need lock streamout first

    const uint32_t current_device = mAudioResourceManager->getDlOutputDevice();
    ALOGD("%s(), current_device = %d, new_device = %d", __FUNCTION__, current_device, new_device);

    SpeechPhoneCallController::GetInstance()->SetRoutingForTty((audio_devices_t)new_device);

    AudioFMController *pAudioFMController = AudioFMController::GetInstance();

    // When FM + (WFD, A2DP, SCO(44.1K -> 8/16K), ...), Policy will routing to AUDIO_DEVICE_NONE
    // Hence, use other device like AUDIO_DEVICE_OUT_REMOTE_SUBMIX instead to achieve FM routing.
    if (new_device == AUDIO_DEVICE_NONE && pAudioFMController->GetFmEnable() == true)
    {
        ALOGD("%s(), Replace AUDIO_DEVICE_NONE with AUDIO_DEVICE_OUT_REMOTE_SUBMIX for AP-path FM routing", __FUNCTION__);
        new_device = AUDIO_DEVICE_OUT_REMOTE_SUBMIX;
    }

    // Check if no need to routing, return
    if ((new_device == 0 || new_device == current_device) ||
        (LoopbackManager::GetInstance()->GetLoopbackType() != NO_LOOPBACK))
    {
        goto EXIT_SETPARAMETERS;
    }

    SetStereoToMonoFlag(new_device);
    // for DAC path
    //   Turn Off : 6320 off (analog off -> digital off) -> 6589 off
    //   Turn On  : 6589 on -> 6320 on  (digital on -> analog on)
    if (mAudioResourceManager->IsModeIncall() == true)
    {
        SpeechPhoneCallController::GetInstance()->ChangeDeviceForModemSpeechControlFlow(mAudioResourceManager->GetAudioMode(), (audio_devices_t)new_device);

        // TODO(Harvey): Reduce copy & paste code here...
        if (android_audio_legacy::AudioSystem::isBluetoothScoDevice((android_audio_legacy::AudioSystem::audio_devices)new_device) == true) // BT
        {
            mDL1Attribute->mSampleRate = 8000;
            mDL1Attribute->mInterruptSample = 256;
        }
        else
        {
            mDL1Attribute->mSampleRate = mSourceSampleRate;
            calInterrupttime();
        }

        mDL1Attribute->mdevices = new_device;
    }
    else if (pAudioFMController->GetFmEnable() == true)
    {
        // Set FM Devices
        pAudioFMController->ChangeDevice(new_device);
        mDL1Attribute->mdevices = new_device;
    }
    else if (mStarting == true)
    {
        // only do with outputdevicechanged
        bool outputdevicechange = mAudioDigitalControl->CheckDlDigitalChange(current_device, new_device);
        if (true == outputdevicechange)
        {
            if (!(current_device & AUDIO_DEVICE_OUT_ALL_SCO) && (new_device & AUDIO_DEVICE_OUT_ALL_SCO)) // change to BTSCO device
            {
                mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK); // disable AUDIO_STREAMOUT_LOCK since standby() will use itemAt
                standby();
                mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
                mDL1Attribute->mSampleRate = 8000;
                mDL1Attribute->mInterruptSample = 256;
                ALOGD("setParameters mStarting=true change to BTSCO device, mDL1Attribute->mSampleRate=%d,mDL1Attribute->mInterruptSample=%d ", mDL1Attribute->mSampleRate, mDL1Attribute->mInterruptSample);
            }
            else if ((current_device & AUDIO_DEVICE_OUT_ALL_SCO) && !(new_device & AUDIO_DEVICE_OUT_ALL_SCO)) // change from BTSCO device
            {
                mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK); // disable AUDIO_STREAMOUT_LOCK since standby() will use it
                standby();
                mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK, AUDIO_LOCK_TIMEOUT_VALUE_MS);
                mDL1Attribute->mSampleRate = mSourceSampleRate; //mSourceSampleRate is from AudioMTKStreamOut constructor
                calInterrupttime();
                SetI2SOutDACAttribute();
                ALOGD("setParameters mStarting=true change from BTSCO device, mDL1Attribute->mSampleRate=%d,mDL1Attribute->mInterruptSample=%d ", mDL1Attribute->mSampleRate, mDL1Attribute->mInterruptSample);
            }
            else
            {
                int OuputPreDevice = 0 , OutPutNewDevice = 0;
                OuputPreDevice = mAudioDigitalControl->DlPolicyByDevice(current_device);
                OutPutNewDevice = mAudioDigitalControl->DlPolicyByDevice(new_device);
                TurnOffAfeDigital(OuputPreDevice, false);
                TurnOnAfeDigital(OutPutNewDevice);
            }
        }
        //select output new_device
        mAudioResourceManager->SelectOutputDevice(new_device);
        mDL1Attribute->mdevices = new_device;
    }
    else
    {
        if (!(current_device & AUDIO_DEVICE_OUT_ALL_SCO) && (new_device & AUDIO_DEVICE_OUT_ALL_SCO)) // change to BTSCO device
        {
            mDL1Attribute->mSampleRate = 8000;
            mDL1Attribute->mInterruptSample = 256;
            ALOGD("setParameters mStarting=false change to BTSCO device, mDL1Attribute->mSampleRate=%d,mDL1Attribute->mInterruptSample=%d ", mDL1Attribute->mSampleRate, mDL1Attribute->mInterruptSample);
        }
        else if ((current_device & AUDIO_DEVICE_OUT_ALL_SCO) && !(new_device & AUDIO_DEVICE_OUT_ALL_SCO)) // change from BTSCO device
        {
            mDL1Attribute->mSampleRate = mSourceSampleRate; //mSourceSampleRate is from AudioMTKStreamOut constructor
            SetI2SOutDACAttribute();
            calInterrupttime();
            ALOGD("setParameters mStarting=false change from BTSCO device, mDL1Attribute->mSampleRate=%d,mDL1Attribute->mInterruptSample=%d ", mDL1Attribute->mSampleRate, mDL1Attribute->mInterruptSample);
        }
        mAudioResourceManager->setDlOutputDevice(new_device);
        mDL1Attribute->mdevices = new_device;  //mDL1Attribute->mdevices to be used in AudioMTKStreamOut::write()
    }
EXIT_SETPARAMETERS:
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMOUT_LOCK);
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_MODE_LOCK);
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_HARDWARE_LOCK);
    return ;
}

status_t AudioMTKStreamOut::setParameters(const String8 &keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 keyRouting = String8(AudioParameter::keyRouting);
    String8 keyStereoOutput = String8("EnableStereoOutput");
    status_t status = NO_ERROR;
    int devices = 0;
    int value = 0;
    ALOGD("setParameters() %s", keyValuePairs.string());
    if (param.getInt(keyRouting, devices) == NO_ERROR)
    {
        param.remove(keyRouting);

        uint32_t olddevice = mDL1Attribute->mdevices;
        dokeyRouting(devices);
        uint32_t newdevice = mDL1Attribute->mdevices;
        if (mFilters) { mFilters->setDevice(newdevice); } //ALPS00721858
        //speaker/receiver(headphone) switch no input path change, but should use receiver params
        if (((newdevice == AUDIO_DEVICE_OUT_SPEAKER) && ((olddevice == AUDIO_DEVICE_OUT_EARPIECE) || (olddevice == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)))
            || (((newdevice == AUDIO_DEVICE_OUT_EARPIECE) || (newdevice == AUDIO_DEVICE_OUT_WIRED_HEADPHONE)) && (olddevice == AUDIO_DEVICE_OUT_SPEAKER)))
        {
            ALOGD("check if need to update VoIP parameters");
            //Mutex::Autolock _l(mSPEVoIPLock);
            mAudioSpeechEnhanceInfoInstance->NeedUpdateVoIPParams();
        }
    }
    if (param.getInt(String8("force_standby"), value) == NO_ERROR) // only set this param when entering call related mode!!
    {
        if (value == true)
        {
            SetSuspend(true);
            if (AudioFMController::GetInstance()->GetFmEnable() == true)
            {
                ALOGD("%s(), force_standby, FM is on, disable it.", __FUNCTION__);
                AudioFMController::GetInstance()->SetFmEnable(false);
            }
            else if (AudioMATVController::GetInstance()->GetMatvEnable() == true)
            {
                ALOGD("%s(), force_standby, mATV is on, disable it.", __FUNCTION__);
                AudioMATVController::GetInstance()->SetMatvEnable(false);
            }
            standby();
        }
        else
        {
            SetSuspend(false);
        }
    }

    if (param.getInt(keyStereoOutput, value) == NO_ERROR)
    {
       if(value)
       {
           mSteroToMono = false;
       }
       else
       {
           mSteroToMono = true;
       }
       param.remove(keyStereoOutput);
    }

    if (param.size())
    {
        status = BAD_VALUE;
    }
    return status;
}


String8 AudioMTKStreamOut::getParameters(const String8 &keys)
{
    ALOGD("AudioMTKHardware getParameters\n");
    AudioParameter param = AudioParameter(keys);
    return param.toString();
}

status_t AudioMTKStreamOut::getRenderPosition(uint32_t *dspFrames)
{
    return NO_ERROR;
}

status_t AudioMTKStreamOut::getNextWriteTimestamp(int64_t *timestamp)
{
    return INVALID_OPERATION;
}

status_t AudioMTKStreamOut::setCallBack(stream_callback_t callback, void *cookie)
{
    //TODO : new for KK
    return INVALID_OPERATION;
}


status_t AudioMTKStreamOut::getPresentationPosition(uint64_t *frames, struct timespec *timestamp)
{

    ALOGV("%s()",__FUNCTION__);

    *frames = mPresentedBytes / (uint64_t) (mDL1Attribute->mChannels * (mDL1Attribute->mFormat == AUDIO_FORMAT_PCM_8_BIT ? 1 : 2));
    *timestamp = mPresentedTime;

    return NO_ERROR;
}


status_t AudioMTKStreamOut::SetStreamRunning(bool bEnable)
{
    ALOGD("+AudioMTKStreamOut SetStreamRunning bEnable = %d", bEnable);
    mStarting = bEnable;
    mAudioSpeechEnhanceInfoInstance->SetOutputStreamRunning(mStarting);
    return NO_ERROR;
}

bool  AudioMTKStreamOut::GetStreamRunning()
{
    return mStarting;
}

status_t AudioMTKStreamOut::SetStreamOutCompFltStatus(AudioCompFltType_t eCompFltType, bool bEnable)
{

    //MTKStreamManager will call this funciton, just ignore.
    // mFilters->setParameter() will resart filter.
    return NO_ERROR;
}

bool AudioMTKStreamOut::GetStreamOutCompFltStatus(AudioCompFltType_t eCompFltType)
{
    //MTKStreamManager will call this funciton, just ignore.
    //mFilters->setParameter() will resart filter.
    return false;
}

status_t AudioMTKStreamOut::StreamOutCompFltPreviewParameter(AudioCompFltType_t eCompFltType, void *ptr , int len)
{
    ALOGD("%s(), filterType %d", __FUNCTION__, eCompFltType);
    if (mFilters)
    {
        mFilters->setParameter(eCompFltType, (AUDIO_ACF_CUSTOM_PARAM_STRUCT *)ptr);
    }
    return NO_ERROR;
}

status_t AudioMTKStreamOut::SetSuspend(bool suspend)
{
    if (suspend)
    {
        mSuspend++;
    }
    else
    {
        mSuspend--;
        if (mSuspend < 0)
        {
            ALOGW("mSuspend = %d", mSuspend);
            mSuspend = 0;
        }
    }
    ALOGD("SetSuspend mSuspend = %d suspend = %d", mSuspend, suspend);
    return NO_ERROR;
}

bool AudioMTKStreamOut::GetSuspend()
{
    return mSuspend;
}

status_t AudioMTKStreamOut::dump(int fd, const Vector<String16> &args)
{
    return NO_ERROR;
}

AudioMTKStreamOut::BliSrc::BliSrc()
    : mHandle(NULL), mBuffer(NULL), mInitCheck(NO_INIT)
{
}

AudioMTKStreamOut::BliSrc::~BliSrc()
{
    close();
}

status_t AudioMTKStreamOut::BliSrc::initStatus()
{
    return mInitCheck;
}

status_t  AudioMTKStreamOut::BliSrc::init(uint32 inSamplerate, uint32 inChannel, uint32 OutSamplerate, uint32 OutChannel)
{
    if (mHandle == NULL)
    {
        uint32_t workBufSize;
        BLI_GetMemSize(inSamplerate, inChannel, OutSamplerate, OutChannel, &workBufSize);
        ALOGD("BliSrc::init InputSampleRate=%u, inChannel=%u, OutputSampleRate=%u, OutChannel=%u, mWorkBufSize = %u",
              inSamplerate, inChannel, OutSamplerate, OutChannel, workBufSize);
        mBuffer = new uint8_t[workBufSize];
        if (!mBuffer)
        {
            ALOGE("BliSrc::init Fail to create work buffer");
            return NO_MEMORY;
        }
        memset((void *)mBuffer, 0, workBufSize);
        mHandle = BLI_Open(inSamplerate, inChannel, OutSamplerate, OutChannel, (char *)mBuffer, NULL);
        if (!mHandle)
        {
            ALOGE("BliSrc::init Fail to get blisrc handle");
            if (mBuffer)
            {
                delete []mBuffer;
                mBuffer = NULL;
            }
            return NO_INIT;
        }
        mInitCheck = OK;
    }
    return NO_ERROR;

}

size_t  AudioMTKStreamOut::BliSrc::process(const void *inbuffer, size_t inBytes, void *outbuffer)
{
    if (mHandle)
    {
        size_t inputLength = inBytes;
        size_t outputLength = inBytes;
        size_t consume = BLI_Convert(mHandle, (short *)inbuffer, &inputLength, (short *)outbuffer, &outputLength);
        ALOGD_IF(consume != inBytes, "inputLength=%d,consume=%d,outputLength=%d", inputLength, consume, outputLength);
        return outputLength;
    }
    ALOGW("BliSrc::process src not initialized");
    return 0;
}

status_t  AudioMTKStreamOut::BliSrc::close(void)
{
    if (mHandle)
    {
        BLI_Close(mHandle, NULL);
        mHandle = NULL;
    }
    if (mBuffer)
    {
        delete []mBuffer;
        mBuffer = NULL;
    }
    mInitCheck = NO_INIT;
    return NO_ERROR;
}

int AudioMTKStreamOut::GetSampleRate(void)
{
    return mDL1Attribute->mSampleRate;
}
int AudioMTKStreamOut::GetChannel(void)
{
    return mDL1Attribute->mChannels;
}

bool AudioMTKStreamOut::EffectMutexLock(void)
{
    return mEffectLock.lock();
}
bool AudioMTKStreamOut::EffectMutexUnlock(void)
{
    mEffectLock.unlock();
    return true;
}

void AudioMTKStreamOut::add_echo_reference(struct echo_reference_itfe *reference)
{
    Mutex::Autolock _l(mEffectLock);
    ALOGD("add_echo_reference %p", reference);
    mEcho_reference = reference;

}
void AudioMTKStreamOut::remove_echo_reference(struct echo_reference_itfe *reference)
{
    Mutex::Autolock _l(mEffectLock);
    ALOGD("remove_echo_reference %p", reference);
    if (mEcho_reference == reference)
    {
        /* stop writing to echo reference */
        reference->write(reference, NULL);
        mEcho_reference = NULL;
    }
    else
    {
        ALOGW("remove wrong echo reference %p", reference);
    }
    ALOGD("remove_echo_reference ---");
}

int AudioMTKStreamOut::get_playback_delay(size_t frames, struct echo_reference_buffer *buffer)
{
    struct timespec tstamp;
    size_t kernel_frames;

    //FIXME:: calculate for more precise time delay

    int rc = clock_gettime(CLOCK_MONOTONIC, &tstamp);
    if (rc != 0)
    {
        buffer->time_stamp.tv_sec  = 0;
        buffer->time_stamp.tv_nsec = 0;
        buffer->delay_ns           = 0;
        ALOGW("get_playback_delay(): pcm_get_htimestamp error,"
              "setting playbackTimestamp to 0");
        return 0;
    }

    /* adjust render time stamp with delay added by current driver buffer.
     * Add the duration of current frame as we want the render time of the last
     * sample being written. */
    buffer->delay_ns = (long)(((int64_t)(frames) * 1000000000) /
                              mDL1Attribute->mSampleRate);

    buffer->time_stamp = tstamp;

    //    ALOGD("get_playback_delay time_stamp = [%ld].[%ld], delay_ns: [%d]",buffer->time_stamp.tv_sec , buffer->time_stamp.tv_nsec, buffer->delay_ns);
    return 0;
}

size_t AudioMTKStreamOut::writeDataToEchoReference(const void *buffer, size_t bytes, int remain_ms, struct timespec TimeInfo)
{
    //push the output data to echo reference
    EffectMutexLock();
    if (mEcho_reference != NULL)
    {
        ALOGV("writeDataToEchoReference echo_reference %p", mEcho_reference);
        struct echo_reference_buffer b;
        b.raw = (void *)buffer;
        b.frame_count = bytes / sizeof(int16_t) / mDL1Attribute->mChannels;

        get_playback_delay(b.frame_count, &b);
        mEcho_reference->write(mEcho_reference, &b);
    }
    EffectMutexUnlock();

    Mutex::Autolock _l(mSPEVoIPLock);
    //do the resample and queue to the SPE buffer queue
    if (mAudioSpeechEnhanceInfoInstance->IsInputStreamAlive())
    {
        //do not queue to echo reference if FM/mATV enalbed
        if (AudioFMController::GetInstance()->GetFmEnable() == true ||
            AudioMATVController::GetInstance()->GetMatvEnable() == true)
        {
            return bytes;
        }

#ifndef MTK_VOIP_ENHANCEMENT_SUPPORT
        bool bSkipVoIP = true;
        if (bSkipVoIP)
        {
            return bytes;
        }
#endif
        size_t outputSize = 0;
        uint8_t *outbuffer;
        struct InBufferInfo BInfo;
        //remain_ms = NO_REMAIN_DATA_INFO;

        if (remain_ms == NO_REMAIN_DATA_INFO)
        {
            BInfo.bHasRemainInfo = false;
            BInfo.time_stamp_queued = TimeInfo;
        }
        else
        {
            BInfo.bHasRemainInfo = true;
            //BInfo.time_stamp_queued= GetSystemTime();
            BInfo.time_stamp_queued = TimeInfo;
            if (remain_ms == 0)
            {
                BInfo.time_stamp_predict = BInfo.time_stamp_queued;
            }
            else
            {
                if ((BInfo.time_stamp_queued.tv_nsec + remain_ms * 1000000) >= 1000000000)
                {
                    BInfo.time_stamp_predict.tv_nsec = BInfo.time_stamp_queued.tv_nsec + remain_ms * 1000000 - 1000000000;
                    BInfo.time_stamp_predict.tv_sec = BInfo.time_stamp_queued.tv_sec + 1;
                }
                else
                {
                    BInfo.time_stamp_predict.tv_sec = BInfo.time_stamp_queued.tv_sec;
                    BInfo.time_stamp_predict.tv_nsec = BInfo.time_stamp_queued.tv_nsec + remain_ms * 1000000;
                }
            }
            //ALOGD("time_stamp_predict sec= %ld, nsec=%ld,remain_ms=%d" , BInfo.time_stamp_predict.tv_sec, BInfo.time_stamp_predict.tv_nsec, remain_ms);
        }

        outputSize = DoVoIPSRC(buffer, bytes, (void **)&outbuffer);
        //write data to SPElayer

        BInfo.pBufBase = (short *)outbuffer;
        BInfo.BufLen = outputSize;

        mAudioSpeechEnhanceInfoInstance->WriteReferenceBuffer(&BInfo);
        return outputSize;
    }

    return bytes;
}

void AudioMTKStreamOut::StopWriteDataToEchoReference()
{
    /* stop writing to echo reference */
    Mutex::Autolock _l(mEffectLock);
    ALOGD("StopWriteDataToEchoReference %p", mEcho_reference);
    if (mEcho_reference != NULL)
    {
        mEcho_reference->write(mEcho_reference, NULL);
        mEcho_reference = NULL;
    }
    ALOGD("StopWriteDataToEchoReference ---");
}

void AudioMTKStreamOut::SetMusicPlusStatus(bool bEnable)
{
    if (mFilters)
    {
        mFilters->setParamFixed(bEnable);
    }
    return;
}

bool AudioMTKStreamOut::GetMusicPlusStatus(void)
{
    if (mFilters)
    {
        return mFilters->isParamFixed();
    }
    return false;
}

size_t AudioMTKStreamOut::WriteDataToAudioHW(const void *buffer, size_t bytes)
{
    const uint32_t current_device = mAudioResourceManager->getDlOutputDevice();
    size_t outputSize = 0;
    int ret_ms = 0;
    int RemainSize = 0;
    void *inbuffer = const_cast<void *>(buffer); // mSwapBufferOne  or mixerbufer of audioflinger
    void *outbuffer = mSwapBufferTwo;
    AudioVUnlockDL *VUnlockhdl = AudioVUnlockDL::getInstance();
    audio_mode_t Mode = mAudioResourceManager->GetAudioMode();
#ifndef DMNR_TUNNING_AT_MODEMSIDE
    //bypass CompFlt if tuning DMNR in AP side
    const bool bAPDMNRTuningEnable = mAudioSpeechEnhanceInfoInstance->IsAPDMNRTuningEnable();
#endif

#ifndef DMNR_TUNNING_AT_MODEMSIDE
    if ((Mode == AUDIO_MODE_IN_COMMUNICATION) || bAPDMNRTuningEnable)
#else
    if (Mode == AUDIO_MODE_IN_COMMUNICATION)
#endif
    {
        outbuffer = (void *)buffer;
        outputSize = bytes;
    }
    else
    {
        if (mFilters)
        {
            outputSize = bufferSize() > bytes ? bytes : bufferSize();
            mFilters->start();
            outputSize = mFilters->process(inbuffer, bytes, outbuffer,outputSize);
            //#if defined(MTK_VIBSPK_SUPPORT) //Add Vibration Signal to Out
            if (IsAudioSupportFeature(AUDIO_SUPPORT_VIBRATION_SPEAKER))
            {
                if (current_device & AUDIO_DEVICE_OUT_SPEAKER)
                {
                    //Temp Use mSwapBufferVoIP for memory reduce
                    if (outputSize == 0)
                    {
                        dumpPcm(mLoudNotchDumpFile, inbuffer, bytes);
                        outputSize = DoVibSignal2DLProcess(outbuffer, (void *)mSwapBufferVoIP, inbuffer, bytes);
                        if (outputSize == 0)
                        {
                            outbuffer  = inbuffer; //filter not handle inbuffer;
                            outputSize = bytes;
                        }
                    }
                    else
                    {
                        dumpPcm(mLoudNotchDumpFile, outbuffer, outputSize);
                        DoVibSignal2DLProcess(outbuffer, (void *)mSwapBufferVoIP, outbuffer, outputSize);
                    }
                }
            }
            //#endif
            if (outputSize == 0)
            {
                ALOGW("filters fail to process");
                outbuffer  = inbuffer; //filter not handle inbuffer;
                outputSize = bytes;
            }

        }
        else    //if no filter
        {
            outbuffer  = inbuffer;
            outputSize = bytes;
        }
    }
    dumpPcm(mPFinalPCMDumpFile, outbuffer, outputSize);

    RemainSize = ::ioctl(mFd, AUDDRV_GET_DL1_REMAINDATA_TIME, NULL);
    struct timespec TimeInfo = GetSystemTime(false);
    ret_ms =  CalRemaintime(RemainSize);
    //============Voice UI&Unlock REFERECE=============
    if (VUnlockhdl != NULL)
    {
        VUnlockhdl->SetDownlinkStartTime(ret_ms);
    }
    //===========================================   
    outputSize =::write(mFd, outbuffer, outputSize);
    UpdateLine(__LINE__);
    
    if (ret_ms == 0)    //there is no data in hardware buffer
    {
        ALOGD("WriteDataToAudioHW ret_ms=%d", ret_ms);
    }

    if (ret_ms < 0)
    {
        ret_ms = 0;
    }

    writeDataToEchoReference(outbuffer, outputSize, ret_ms, TimeInfo);
    if (VUnlockhdl != NULL)
    {
        //VUnlockhdl->SetInputStandBy(false);
        if ((current_device & AUDIO_DEVICE_OUT_WIRED_HEADSET) || (current_device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE))
        {
            memset(outbuffer, 0, outputSize);
        }
        VUnlockhdl->WriteStreamOutToRing(outbuffer, outputSize);
    }
    
    mPresentedBytes += outputSize;    
    clock_gettime(CLOCK_MONOTONIC, &mPresentedTime);
    
    return  outputSize;
}

int AudioMTKStreamOut::CalRemaintime(int buffersize)
{
    int retms = 0;
    if (buffersize >= 0)
    {
        retms = (buffersize * 1000) / (mDL1Attribute->mSampleRate * mDL1Attribute->mChannels *
                                       (mDL1Attribute->mFormat == AUDIO_FORMAT_PCM_8_BIT ? 1 :    //8  1byte/frame
                                        (mDL1Attribute->mFormat == AUDIO_FORMAT_PCM_32_BIT ? 4 :   //24bit 3bytes/frame
                                         2)));   //default 2bytes/sample
    }
    //ALOGD("CalRemaintime buffersize=%d, retms=%d", buffersize, retms);
    return retms;
}

bool AudioMTKStreamOut::NeedToDoBTSCOProcess(uint32 DigitalPart)
{
#if 0 //0902
#if defined(BTCVSD_ENC_DEC_LOOPBACK) || defined(BTCVSD_KERNEL_LOOPBACK)
    return true;
#elif (defined(BTCVSD_LOOPBACK_WITH_CODEC))
    if (mSourceSampleRate == 8000)
    {
        return true;
    }
#else
    if (DigitalPart == AudioDigitalType::DAI_BT)
    {
        return true;
    }
#endif
#else
    if (AudioLoopbackController::GetInstance()->IsAPBTLoopbackWithCodec() == true)
    {
        return true;
    }

    if (DigitalPart == AudioDigitalType::DAI_BT)
    {
        return true;
    }
#endif

    return false;
}

#ifdef EXTMD_LOOPBACK_TEST
void AudioMTKStreamOut::BTCVSD_ExtMDLoopBackTest_Init(void)
{
    SetStreamRunning(true);

    uint32 DigitalPart = mAudioDigitalControl->DlPolicyByDevice(mDL1Attribute->mdevices);
    if (DigitalPart == AudioDigitalType::DAI_BT)
    {
        ALOGD("EXTMD_LOOPBACK_TEST AudioExtMDCVSDCreateThread()");
        mAudioBTCVSDControl->AudioExtMDCVSDCreateThread();
    }
}

void AudioMTKStreamOut::BTCVSD_ExtMDLoopBackTest_Stadby(void)
{
    uint32 DigitalPart = mAudioDigitalControl->DlPolicyByDevice(mDL1Attribute->mdevices);
    if (DigitalPart == AudioDigitalType::DAI_BT)
    {
        ALOGD("EXTMD_LOOPBACK_TEST AudioExtMDCVSDDeleteThread()");
        mAudioBTCVSDControl->AudioExtMDCVSDDeleteThread();
    }

}
#endif
//#if defined(MTK_VIBSPK_SUPPORT)
const int32_t AUD_VIBR_FILTER_COEF_Table[VIBSPK_FILTER_NUM][2][6][3] =
{
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_141,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_144,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_147,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_150,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_153,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_156,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_159,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_162,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_165,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_168,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_171,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_174,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_177,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_180,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_183,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_186,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_189,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_192,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_195,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_198,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_201,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_204,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_207,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_210,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_213,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_216,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_219,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_222,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_225,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_228,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_231,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_234,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_237,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_240,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_243,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_246,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_249,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_252,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_255,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_258,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_261,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_264,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_267,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_270,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_273,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_276,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_279,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_282,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_285,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_288,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_291,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_294,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_297,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_300,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_303,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_306,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_309,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_312,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_315,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_318,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_321,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_324,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_327,
    DEFAULT_AUD_VIBR_LOUDNESS_FILTER_COEF_330
};

status_t AudioMTKStreamOut::SetVibSpkDefaultParam()
{
    AUDIO_ACF_CUSTOM_PARAM_STRUCT cali_param;
    memset(&cali_param, 0, sizeof(AUDIO_ACF_CUSTOM_PARAM_STRUCT));
    memcpy(&cali_param.bes_loudness_bpf_coeff, &AUD_VIBR_FILTER_COEF_Table[(VIBSPK_DEFAULT_FREQ - VIBSPK_FREQ_LOWBOUND + 1) / VIBSPK_FILTER_FREQSTEP], sizeof(uint32_t)*VIBSPK_AUD_PARAM_SIZE);
    cali_param.bes_loudness_WS_Gain_Min = VIBSPK_DEFAULT_FREQ;
    cali_param.bes_loudness_WS_Gain_Max = VIBSPK_SETDEFAULT_VALUE;
    SetAudioCompFltCustParamToNV(AUDIO_COMP_FLT_VIBSPK, &cali_param);
    ALOGD("[VibSpk] SetDefaultFreq");
    return NO_ERROR;
}


static short clamp16(int sample)
{
    if ((sample >> 15) ^ (sample >> 31))
    {
        sample = 0x7FFF ^ (sample >> 31);
    }
    return sample;
}

void AudioMTKStreamOut::UpdateLine(int line)
{
#ifdef AUDIOLOCK_DEBUG_ENABLE
    mAudioResourceManager->SetDebugLine(line);
#endif
}

size_t  AudioMTKStreamOut::DoVibSignal2DLProcess(void *outbuffer, void *vibtonebuffer, void *src2DLbuffer, size_t bytes)
{
    if (mVibSpk->getVibSpkEnable())
    {

        if (mVibSpkEnable == false)
        {
            mVibSpkEnable = true;
            mVibSpk->VibSpkRampControl(2);
        }
        mVibSpk->VibSpkProcess(bytes, vibtonebuffer, mDL1Attribute->mChannels);//Gen Tone
        dumpPcm(mVIBsignalDumpFile, vibtonebuffer, bytes);
    }
    else
    {
        if (mVibSpkEnable == true)
        {
            mVibSpkEnable = false;
            mVibSpk->VibSpkRampControl(1);
            mVibSpk->VibSpkProcess(bytes, vibtonebuffer, mDL1Attribute->mChannels);
            dumpPcm(mVIBsignalDumpFile, vibtonebuffer, bytes);
        }
        else
        {
            //Don't add vibration signal, however must apply gain to keep the volume
            int16_t dAudioGain = 0x7FFF - mVibSpk->getVibSpkGain();
            uint32 dSampleCount = bytes >> 1;
            short *pAudioData = (short *)src2DLbuffer;
            short *pOutputData = (short *)outbuffer;
            while (dSampleCount)
            {
                *pOutputData = (((*pAudioData) * dAudioGain) >> 15);
                pOutputData++;
                pAudioData++;
                dSampleCount--;
            }
            return bytes;
        }

    }
    int16_t dAudioGain = 0x7FFF - mVibSpk->getVibSpkGain();
    uint32 dSampleCount = bytes >> 1;
    short *pVibToneData = (short *)vibtonebuffer;
    short *pAudioData = (short *)src2DLbuffer;
    short *pOutputData = (short *)outbuffer;
    while (dSampleCount)
    {
        //*pOutputData = clamp16((int)(*pVibToneData)+(((int)(*pAudioData))*dAudioGain)>>15);
        *pOutputData = (*pVibToneData) + (((*pAudioData) * dAudioGain) >> 15);
        pOutputData++;
        pVibToneData++;
        pAudioData++;
        dSampleCount--;
    }

    return bytes;

}
//#endif

}
