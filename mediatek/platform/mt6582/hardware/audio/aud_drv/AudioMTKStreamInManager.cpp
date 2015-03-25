#include "AudioMTKStreamInManager.h"
#include "AudioResourceManager.h"
#include "AudioResourceFactory.h"
#include "AudioAnalogType.h"
#include <utils/String16.h>
#include "AudioUtility.h"

#include "SpeechDriverFactory.h"
#include "AudioBTCVSDControl.h"
#include "AudioMTKStreamIn.h"

#include <linux/rtpm_prio.h>

#ifdef MTK_AUDIO_HD_REC_SUPPORT
#include "AudioCustParam.h"
#endif

#include "AudioFMController.h"
#include "AudioMATVController.h"
#include "WCNChipController.h"


extern "C" {
#include "bli_exp.h"
}

#define LOG_TAG "AudioMTKStreamInManager"
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

#define DAIBT_SAMPLE_RATE (8000)

#define RECORD_DROP_MS_MAX (200)

#define MTK_STREAMIN_VOLUEM_MAX (0x1000)
#define MTK_STREAMIN_VOLUME_VALID_BIT (12)
#define MAX_DUMP_NUM (6)
#define MAX_FILE_LENGTH (60000000)

namespace android
{

#define ASSERT_FTRACE(exp) \
    do { \
        if (!(exp)) { \
            ALOGE("ASSERT_FTRACE("#exp") fail: \""  __FILE__ "\", %uL", __LINE__); \
            aee_system_exception("libaudio.primary.default.so", NULL, DB_OPT_FTRACE, "AudioMTkRecordThread is block?"); \
        } \
    } while(0)


AudioMTKStreamInManager *AudioMTKStreamInManager::UniqueStreamInManagerInstance = NULL;
int AudioMTKStreamInManager::AudioMTkRecordThread::DumpFileNum = 0;

AudioMTKStreamInManager *AudioMTKStreamInManager::getInstance()
{
    if (UniqueStreamInManagerInstance == NULL)
    {
        ALOGD("+AudioMTKStreamInManager");
        UniqueStreamInManagerInstance = new AudioMTKStreamInManager();
        ALOGD("-AudioMTKStreamInManager");
    }
    ALOGV("getInstance()");
    return UniqueStreamInManagerInstance;
}

void AudioMTKStreamInManager::freeInstance()
{
    return;
}

AudioMTKStreamInManager::AudioMTKStreamInManager()
{
    mClientNumber = 1  ;
    ALOGD("AudioMTKStreamInManager constructor");

    // allcoate buffer
    mAWBbuffer = new char[MemAWBBufferSize];
    mVULbuffer = new char[MemVULBufferSize];
#if 0
    mDAIbuffer = new char[MemDAIBufferSize];
#endif
    mMODDAIbuffer = new char[MemDAIBufferSize];
    // get digital control
    mAudioDigital = AudioDigitalControlFactory::CreateAudioDigitalControl();
    mAudioResourceManager = AudioResourceManagerFactory::CreateAudioResource();

    memset((void *)&mIncallRingBuffer, 0, sizeof(RingBuf));
    mIncallRingBuffer.pBufBase = new char[INCALL_RINGBUFFERE_SIZE];
    if (mIncallRingBuffer.pBufBase  == NULL)
    {
        ALOGW("mRingBuf.pBufBase allocate fail");
    }
    mIncallRingBuffer.bufLen = INCALL_RINGBUFFERE_SIZE;
    mIncallRingBuffer.pRead = mIncallRingBuffer.pBufBase ;
    mIncallRingBuffer.pWrite = mIncallRingBuffer.pBufBase ;
    mMicMute = false;
    mMuteTransition = false;
    mBackUpRecordDropTime = AUDIO_RECORD_DROP_MS;

    mBliHandlerDMIC = NULL;
    mBliOutputBufferDMIC = NULL;
    mBliHandlerDAIBT = NULL;
    mBliOutputBufferDAIBT = NULL;

    PreLoadHDRecParams();
}

AudioMTKStreamInManager::~AudioMTKStreamInManager()
{
    ALOGD("AudioMTKStreamInManager destructor");
}

status_t  AudioMTKStreamInManager::initCheck()
{
    return NO_ERROR;
}

void AudioMTKStreamInManager::PreLoadHDRecParams(void)
{
#ifdef MTK_AUDIO_HD_REC_SUPPORT
    ALOGD("PreLoadHDRecParams+++");
    //for NVRAM create file first to reserve the memory
    AUDIO_HD_RECORD_SCENE_TABLE_STRUCT DummyhdRecordSceneTable;
    AUDIO_HD_RECORD_PARAM_STRUCT DummyhdRecordParam;
    GetHdRecordSceneTableFromNV(&DummyhdRecordSceneTable);
    GetHdRecordParamFromNV(&DummyhdRecordParam);
    ALOGD("PreLoadHDRecParams---");
#endif
}

timespec AudioMTKStreamInManager::GetSystemTime(bool print)
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

unsigned long long AudioMTKStreamInManager::ProcessTimeCheck(struct timespec StartTime, struct timespec EndTime)
{
    unsigned long long diffns = 0;
    int thresholdms = 40;
    struct timespec tstemp1 = StartTime;
    struct timespec tstemp2 = EndTime;

    if (tstemp1.tv_sec > tstemp2.tv_sec)
    {
        if (tstemp1.tv_nsec >= tstemp2.tv_nsec)
        {
            diffns = ((tstemp1.tv_sec - tstemp2.tv_sec) * 1000000000) + tstemp1.tv_nsec - tstemp2.tv_nsec;
        }
        else
        {
            diffns = ((tstemp1.tv_sec - tstemp2.tv_sec - 1) * 1000000000) + tstemp1.tv_nsec + 1000000000 - tstemp2.tv_nsec;
        }
    }
    else if (tstemp1.tv_sec == tstemp2.tv_sec)
    {
        if (tstemp1.tv_nsec >= tstemp2.tv_nsec)
        {
            diffns = tstemp1.tv_nsec - tstemp2.tv_nsec;
        }
        else
        {
            diffns = tstemp2.tv_nsec - tstemp1.tv_nsec;
        }
    }
    else
    {
        if (tstemp2.tv_nsec >= tstemp1.tv_nsec)
        {
            diffns = ((tstemp2.tv_sec - tstemp1.tv_sec) * 1000000000) + tstemp2.tv_nsec - tstemp1.tv_nsec;
        }
        else
        {
            diffns = ((tstemp2.tv_sec - tstemp1.tv_sec - 1) * 1000000000) + tstemp2.tv_nsec + 1000000000 - tstemp1.tv_nsec;
        }
    }
    /*
        if((diffns/1000000)>=thresholdms)
        {
            ALOGW("ProcessTimeCheck, process too long? sec %ld nsec %ld, sec %ld nsec %ld", StartTime.tv_sec, StartTime.tv_nsec, EndTime.tv_sec, EndTime.tv_nsec);
        }*/
    return diffns;

}

uint32_t AudioMTKStreamInManager::BesRecordProcess(AudioMTKStreamInClient *Client, void *buffer , uint32 copy_size, AdditionalInfo_STRUCT AddInfo)
{
#ifdef MTK_AUDIO_HD_REC_SUPPORT
    //ALOGD("BesRecordProcess mEnableBesRecord=%d, copy_size=%d",Client->mEnableBesRecord,copy_size);
    if (Client->mEnableBesRecord == true)
        //if(Client->mEnableBesRecord==true && Client->mMemDataType!=AudioDigitalType::MEM_DAI) //temp solution for BTSCO
    {
        Client->mStreamIn->BesRecordPreprocess(buffer, copy_size, AddInfo);

    }
#endif
    return copy_size;
}

uint32_t AudioMTKStreamInManager::CopyBufferToClient(uint32 mMemDataType, void *buffer , uint32 copy_size, AdditionalInfo_STRUCT AddInfo)
{
    /*
    ALOGD("CopyBufferToClient mMemDataType = %d buffer = %p copy_size = %d mAudioInput.size() = %d buffer = 0x%x",
         mMemDataType, buffer, copy_size, mAudioInput.size(), *(unsigned int *)buffer);*/

    int ret = 0;
    ret = mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGER_LOCK, 300);
    if (ret)
    {
        ALOGW("EnableAudioLock AUDIO_STREAMINMANAGER_LOCK fail ret = %d", ret);
        return 0;
    }
    int FreeSpace = 0;
    char *tempBuf = new char[copy_size];
    //MTK_VOIP_ENHANCEMENT_SUPPORT debug purpose+++
    struct timespec EnterTime;
    struct timespec Finishtime;
    unsigned long long timecheck = 0;
    EnterTime = GetSystemTime();
    //---

    for (int i = 0 ; i < mAudioInput.size() ; i ++)
    {
        AudioMTKStreamInClient *temp = mAudioInput.valueAt(i) ;
        if (temp->mMemDataType == mMemDataType && true == temp->mEnable)
        {
            memcpy(tempBuf, buffer, copy_size * sizeof(char));
            BesRecordProcess(temp, tempBuf, copy_size, AddInfo);
            temp->mLock.lock();
            FreeSpace =  RingBuf_getFreeSpace(&temp->mRingBuf);
            if (FreeSpace >= copy_size)
            {
                //ALOGD("1 RingBuf_copyToLinear FreeSpace = %d temp = %p copy_size = %d mRingBuf = %p", FreeSpace, temp, copy_size, &temp->mRingBuf);
                RingBuf_copyFromLinear(&temp->mRingBuf, (char *)tempBuf, copy_size);
            }
            else
            {
                // do not copy , let buffer keep going
            }
            temp->mWaitWorkCV.signal();
            temp->mLock.unlock();
        }
    }

    // free temp data buffer
    delete[] tempBuf;

    //MTK_VOIP_ENHANCEMENT_SUPPORT debug purpose+++
    Finishtime = GetSystemTime();
    timecheck = ProcessTimeCheck(EnterTime, Finishtime);
    if (timecheck > mMaxProcessTime)
    {
        mMaxProcessTime = timecheck;
        ALOGD("CopyBufferToClient, mMaxProcessTime = %lld", mMaxProcessTime);
    }
    if ((timecheck / 1000000) >= 50)
    {
        ALOGW("CopyBufferToClient, process too long? sec %ld nsec %ld, sec %ld nsec %ld", EnterTime.tv_sec, EnterTime.tv_nsec, Finishtime.tv_sec, Finishtime.tv_nsec);
    }
    //---
    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGER_LOCK);
    return 0;
}

uint32_t AudioMTKStreamInManager::CopyBufferToClientIncall(RingBuf ul_ring_buf)
{
    mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGER_LOCK, 1000);

    // get M2A share buffer record data
    const uint32_t kNumModemData = RingBuf_getDataCount(&ul_ring_buf);
    char *p_modem_data = new char[kNumModemData];
    RingBuf_copyToLinear(p_modem_data, &ul_ring_buf, kNumModemData);

    for (size_t i = 0; i < mAudioInput.size(); ++i) // copy data to all clients
    {
        AudioMTKStreamInClient *pClient = mAudioInput.valueAt(i) ;
        if (pClient->mEnable)
        {
            pClient->mLock.lock();

            uint32_t num_free_space = RingBuf_getFreeSpace(&pClient->mRingBuf);

            if (pClient->mBliHandlerBuffer == NULL) // No need SRC
            {
                //ASSERT(num_free_space >= kNumModemData);
                if (num_free_space < kNumModemData)
                {
                    ALOGW("%s(), num_free_space(%u) < kNumModemData(%u)", __FUNCTION__, num_free_space, kNumModemData);
                }
                else
                {
                    RingBuf_copyFromLinear(&pClient->mRingBuf, p_modem_data, kNumModemData);
                }
            }
            else // Need SRC
            {
                char *p_read = p_modem_data;
                uint32_t num_modem_left_data = kNumModemData;
                uint32_t num_converted_data = num_free_space; // max convert num_free_space

                p_read += BLI_Convert(pClient->mBliHandlerBuffer,
                                      (int16_t *)p_read, &num_modem_left_data,
                                      (int16_t *)pClient->mBliOutputLinearBuffer, &num_converted_data);
                SLOGV("%s(), num_modem_left_data = %u, num_converted_data = %u", __FUNCTION__, num_modem_left_data, num_converted_data);

                if (num_modem_left_data > 0) { ALOGW("%s(), num_modem_left_data(%u) > 0", __FUNCTION__, num_modem_left_data); }
                //ASSERT(num_modem_left_data == 0);

                RingBuf_copyFromLinear(&pClient->mRingBuf, pClient->mBliOutputLinearBuffer, num_converted_data);
                SLOGV("%s(), pRead:%u, pWrite:%u, dataCount:%u", __FUNCTION__,
                      pClient->mRingBuf.pRead - pClient->mRingBuf.pBufBase,
                      pClient->mRingBuf.pWrite - pClient->mRingBuf.pBufBase,
                      RingBuf_getDataCount(&pClient->mRingBuf));
            }

            pClient->mWaitWorkCV.signal();

            pClient->mLock.unlock();
        }
    }

    // free linear UL data buffer
    delete[] p_modem_data;

    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGER_LOCK);
    return 0;
}

status_t AudioMTKStreamInManager::StartModemRecord(AudioMTKStreamInClient *Client)
{
    // start modem record only for the first client
    if (mAudioInput.size() != 1)
    {
        ALOGW("%s(), mAudioInput.size() = %u != 1", __FUNCTION__, mAudioInput.size());
        return ALREADY_EXISTS;
    }
    else
    {
        return SpeechDriverFactory::GetInstance()->GetSpeechDriver()->RecordOn();
    }
}

status_t AudioMTKStreamInManager::StopModemRecord()
{
    // stop modem record only for the last client
    if (mAudioInput.size() != 1)
    {
        ALOGW("%s(), mAudioInput.size() = %u != 1", __FUNCTION__, mAudioInput.size());
        return ALREADY_EXISTS;
    }
    else
    {
        return SpeechDriverFactory::GetInstance()->GetSpeechDriver()->RecordOff();
    }
}

bool AudioMTKStreamInManager::checkMemInUse(AudioMTKStreamInClient *Client)
{
    ALOGD("checkMemInUse Client = %p", Client);
    for (int i = 0; i < mAudioInput.size(); i++)
    {
        AudioMTKStreamInClient *mTemp = mAudioInput.valueAt(i);
        // if mem is the same  and other client is enable , measn this mem is alreay on
        if (mTemp->mMemDataType == Client->mMemDataType && mTemp->mEnable)
        {
            ALOGD("vector has same memif in use Client->mem = %d", Client->mMemDataType);
            return true;
        }
    }
    return false;
}

bool AudioMTKStreamInManager::checkSourceInUse(AudioMTKStreamInClient *Client)
{
    ALOGD("checkSourceInUse Client = %p", Client);
    for (int i = 0; i < mAudioInput.size(); i++)
    {
        AudioMTKStreamInClient *mTemp = mAudioInput.valueAt(i);
        // if mem is the same  and other client is enable , measn this mem is alreay on
        if (mTemp->mSourceType == Client->mSourceType && mTemp->mEnable)
        {
            ALOGD("vector has same source memif in use Client->mSourceType = %d", Client->mSourceType);
            return true;
        }
    }
    return false;
}

// this function should start a thread to read kernel sapce buffer. , base on memory type
status_t AudioMTKStreamInManager::StartStreamInThread(uint32 mMemDataType)
{

    ALOGD("StartStreamInThread mMemDataType = %d", mMemDataType);
    switch (mMemDataType)
    {
        case AudioDigitalType::MEM_VUL:
            mVULThread = new AudioMTkRecordThread(this, mMemDataType , mVULbuffer, MemVULBufferSize);
            if (mVULThread.get())
            {
                mVULThread->run();
            }
            break;
        case AudioDigitalType::MEM_DAI:
            mDAIThread = new AudioMTkRecordThread(this, mMemDataType , NULL, 0);
            if (mDAIThread.get())
            {
                mDAIThread->run();
            }
            break;
        case AudioDigitalType::MEM_AWB:
            mAWBThread = new AudioMTkRecordThread(this, mMemDataType , mAWBbuffer, MemAWBBufferSize);

            if (mAWBThread.get())
            {
                mAWBThread->run();
            }
            break;
        case AudioDigitalType::MEM_MOD_DAI:
            mMODDAIThread = new AudioMTkRecordThread(this, mMemDataType , mMODDAIbuffer, MemDAIBufferSize);
            if (mMODDAIThread.get())
            {
                mMODDAIThread->run();
            }
            break;
        default:
            break;
    }
    return NO_ERROR;
}

status_t AudioMTKStreamInManager::AcquireMemBufferLock(AudioDigitalType::Digital_Block MemBlock, bool bEnable)
{
    switch (MemBlock)
    {
        case AudioDigitalType::MEM_VUL:
        {
            if (bEnable)
            {
                mAULBufferLock.lock();
            }
            else
            {

                mAULBufferLock.unlock();
            }
            break;
        }
#if 0
        case AudioDigitalType::MEM_DAI:
        {
            if (bEnable)
            {
                mDAIBufferLock.lock();
            }
            else
            {
                mDAIBufferLock.unlock();
            }
            break;
        }
#endif
        case AudioDigitalType::MEM_AWB:
        {
            if (bEnable)
            {
                mAWBBufferLock.lock();
            }
            else
            {
                mAWBBufferLock.unlock();
            }
            break;
        }
        case AudioDigitalType::MEM_MOD_DAI:
        {
            if (bEnable)
            {
                mMODDAIBufferLock.lock();
            }
            else
            {
                mMODDAIBufferLock.unlock();
            }
            break;
        }
        default:
            ALOGE("AcquireMemBufferLock with wrong bufer lock");
            return INVALID_OPERATION;
    }
    return NO_ERROR;
}

status_t  AudioMTKStreamInManager::Do_input_standby(AudioMTKStreamInClient *Client)
{
    ALOGD("+Do_input_standby Client = %p", Client);
    uint32 AudioIn1 = 0, AudioIn2 = 0 , AudioOut1 = 0, AudioOut2 = 0;
    int ret = 0;
    Client->mEnable = false;

    switch (mAudioResourceManager->GetAudioMode())
    {
        case AUDIO_MODE_NORMAL:
        case AUDIO_MODE_RINGTONE:
        case AUDIO_MODE_IN_COMMUNICATION:   // fix me, is that mode in communication needs to be care more??
        {
            ALOGD("%s(), Client->mSourceType = %d, Client->mMemDataType = %d\n", __FUNCTION__, Client->mSourceType, Client->mMemDataType);
            switch (Client->mMemDataType)
            {
                case AudioDigitalType::MEM_VUL:
                    AudioOut1 = AudioDigitalType::O09;
                    AudioOut2 = AudioDigitalType::O10;
                    if (mVULThread.get() && !checkMemInUse(Client))
                    {
                        // disable memif before thread exit bacause in thread destructor will release memif fp and cause the kernel change base address of memif
                        mAudioDigital->SetMemIfEnable(Client->mMemDataType, false);
                        ret = mVULThread->requestExitAndWait();
                        if (ret == WOULD_BLOCK)
                        {
                            mVULThread->requestExit();
                        }
                        mVULThread.clear();
                    }
                    break;

                case AudioDigitalType::MEM_DAI:
                    if (WCNChipController::GetInstance()->BTUseCVSDRemoval() != true)
                    {
                        AudioOut1 = AudioDigitalType::O11;
                        AudioOut2 = AudioDigitalType::O11;
                        AudioIn1 = AudioDigitalType::I02;
                        AudioIn2 = AudioDigitalType::I02;
                    }
                    if (mDAIThread.get() && !checkMemInUse(Client))
                    {
                        // disable memif before thread exit bacause in thread destructor will release memif fp and cause the kernel change base address of memif
                        mAudioDigital->SetMemIfEnable(Client->mMemDataType, false);
                        ret = mDAIThread->requestExitAndWait();
                        if (ret == WOULD_BLOCK)
                        {
                            mDAIThread->requestExit();
                        }
                        mDAIThread.clear();
                    }
                    break;

                case AudioDigitalType::MEM_AWB:
                    AudioOut1 = AudioDigitalType::O05;
                    AudioOut2 = AudioDigitalType::O06;
                    ALOGD("+Do_input_standby mAWBThread->requestExitAndWait()");
                    if (mAWBThread.get() && !checkMemInUse(Client))
                    {
                        // disable memif before thread exit bacause in thread destructor will release memif fp and cause the kernel change base address of memif
                        mAudioDigital->SetMemIfEnable(Client->mMemDataType, false);
                        ret = mAWBThread->requestExitAndWait();
                        if (ret == WOULD_BLOCK)
                        {
                            mAWBThread->requestExit();
                        }
                        mAWBThread.clear();
                    }
                    break;
                default:
                    ALOGD("NO support for memory interface");
                    break;
            }

            // ih no user is used , disable irq2
            if (mAudioInput.size() == 1)
            {
                mAudioDigital->SetIrqMcuEnable(AudioDigitalType::IRQ2_MCU_MODE, false);
            }

            // Disable input source module
            if (checkSourceInUse(Client) == false)
            {
                switch (Client->mSourceType)
                {
                    case AudioDigitalType::I2S_IN_ADC:
                        AudioIn1 = AudioDigitalType::I03;
                        AudioIn2 = AudioDigitalType::I04;

                        mAudioResourceManager->StopInputDevice();
                        mAudioDigital->SetI2SAdcEnable(false);
                        break;
                    case AudioDigitalType::I2S_IN_2:
                        AudioIn1 = AudioDigitalType::I00;
                        AudioIn2 = AudioDigitalType::I01;

                        if (AudioFMController::GetInstance()->GetFmEnable() == false &&
                            AudioMATVController::GetInstance()->GetMatvEnable() == false) // Note: FM & mATV will disable 2nd I2S when FM/mATV disabled
                        {
                            mAudioDigital->SetI2SASRCEnable(false);
                            mAudioDigital->SetI2SASRCConfig(false, 0); // Setting to bypass ASRC
                            mAudioDigital->Set2ndI2SInEnable(false);
                        }
                        break;
                    default:
                        break;
                }
            }

            // Disable interconn
            if (checkSourceInUse(Client) == false)
            {
                mAudioDigital->SetinputConnection(AudioDigitalType::DisConnect, AudioIn1, AudioOut1);
                mAudioDigital->SetinputConnection(AudioDigitalType::DisConnect, AudioIn2, AudioOut2);
            }

            // AFE_ON = false
            mAudioDigital->SetAfeEnable(false);
            break;
        }
        case AUDIO_MODE_IN_CALL:
        case AUDIO_MODE_IN_CALL_2:
        case AUDIO_MODE_IN_CALL_EXTERNAL:
        {
            StopModemRecord();
            break;
        }
        default:
            break;
    }

    // ih no user is used , close DMIC handler (prevent there is still has input stream from DMIC)
    if (mAudioInput.size() == 1)
    {
        if (mBliHandlerDMIC)
        {
            BLI_Close(mBliHandlerDMIC, NULL);
            delete[] mBliHandlerDMIC;
            mBliHandlerDMIC = NULL;
            ALOGD("-Do_input_standby delete mBliHandlerDMIC");
        }
        if (mBliOutputBufferDMIC)
        {
            delete[] mBliOutputBufferDMIC;
            mBliOutputBufferDMIC = NULL;
            ALOGD("-Do_input_standby delete mBliOutputBufferDMIC");
        }

        if (mBliHandlerDAIBT)
        {
            BLI_Close(mBliHandlerDAIBT, NULL);
            delete[] mBliHandlerDAIBT;
            mBliHandlerDAIBT = NULL;
            ALOGD("-Do_input_standby delete mBliHandlerDAIBT");
        }
        if (mBliOutputBufferDAIBT)
        {
            delete[] mBliOutputBufferDAIBT;
            mBliOutputBufferDAIBT = NULL;
            ALOGD("-Do_input_standby delete mBliOutputBufferDAIBT");
        }
    }
    else
    {
        ALOGD("-Do_input_standby mAudioInput.size() = %d, not 1!!!", mAudioInput.size());
    }

    ALOGD("-Do_input_standby Client = %p", Client);
    return NO_ERROR;
}

status_t AudioMTKStreamInManager::I2SAdcInSet(AudioDigtalI2S *AdcI2SIn, AudioStreamAttribute *AttributeClient)
{
    AdcI2SIn->mLR_SWAP = AudioDigtalI2S::NO_SWAP;
    AdcI2SIn->mBuffer_Update_word = 8;
    AdcI2SIn->mFpga_bit_test = 0;
    AdcI2SIn->mFpga_bit = 0;
    AdcI2SIn->mloopback = 0;
    AdcI2SIn->mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
    AdcI2SIn->mI2S_FMT = AudioDigtalI2S::I2S;
    AdcI2SIn->mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;

    if (AttributeClient->mIsDigitalMIC) //for digital MIC 32K constrain
    {
        AdcI2SIn->mI2S_SAMPLERATE = getMemVULSamplerate();
    }
    else
    {
        AdcI2SIn->mI2S_SAMPLERATE = (AttributeClient->mSampleRate);
    }

    AdcI2SIn->mI2S_EN = false;
    return NO_ERROR;
}

status_t  AudioMTKStreamInManager:: Do_input_start(AudioMTKStreamInClient *Client)
{
    // savfe interconnection
    ALOGD("+%s(), client = %p\n", __FUNCTION__, Client);
    uint32 AudioIn1 = 0, AudioIn2 = 0 , AudioOut1 = 0, AudioOut2 = 0;
    uint32 MemIfSamplerate = 0, MemIfChannel = 0;
    uint32_t srcBufLen = 0;
    char *srcBuf;
    int ret = 0;

    switch (mAudioResourceManager->GetAudioMode())
    {
        case AUDIO_MODE_NORMAL:
        case AUDIO_MODE_RINGTONE:
        case AUDIO_MODE_IN_COMMUNICATION:
        {
            // fix me, is that mode in communication needs to be care more??
            ALOGD("%s(), Client->mSourceType = %d, Client->mMemDataType = %d\n", __FUNCTION__, Client->mSourceType, Client->mMemDataType);
            switch (Client->mSourceType)
            {
                    // fix me:: pcm recording curretn get data from modem.
                case AudioDigitalType::MODEM_PCM_1_O:
                case AudioDigitalType::MODEM_PCM_2_O:
                    break;
                case AudioDigitalType::I2S_IN_ADC:
                    I2SAdcInSet(&mAdcI2SIn, Client->mAttributeClient);
                    mAudioDigital->SetI2SAdcIn(&mAdcI2SIn);
                    if (Client->mAttributeClient->mIsDigitalMIC) //for digital MIC 32K constrain
                    {
                        if (mBliHandlerDMIC == NULL)
                        {
                            mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_IN_ADC, getMemVULSamplerate());
                            uint32_t srcBufLen = 0;
                            // Need SRC 32k->48k
                            BLI_GetMemSize(getMemVULSamplerate(), 2, 48000, 2, &srcBufLen);
                            mBliHandlerDMIC =
                                BLI_Open(getMemVULSamplerate(), 2, 48000, 2, new char[srcBufLen], NULL);
                            mBliOutputBufferDMIC = new char[BliOutBufferSizeDMIC]; // tmp buffer for blisrc out
                            ASSERT(mBliOutputBufferDMIC != NULL);
                            ALOGD("Do_input_start create mBliHandlerDMIC");
                        }
                        else
                        {
                            ALOGD("Do_input_start mBliHandlerDMIC already exist!!");
                        }
                    }
                    else
                    {
                        mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_IN_ADC, Client->mAttributeClient->mSampleRate);
                    }
                    mAudioDigital->SetI2SAdcEnable(true);

                    // here open analog control
                    mAudioResourceManager->StartInputDevice();

                    AudioIn1 = AudioDigitalType::I03;
                    AudioIn2 = AudioDigitalType::I04;
                    break;
                case AudioDigitalType::I2S_IN_2:
                    AudioIn1 = AudioDigitalType::I00;
                    AudioIn2 = AudioDigitalType::I01;

                    // Config & Enable 2nd I2S In
                    if (AudioFMController::GetInstance()->GetFmEnable() == false &&
                        AudioMATVController::GetInstance()->GetMatvEnable() == false) // Note: FM & mATV will enable 2nd I2S when FM/mATV enabled
                    {
                        AudioDigtalI2S m2ndI2SInAttribute;
                        memset((void *)&m2ndI2SInAttribute, 0, sizeof(m2ndI2SInAttribute));

                        m2ndI2SInAttribute.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
                        m2ndI2SInAttribute.mI2S_IN_PAD_SEL = AudioDigtalI2S::I2S_IN_FROM_IO_MUX;
                        m2ndI2SInAttribute.mI2S_SLAVE = AudioDigtalI2S::MASTER_MODE; // TODO: DEFAULT MASTER MODE ???
                        m2ndI2SInAttribute.mI2S_SAMPLERATE = Client->mAttributeClient->mSampleRate;
                        m2ndI2SInAttribute.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
                        m2ndI2SInAttribute.mI2S_FMT = AudioDigtalI2S::I2S;
                        m2ndI2SInAttribute.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
                        mAudioDigital->Set2ndI2SIn(&m2ndI2SInAttribute);

                        // TODO: Enable ASRC ??? // Work on slave mode only!!
                        //mAudioDigital->SetI2SASRCConfig(true, Client->mAttributeClient->mSampleRate);
                        //mAudioDigital->SetI2SASRCEnable(true);

                        // Enable 2nd I2S In
                        mAudioDigital->Set2ndI2SInEnable(true);
                    }
                    break;
#if 0
                case AudioDigitalType::DAI_BT:
                    AudioIn1 = AudioDigitalType::I02;
                    AudioIn2 = AudioDigitalType::I02;
                    // here for SW MIC gain calculate
                    mAudioResourceManager->StartInputDevice();
                    break;
#endif
                default:
                    break;
            }

            switch (Client->mMemDataType)
            {
                case AudioDigitalType::MEM_VUL:
                    AudioOut1 = AudioDigitalType::O09;
                    AudioOut2 = AudioDigitalType::O10;
                    MemIfSamplerate = getMemVULSamplerate();
                    MemIfChannel = 2;
                    break;

                case AudioDigitalType::MEM_DAI:
                    // SRC 8k->48k
                    ALOGD("Do_input_start MEM_DAI fs=%d", Client->mAttributeClient->mSampleRate);
                    if (mBliHandlerDAIBT == NULL)
                    {
#if 1
                        mAudioBTCVSDControl = AudioBTCVSDControl::getInstance();
                        if (!mAudioBTCVSDControl)
                        {
                            ALOGE("Do_input_start AudioBTCVSDControl::getInstance() fail");
                        }
                        ASSERT(mAudioBTCVSDControl != NULL);

                        uint32_t sample_rate;
                        if (mAudioBTCVSDControl->BT_SCO_isWideBand() == 1)
                        {
                            ALOGD("Do_input_start mAudioBTCVSDControl->BTmode=1");
                            sample_rate = 16000;
                            BliOutBufferSizeDAIBT = MSBC_PCM_FRAME_BYTE * 6 * 2; // 16k mono->48k stereo
                        }
                        else
                        {
                            ALOGD("Do_input_start mAudioBTCVSDControl->BTmode=0");
                            sample_rate = DAIBT_SAMPLE_RATE;
                            BliOutBufferSizeDAIBT = SCO_RX_PCM8K_BUF_SIZE * 12 * 2; //8k mono->48k stereo
                        }
#else
                        sample_rate = DAIBT_SAMPLE_RATE;
                        BliOutBufferSizeDAIBT = SCO_RX_PCM8K_BUF_SIZE * 12 * 2; //8k mono->48k stereo
#endif

                        BLI_GetMemSize(sample_rate, 1, Client->mAttributeClient->mSampleRate, 2, &srcBufLen);
                        // mBliHandlerDAIBT will has the same ptr as working buf ptr, so only need to free mBliHandlerDAIBT
                        mBliHandlerDAIBT = BLI_Open(sample_rate, 1, Client->mAttributeClient->mSampleRate, 2, new char[srcBufLen], NULL);
                        ALOGD("Do_input_start MEM_DAI mBliHandlerDAIBT=0x%x", mBliHandlerDAIBT);
                        mBliOutputBufferDAIBT = new char[BliOutBufferSizeDAIBT]; // buffer for blisrc out
                        ASSERT(mBliOutputBufferDAIBT != NULL);
                        ALOGD("Do_input_start MEM_DAI create mBliHandlerDAIBT");
                    }
                    else
                    {
                        ALOGD("Do_input_start MEM_DAI mBliHandlerDAIBT existed!!!");
                    }
                    if (WCNChipController::GetInstance()->BTUseCVSDRemoval() != true)
                    {
                        AudioOut1 = AudioDigitalType::O11;
                        AudioOut2 = AudioDigitalType::O11;
                        AudioIn1 = AudioDigitalType::I02;
                        AudioIn2 = AudioDigitalType::I02;
                        MemIfSamplerate = MemDAISamplerate;
                        MemIfChannel = 1;
                        ALOGD("!!!Do_input_start MEM_DAI MemIfChannel=1");
                    }
                    break;

                case AudioDigitalType::MEM_AWB:
                    AudioOut1 = AudioDigitalType::O05;
                    AudioOut2 = AudioDigitalType::O06;
                    MemIfSamplerate = Client->mAttributeClient->mSampleRate;
                    MemIfChannel = 2;
                    break;
                default:
                    ALOGD("NO support for memory interface");
                    break;
            }

            bool bAfeEnabled = mAudioDigital->GetAfeEnable(true);
            bool bStartTimeSet = false;
            // set digital memif attribute
            if (!checkMemInUse(Client))
            {
                bool bActivateAudMem = false;
                ALOGD("checkMemInUse Start memtype = %d", Client->mMemDataType);
                if ((Client->mMemDataType != AudioDigitalType::MEM_DAI) ||
                    (WCNChipController::GetInstance()->BTUseCVSDRemoval() != true))
                {
                    bActivateAudMem = true;
                }
                if (bActivateAudMem == true)
                {
                    mAudioDigital->SetMemIfSampleRate(Client->mMemDataType, MemIfSamplerate);
                    mAudioDigital->SetMemIfChannelCount(Client->mMemDataType, MemIfChannel);

                }
                StartStreamInThread(Client->mMemDataType);
                if (bActivateAudMem == true)
                {
                    mAudioDigital->SetMemIfEnable(Client->mMemDataType, true);
                    if (bAfeEnabled)
                    {
                        ALOGD("AFE is already opened");
                        Client->mInputStartTime = GetSystemTime(true);
                        bStartTimeSet = true;
                    }
                }
            }

            // set irq enable , need handle with irq2 mcu mode.
            AudioIrqMcuMode IrqStatus;
            mAudioDigital->GetIrqStatus(AudioDigitalType::IRQ2_MCU_MODE, &IrqStatus);
            if (IrqStatus.mStatus == false)
            {
                ALOGD("SetIrqMcuSampleRate mSampleRate = %d", Client->mAttributeClient->mSampleRate);
                if (WCNChipController::GetInstance()->BTUseCVSDRemoval() != true)
                {
                    if (Client->mMemDataType == AudioDigitalType::MEM_DAI)
                    {
                        ALOGD("Do_input_start SetIrqMcuSampleRate = 8000, SetIrqMcuCounter=256 ");
                        mAudioDigital->SetIrqMcuSampleRate(AudioDigitalType::IRQ2_MCU_MODE, 8000);
                        mAudioDigital->SetIrqMcuCounter(AudioDigitalType::IRQ2_MCU_MODE, 256);
                    }
                    else
                    {
                        mAudioDigital->SetIrqMcuSampleRate(AudioDigitalType::IRQ2_MCU_MODE, 16000);
                        mAudioDigital->SetIrqMcuCounter(AudioDigitalType::IRQ2_MCU_MODE, 800); // 50ms
                        mAudioDigital->SetIrqMcuEnable(AudioDigitalType::IRQ2_MCU_MODE, true);
                    }
                }
                else
                {
                    if (Client->mMemDataType != AudioDigitalType::MEM_DAI)
                    {
                        mAudioDigital->SetIrqMcuSampleRate(AudioDigitalType::IRQ2_MCU_MODE, 16000);
                        mAudioDigital->SetIrqMcuCounter(AudioDigitalType::IRQ2_MCU_MODE, 800); // 50ms
                        mAudioDigital->SetIrqMcuEnable(AudioDigitalType::IRQ2_MCU_MODE, true);
                    }
                }
            }
            else
            {
                ALOGD("IRQ2_MCU_MODE is enabled , use original irq2 interrupt mode");
            }

            // set interconnection
            if (!checkMemInUse(Client))
            {
                if ((Client->mMemDataType != AudioDigitalType::MEM_DAI) ||
                    (WCNChipController::GetInstance()->BTUseCVSDRemoval() != true))

                {
                    mAudioDigital->SetinputConnection(AudioDigitalType::Connection, AudioIn1, AudioOut1);
                    mAudioDigital->SetinputConnection(AudioDigitalType::Connection, AudioIn2, AudioOut2);
                    mAudioDigital->SetAfeEnable(true);
                }
            }
            if (!bStartTimeSet)
            {
                Client->mInputStartTime = GetSystemTime(true);
            }
            break;
        }
        case AUDIO_MODE_IN_CALL:
        case AUDIO_MODE_IN_CALL_2:
        case AUDIO_MODE_IN_CALL_EXTERNAL:
        {
            StartModemRecord(Client);
            break;
        }
        default:
            break;
    }

    Client->mEnable = true;

    ALOGD("-Do_input_start client = %p", Client);
    return NO_ERROR;
}

AudioMTKStreamInClient *AudioMTKStreamInManager::RequestClient(uint32_t Buflen)
{
    AudioMTKStreamInClient *Client = NULL;
    Client = new AudioMTKStreamInClient(Buflen, mClientNumber);
    ALOGD("RequestClient Buflen = %d Client = %p AudioInput.size  = %d ", Buflen, Client, mAudioInput.size());
    if (Client == NULL)
    {
        ALOGW("RequestClient but return NULL");
        return NULL;
    }
    //until start should add output
    mClientNumber++;
    mAudioInput.add(mClientNumber, Client);
    Client->mClientId = mClientNumber;
    Client->mEnable = false;
    ALOGD("%s(), mAudioInput.size() = %d, mClientNumber = %d", __FUNCTION__, mAudioInput.size(), mClientNumber);
    return Client;
}

status_t AudioMTKStreamInManager::ReleaseClient(AudioMTKStreamInClient *Client)
{
    // remove from vector
    uint32_t clientid = Client->mClientId;
    ssize_t index = mAudioInput.indexOfKey(clientid);
    int ret = 0;

    //for protect not do release the client when copybuffertoclient is doing
    ret = mAudioResourceManager->EnableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGER_LOCK, 500);
    if (ret)
    {
        ALOGW("ReleaseClient AUDIO_STREAMINMANAGER_LOCK fail ret = %d", ret);
    }

    ALOGD("ReleaseClient Client = %p clientid = %d mAudioInput.size  = %d", Client, clientid, mAudioInput.size());
    if (Client != NULL)
    {
        ALOGD("remove  mAudioInputcloient index = %d", index);
        delete mAudioInput.valueAt(index);
        mAudioInput.removeItem(clientid);
        //Client is deleted;
        Client = NULL;
    }

    mAudioResourceManager->DisableAudioLock(AudioResourceManagerInterface::AUDIO_STREAMINMANAGER_LOCK);
    ALOGD("ReleaseClient mAudioInput.size = %d", mAudioInput.size());
    return NO_ERROR;
}

void AudioMTKStreamInManager::SetInputMute(bool bEnable)
{
    if (mMicMute != bEnable)
    {
        mMicMute =  bEnable;
        mMuteTransition = false;
    }
}
void AudioMTKStreamInManager::BackupRecordDropTime(uint32 droptime)
{
    ALOGD("BackupRecordDropTime = %d", droptime);
    mBackUpRecordDropTime = droptime;
}

uint32 AudioMTKStreamInManager::GetRecordDropTime()
{
    ALOGD("GetRecordDropTime = %d", mBackUpRecordDropTime);
    return mBackUpRecordDropTime;
}

static short clamp16(int sample)
{
    if ((sample >> 15) ^ (sample >> 31))
    {
        sample = 0x7FFF ^ (sample >> 31);
    }
    return sample;
}

status_t AudioMTKStreamInManager::ApplyVolume(void *Buffer , uint32 BufferSize)
{
    // cehck if need apply mute
    if (mMicMute == true)
    {
        // do ramp down
        if (mMuteTransition == false)
        {
            uint32 count = BufferSize >> 1;
            float Volume_inverse = (float)(MTK_STREAMIN_VOLUEM_MAX / count) * -1;
            short *pPcm = (short *)Buffer;
            int ConsumeSample = 0;
            int value = 0;
            while (count)
            {
                value = *pPcm * (MTK_STREAMIN_VOLUEM_MAX + (Volume_inverse * ConsumeSample));
                *pPcm = clamp16(value >> MTK_STREAMIN_VOLUME_VALID_BIT);
                pPcm++;
                count--;
                ConsumeSample ++;
                //ALOGD("ApplyVolume Volume_inverse = %f ConsumeSample = %d",Volume_inverse,ConsumeSample);
            }
            mMuteTransition = true;
        }
        else
        {
            memset(Buffer, 0, BufferSize);
        }
    }
    else if (mMicMute == false)
    {
        // do ramp up
        if (mMuteTransition == false)
        {
            uint32 count = BufferSize >> 1;
            float Volume_inverse = (float)(MTK_STREAMIN_VOLUEM_MAX / count);
            short *pPcm = (short *)Buffer;
            int ConsumeSample = 0;
            int value = 0;
            while (count)
            {
                value = *pPcm * (Volume_inverse * ConsumeSample);
                *pPcm = clamp16(value >> MTK_STREAMIN_VOLUME_VALID_BIT);
                pPcm++;
                count--;
                ConsumeSample ++;
                //ALOGD("ApplyVolume Volume_inverse = %f ConsumeSample = %d",Volume_inverse,ConsumeSample);
            }
            mMuteTransition = true;
        }
    }
    return NO_ERROR;
}


AudioMTKStreamInManager::AudioMTkRecordThread::AudioMTkRecordThread(AudioMTKStreamInManager *AudioManager, uint32 Mem_type, char *RingBuffer, uint32 BufferSize)
{
    ALOGD("AudioMTkRecordThread constructor Mem_type = %d", Mem_type);

    struct sched_param sched_p;
    sched_getparam(0, &sched_p);
    sched_p.sched_priority = RTPM_PRIO_AUDIO_RECORD + 1;
    if (0 != sched_setscheduler(0, SCHED_RR, &sched_p))
    {
        ALOGE("[%s] failed, errno: %d", __func__, errno);
    }
    else
    {
        sched_p.sched_priority = RTPM_PRIO_AUDIO_RECORD + 1;
        sched_getparam(0, &sched_p);
        ALOGD("sched_setscheduler ok, priority: %d", sched_p.sched_priority);
    }

    mFd = 0;
    mMemType = Mem_type;
    mManager = AudioManager;

    char Buf[10];
    sprintf(Buf, "%d.pcm", DumpFileNum);

    if (WCNChipController::GetInstance()->BTUseCVSDRemoval() == true)
    {
#if !defined(EXTMD_LOOPBACK_TEST) //use VoIP to test. remark this to avoid VoIP MTKRecordThread controls AFE and kernel since they are controller by ExtMd threads
        if (mMemType == AudioDigitalType::MEM_DAI)
        {
            mFd2 = 0;

            mAudioBTCVSDControl = AudioBTCVSDControl::getInstance();
            if (!mAudioBTCVSDControl)
            {
                ALOGE("AudioBTCVSDControl::getInstance() fail");
            }

            mFd2 =  ::open(kBTDeviceName, O_RDWR);
            if (mFd2 <= 0)
            {
                ALOGW("open device fail");
            }
        }
#endif
    }

    switch (mMemType)
    {
        case AudioDigitalType::MEM_VUL:
            mName = String8("AudioMTkRecordThreadVUL");
            DumpFileName = String8(streaminmanager);
            DumpFileName.append((const char *)Buf);
            ALOGD("AudioMTkRecordThread DumpFileName = %s", DumpFileName.string());
            mPAdcPCMDumpFile = NULL;
            mPAdcPCMDumpFile = AudioOpendumpPCMFile(DumpFileName, streamin_propty);
            break;
        case AudioDigitalType::MEM_DAI:
            mName = String8("AudioMTkRecordThreadDAI");
            DumpFileName = String8(streaminmanager);
            DumpFileName.append((const char *)Buf);
            mPDAIInPCMDumpFile = NULL;
            mPDAIInPCMDumpFile = AudioOpendumpPCMFile(DumpFileName, streamin_propty);
            if (WCNChipController::GetInstance()->BTUseCVSDRemoval() == true)
            {
#if !defined(EXTMD_LOOPBACK_TEST)
                mAudioBTCVSDControl->BT_SCO_RX_Begin(mFd2);
#endif
            }
            break;
        case AudioDigitalType::MEM_AWB:
            mName = String8("AudioMTkRecordThreadAWB");
            DumpFileName = String8(streaminmanager);
            DumpFileName.append((const char *)Buf);
            mPI2SPCMDumpFile = NULL;
            mPI2SPCMDumpFile = AudioOpendumpPCMFile(DumpFileName, streamin_propty);
            break;
        default:
            ALOGD("NO support for memory interface");
            break;
    }

    if (mFd == 0)
    {
        mFd =  ::open(kAudioDeviceName, O_RDWR);
        if (mFd <= 0)
        {
            ALOGW("open device fail");
        }
    }
#ifndef EXTMD_LOOPBACK_TEST
    ::ioctl(mFd, START_MEMIF_TYPE, mMemType);
#endif

    // ring buffer to copy data into this ring buffer
    DumpFileNum++;
    DumpFileNum %= MAX_DUMP_NUM;
    mRingBuffer = RingBuffer;
    mBufferSize = BufferSize;

    mManager->mMaxProcessTime = 0;

    memset(&mEnterTime, 0, sizeof(timespec));
    memset(&mFinishtime, 0, sizeof(timespec));
    mStart = false;
    readperiodtime = 0;

}

AudioMTKStreamInManager::AudioMTkRecordThread::~AudioMTkRecordThread()
{
    ALOGD("+AudioMTkRecordThread()");
    ClosePcmDumpFile();

    if (mMemType == AudioDigitalType::MEM_DAI)
    {
        if (WCNChipController::GetInstance()->BTUseCVSDRemoval() == true)
        {
            mAudioBTCVSDControl->BT_SCO_RX_End(mFd2);

            if (mFd2)
            {
                ::close(mFd2);
                mFd2 = 0;
            }
        }
    }
    else
        // do thread exit routine
    {
#ifndef EXTMD_LOOPBACK_TEST
        if (mFd)
        {
            ALOGD("threadLoop exit STANDBY_MEMIF_TYPE mMemType = %d", mMemType);
            ::ioctl(mFd, STANDBY_MEMIF_TYPE, mMemType);
        }
#endif
    }

#ifndef EXTMD_LOOPBACK_TEST
    if (mFd)
    {
        ::close(mFd);
        mFd = 0;
    }
#endif
    ALOGD("-AudioMTkRecordThread()");
}

void AudioMTKStreamInManager::AudioMTkRecordThread::onFirstRef()
{
    ALOGD("AudioMTkRecordThread onFirstRef");
    tempdata = 0;
    if (mMemType == AudioDigitalType::MEM_VUL)
    {
        mRecordDropms = AUDIO_RECORD_DROP_MS;
    }
    else
    {
        mRecordDropms = 0;
    }

    mManager->BackupRecordDropTime(mRecordDropms);
}

// Good place to do one-time initializations
status_t  AudioMTKStreamInManager::AudioMTkRecordThread::readyToRun()
{
    ALOGD("AudioMTkRecordThread::readyToRun()");

    if (mMemType == AudioDigitalType::MEM_VUL)
    {
        DropRecordData();
    }
    return NO_ERROR;
}

void AudioMTKStreamInManager::AudioMTkRecordThread::WritePcmDumpData()
{
    int written_data = 0;
    switch (mMemType)
    {
        case AudioDigitalType::MEM_VUL:
            if (mPAdcPCMDumpFile)
            {
                long int position = 0;
                position = ftell(mPAdcPCMDumpFile);
                if (position > MAX_FILE_LENGTH)
                {
                    rewind(mPAdcPCMDumpFile);
                }
                //written_data = fwrite((void *)mRingBuffer, 1, mBufferSize, mPAdcPCMDumpFile);
                AudioDumpPCMData((void *)mRingBuffer , mBufferSize, mPAdcPCMDumpFile);
            }
            break;
        case AudioDigitalType::MEM_DAI:
            if (mPDAIInPCMDumpFile)
            {
                long int position = 0;
                position = ftell(mPDAIInPCMDumpFile);
                if (position > MAX_FILE_LENGTH)
                {
                    rewind(mPDAIInPCMDumpFile);
                }
                //written_data = fwrite((void *)mRingBuffer, 1, mBufferSize, mPDAIInPCMDumpFile);
                AudioDumpPCMData((void *)mRingBuffer , mBufferSize, mPDAIInPCMDumpFile);
            }
            break;
        case AudioDigitalType::MEM_AWB:
            if (mPI2SPCMDumpFile)
            {
                long int position = 0;
                position = ftell(mPI2SPCMDumpFile);
                if (position > MAX_FILE_LENGTH)
                {
                    rewind(mPI2SPCMDumpFile);
                }
                //written_data = fwrite((void *)mRingBuffer, 1, mBufferSize, mPI2SPCMDumpFile);
                AudioDumpPCMData((void *)mRingBuffer , mBufferSize, mPI2SPCMDumpFile);
            }
            break;
    }
}

void AudioMTKStreamInManager::AudioMTkRecordThread::ClosePcmDumpFile()
{
    ALOGD("ClosePcmDumpFile");
    switch (mMemType)
    {
        case AudioDigitalType::MEM_VUL:
            if (mPAdcPCMDumpFile)
            {
                AudioCloseDumpPCMFile(mPAdcPCMDumpFile);
                ALOGD("ClosePcmDumpFile mPAdcPCMDumpFile");
            }
            break;
        case AudioDigitalType::MEM_DAI:
            if (mPDAIInPCMDumpFile)
            {
                AudioCloseDumpPCMFile(mPDAIInPCMDumpFile);
                ALOGD("ClosePcmDumpFile mPDAIInPCMDumpFile");
            }
            break;
        case AudioDigitalType::MEM_AWB:
            if (mPI2SPCMDumpFile)
            {
                AudioCloseDumpPCMFile(mPI2SPCMDumpFile);
                ALOGD("ClosePcmDumpFile mPI2SPCMDumpFile");
            }
            break;
    }
}

void AudioMTKStreamInManager::AudioMTkRecordThread::DropRecordData()
{
    int Read_Size = 0;
    int Read_Buffer_Size = 0;
    // drop data for pop
    if (mRecordDropms != 0)
    {
        if (mRecordDropms > RECORD_DROP_MS_MAX)
        {
            mRecordDropms = RECORD_DROP_MS_MAX;
        }
        Read_Buffer_Size = ((mManager->getMemVULSamplerate() * mRecordDropms << 2) / 1000);
        ALOGD("1. DropRecordData Read_Buffer_Size = %d Read_Size = %d", Read_Buffer_Size, Read_Size);
    }
    while (Read_Buffer_Size > 0)
    {
        if (Read_Buffer_Size > mBufferSize)
        {
            Read_Size = ::read(mFd, mRingBuffer, mBufferSize);
        }
        else
        {
            Read_Size = ::read(mFd, mRingBuffer, Read_Buffer_Size);
        }
        Read_Buffer_Size -= mBufferSize;
        ALOGD("DropRecordData Read_Buffer_Size = %d Read_Size = %d", Read_Buffer_Size, Read_Size);
    }
}

bool AudioMTKStreamInManager::AudioMTkRecordThread::threadLoop()
{
    uint32 Read_Size = 0;
    //MTK_VOIP_ENHANCEMENT_SUPPORT debug purpose+++
    unsigned long long timecheck = 0;
    //---

    while (!(exitPending() == true))
    {
        if ((WCNChipController::GetInstance()->BTUseCVSDRemoval() == true)&&(mMemType == AudioDigitalType::MEM_DAI))
        {
#if defined(__MSBC_CODEC_SUPPORT__)
            if (mAudioBTCVSDControl->BT_SCO_isWideBand())
            {
                btsco_mSBC_RX_main();
            }
            else
#endif
            {
#ifndef EXTMD_LOOPBACK_TEST
                btsco_cvsd_RX_main();
#endif
            }
            return true;
        }
        else
        {

            //ALOGD("AudioMTkRecordThread threadLoop() read mBufferSize = %d mRingBuffer = %p ", mBufferSize, mRingBuffer);
            //MTK_VOIP_ENHANCEMENT_SUPPORT debug purpose+++
            mEnterTime = mManager->GetSystemTime(false);
            if (mStart == true)
            {
                timecheck = mManager->ProcessTimeCheck(mEnterTime, mFinishtime);
                if (timecheck > readperiodtime)
                {
                    readperiodtime = timecheck;
                    ALOGD("AudioMTkRecordThread readperiodtime=%lld", readperiodtime);
                    if ((timecheck / 1000000) > 60)
                    {
                        ALOGW("AudioMTkRecordThread, readperiodtime too long? sec %ld nsec %ld, sec %ld nsec %ld", mEnterTime.tv_sec, mEnterTime.tv_nsec, mFinishtime.tv_sec, mFinishtime.tv_nsec);
                    }
                    if ((timecheck / 1000000) > 90) //one hardware buffer time
                    {
                        //ASSERT_FTRACE(0);
                        ALOGW("AudioMTkRecordThread something wrong?");
                    }
                }
            }
            //---

            int ret_ms = ::ioctl(mFd, AUDDRV_GET_UL_REMAINDATA_TIME, NULL);
            //int ret_ms = 0;

            Read_Size = ::read(mFd, mRingBuffer, mBufferSize);

            struct timespec tempTime = mEnterTime;

            if (ret_ms != 0)
            {
                if ((tempTime.tv_nsec - ret_ms * 1000000) >= 0)
                {
                    tempTime.tv_nsec = tempTime.tv_nsec - ret_ms * 1000000;
                }
                else
                {
                    tempTime.tv_sec = tempTime.tv_sec - 1;
                    tempTime.tv_nsec = 1000000000 + tempTime.tv_nsec - ret_ms * 1000000;
                }
            }
            //ALOGD("input tempTime sec= %ld, nsec=%ld, ret_ms=%d" , tempTime.tv_sec, tempTime.tv_nsec, ret_ms);

            struct AdditionalInfo_STRUCT tempAddInfo;
            tempAddInfo.bHasAdditionalInfo = true;//false;
            tempAddInfo.timestamp_info = tempTime;

            //MTK_VOIP_ENHANCEMENT_SUPPORT debug purpose+++
            mFinishtime = mManager->GetSystemTime(false);
            //timecheck = mManager->ProcessTimeCheck(mFinishtime, mEnterTime);
            mStart = true;
            //if((timecheck/1000000)>70)
            //{
            //ALOGW("AudioMTkRecordThread, process too long? sec %ld nsec %ld, sec %ld nsec %ld", mEnterTime.tv_sec, mEnterTime.tv_nsec, mFinishtime.tv_sec, mFinishtime.tv_nsec);
            //}
            //ALOGD("AudioMTkRecordThread read finish");
            //---

            WritePcmDumpData();
            mManager->ApplyVolume(mRingBuffer, mBufferSize);

            if (IsAudioSupportFeature(AUDIO_SUPPORT_DMIC))
            {
                uint32 consume = 0;
                size_t inputLength = mBufferSize;
                size_t outputLength = BliOutBufferSizeDMIC;
                if (mManager->mBliHandlerDMIC)
                {
                    //                ALOGD("mBufferSize=%d, outputLength=%d",mBufferSize,outputLength);
                    consume = BLI_Convert(mManager->mBliHandlerDMIC, (int16_t *)mRingBuffer, &inputLength, (int16_t *)mManager->mBliOutputBufferDMIC, &outputLength);
                    //                ALOGD("consume=%d, inputLength=%d, outputLength=%d",consume,inputLength,outputLength);
                    mManager->CopyBufferToClient(mMemType, (void *)mManager->mBliOutputBufferDMIC, outputLength, tempAddInfo);
                }
                else
                {
                    mManager->CopyBufferToClient(mMemType, (void *)mRingBuffer, mBufferSize, tempAddInfo);
                }
            }
            else
            {
                mManager->CopyBufferToClient(mMemType, (void *)mRingBuffer, mBufferSize, tempAddInfo);
            }
            return true;

        }

    }
    ALOGD("threadLoop exit");
    return false;
}

void AudioMTKStreamInManager::AudioMTkRecordThread::btsco_cvsd_RX_main(void)
{
    uint8_t packetvalid, *outbuf, *workbuf, *tempbuf, *inbuf;
    uint32_t i, Read_Size, outsize, workbufsize, insize, bytes, offset;

    ALOGD("btsco_cvsd_RX_main(+)");
    Read_Size = ::read(mFd2, mAudioBTCVSDControl->BT_SCO_RX_GetCVSDTempInBuf(), BTSCO_CVSD_RX_TEMPINPUTBUF_SIZE);
    ALOGD("btsco_cvsd_RX_main ::read() done Read_Size=%d", Read_Size);

    outbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDOutBuf();
    outsize = SCO_RX_PCM8K_BUF_SIZE;
    workbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDWorkBuf();
    workbufsize = SCO_RX_PCM64K_BUF_SIZE;
    tempbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDTempInBuf();
    inbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDInBuf();
    insize = SCO_RX_PLC_SIZE;
    bytes = BTSCO_CVSD_RX_INBUF_SIZE;
    i = 0;
    offset = 0;
    do
    {
        packetvalid = *((char *)tempbuf + SCO_RX_PLC_SIZE + offset + i * BTSCO_CVSD_PACKET_VALID_SIZE); //parser packet valid info for each 30-byte packet
        //packetvalid   = 1; //force packvalid to 1 for test
        memcpy(inbuf + offset, tempbuf + offset + i * BTSCO_CVSD_PACKET_VALID_SIZE, SCO_RX_PLC_SIZE);

#ifdef BTCVSD_TEST_HW_ONLY
        WritePcmDumpData(inbuf + offset, insize);
#endif

        ALOGVV("btsco_process_RX_CVSD(+) insize=%d,outsize=%d,packetvalid=%d ", insize, outsize, packetvalid);
        mAudioBTCVSDControl->btsco_process_RX_CVSD(inbuf + offset, &insize, outbuf, &outsize, workbuf, workbufsize, packetvalid);
        offset += SCO_RX_PLC_SIZE;
        bytes -= insize;
        ALOGVV("btsco_process_RX_CVSD(-) consumed=%d,outsize=%d, bytes=%d", insize, outsize, bytes);
        mRingBuffer = (char *)outbuf;
        mBufferSize = outsize;
        WritePcmDumpData();

        mManager->ApplyVolume(outbuf, outsize);

        uint32 consume = 0;
        size_t inputLength = outsize;
        size_t outputLength = mManager->BliOutBufferSizeDAIBT;
        struct AdditionalInfo_STRUCT tempAddInfo;
        tempAddInfo.bHasAdditionalInfo = false;

        if (mManager->mBliHandlerDAIBT)
        {
            //ALOGD("BTRX: mBufferSize=%d, outputLength=%d",mBufferSize,outputLength);
            consume = BLI_Convert(mManager->mBliHandlerDAIBT, (int16_t *)outbuf, &inputLength, (int16_t *)mManager->mBliOutputBufferDAIBT, &outputLength);
            //ALOGD("BTRX: consume=%d, inputLength=%d, outputLength=%d",consume,inputLength,outputLength);
            mManager->CopyBufferToClient(mMemType, (void *)mManager->mBliOutputBufferDAIBT, outputLength, tempAddInfo);
        }
        else
        {
            mManager->CopyBufferToClient(mMemType, (void *)outbuf, outsize, tempAddInfo);
        }

        outsize = SCO_RX_PCM8K_BUF_SIZE;
        insize = SCO_RX_PLC_SIZE;
        i++;
    }
    while (bytes > 0);
    ALOGVV("btsco_cvsd_RX_main(-)");
}

#if defined(__MSBC_CODEC_SUPPORT__)
void AudioMTKStreamInManager::AudioMTkRecordThread::btsco_mSBC_RX_main(void)
{
    uint8_t packetvalid, *outbuf, *workbuf, *tempbuf, *inbuf;
    uint32_t i, Read_Size, outsize, workbufsize, insize, bytes, offset;

    ALOGD("btsco_mSBC_RX_main(+)");

    Read_Size = ::read(mFd2, mAudioBTCVSDControl->BT_SCO_RX_GetCVSDTempInBuf(), BTSCO_CVSD_RX_TEMPINPUTBUF_SIZE);

    outbuf = mAudioBTCVSDControl->BT_SCO_RX_GetMSBCOutBuf();
    outsize = MSBC_PCM_FRAME_BYTE;
    workbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDWorkBuf();
    workbufsize = SCO_RX_PCM64K_BUF_SIZE;
    tempbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDTempInBuf();
    inbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDInBuf();
    insize = SCO_RX_PLC_SIZE;
    bytes = BTSCO_CVSD_RX_INBUF_SIZE;
    i = 0;
    offset = 0;
    do
    {
        outsize = MSBC_PCM_FRAME_BYTE;
        insize = SCO_RX_PLC_SIZE;

        packetvalid = *((char *)tempbuf + SCO_RX_PLC_SIZE + offset + i * BTSCO_CVSD_PACKET_VALID_SIZE); //parser packet valid info for each 30-byte packet

        memcpy(inbuf + offset, tempbuf + offset + i * BTSCO_CVSD_PACKET_VALID_SIZE, SCO_RX_PLC_SIZE);

        ALOGD("btsco_process_RX_MSBC(+) insize=%d,outsize=%d,packetvalid=%d ", insize, outsize, packetvalid);
        mAudioBTCVSDControl->btsco_process_RX_MSBC(inbuf + offset, &insize, outbuf, &outsize, workbuf, workbufsize, packetvalid);
        offset += SCO_RX_PLC_SIZE;
        bytes -= insize;
        ALOGD("btsco_process_RX_MSBC(-) consumed=%d,outsize=%d, bytes=%d", insize, outsize, bytes);

        if (outsize != 0)
        {
            mRingBuffer = (char *)outbuf;
            mBufferSize = outsize;
            WritePcmDumpData();

            mManager->ApplyVolume(outbuf, outsize);
#if 0
            mManager->CopyBufferToClient(mMemType, (void *)outbuf, outsize);
#else


            uint32 consume = 0;
            size_t inputLength = outsize;
            size_t outputLength = mManager->BliOutBufferSizeDAIBT;
            struct AdditionalInfo_STRUCT tempAddInfo;
            tempAddInfo.bHasAdditionalInfo = false;

            if (mManager->mBliHandlerDAIBT)
            {
                ALOGD("BTRX: mBufferSize=%d, outputLength=%d", mBufferSize, outputLength);
                consume = BLI_Convert(mManager->mBliHandlerDAIBT, (int16_t *)outbuf, &inputLength, (int16_t *)mManager->mBliOutputBufferDAIBT, &outputLength);
                ALOGD("BTRX: consume=%d, inputLength=%d, outputLength=%d", consume, inputLength, outputLength);
                mManager->CopyBufferToClient(mMemType, (void *)mManager->mBliOutputBufferDAIBT, outputLength, tempAddInfo);
            }
            else
            {
                mManager->CopyBufferToClient(mMemType, (void *)outbuf, outsize, tempAddInfo);
            }
#endif

        }

        i++;
    }
    while (bytes > 0);
    ALOGD("btsco_mSBC_RX_main(-)");
}
#endif

uint32 AudioMTKStreamInManager::getMemVULSamplerate(void)
{
    if (IsAudioSupportFeature(AUDIO_SUPPORT_DMIC))
    {
        return AudioMTKStreamInManager::MemVULSamplerate_DMIC;
    }
    else
    {
        return AudioMTKStreamInManager::MemVULSamplerate_AMIC;
    }
}


}
