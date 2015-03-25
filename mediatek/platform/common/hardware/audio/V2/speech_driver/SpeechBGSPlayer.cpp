#include <sys/time.h>

#include "SpeechBGSPlayer.h"
#include "SpeechDriverInterface.h"

#include <utils/threads.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "BGSPlayBuffer"

#ifndef bgs_msleep
#define bgs_msleep(ms) usleep((ms)*1000)
#endif

namespace android
{
#define BGS_TARGET_SAMPLE_RATE  (16000)
#define BGS_CHANNEL_NUM         (1)

#define BGS_PLAY_BUFFER_LEN     (3072)  // AudioMTKStreamOut write max 16384 bytes * (16000Hz / 44100Hz) * (1ch / 2ch) = 2972 bytes

#ifdef BGS_USE_SINE_WAVE
static const uint16_t table_1k_tone_16000_hz[] =
{
    0x0000, 0x30FC, 0x5A82, 0x7641,
    0x7FFF, 0x7641, 0x5A82, 0x30FB,
    0x0001, 0xCF05, 0xA57E, 0x89C0,
    0x8001, 0x89BF, 0xA57E, 0xCF05
};
static const uint32_t kSizeSinewaveTable = sizeof(table_1k_tone_16000_hz);
#endif

BGSPlayBuffer::BGSPlayBuffer()
{
#ifdef DUMP_BGS_BLI_BUF
    struct tm *timeinfo;
    time_t rawtime;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char path[80];
    memset((void *)path, 0, 80);
    strftime(path, 80, "/sdcard/mtklog/audio_dump/%a_%b_%Y_%H_%M_%S_BGSBefore_BLI.pcm", timeinfo);
    ALOGD("fopen path is : %s", path);
    pOutFile = fopen(path, "w");
    if (pOutFile == NULL)
    {
        ALOGD("Fail to Open %s", path);
    }
#endif
}

status_t BGSPlayBuffer::InitBGSPlayBuffer(BGSPlayer *playPointer,
                                          uint32_t sampleRate,
                                          uint32_t chNum,
                                          int32_t format)
{
    ALOGD("InitBGSPlayBuffer sampleRate=%d ,ch=%d, format=%d", sampleRate, chNum, format);

    // keep the format
    ASSERT(format == AUDIO_FORMAT_PCM_16_BIT);
    mFormat = format;

    // set internal ring buffer
    mRingBuf.pBufBase = new char[BGS_PLAY_BUFFER_LEN];
    mRingBuf.bufLen   = BGS_PLAY_BUFFER_LEN;
    mRingBuf.pRead    = mRingBuf.pBufBase;
    mRingBuf.pWrite   = mRingBuf.pBufBase;

    SLOGV("%s(), pBufBase: %p, pRead: %u, pWrite: %u, bufLen:%u", __FUNCTION__,
          mRingBuf.pBufBase, mRingBuf.pRead - mRingBuf.pBufBase, mRingBuf.pWrite - mRingBuf.pBufBase, mRingBuf.bufLen);

    // set blisrc
    mBliSrc = new MtkAudioSrc(sampleRate, chNum, BGS_TARGET_SAMPLE_RATE, BGS_CHANNEL_NUM, SRC_IN_Q1P15_OUT_Q1P15);
    mBliSrc->Open();

    ALOGD("%s(), mBliSrc: %p", __FUNCTION__, mBliSrc);
    ASSERT(mBliSrc != NULL);

    // set blisrc converted buffer
    mBliOutputLinearBuffer = new char[BGS_PLAY_BUFFER_LEN];
    SLOGV("%s(), mBliOutputLinearBuffer = %p, size = %u", __FUNCTION__, mBliOutputLinearBuffer, BGS_PLAY_BUFFER_LEN);

    return NO_ERROR;
}

BGSPlayBuffer::~BGSPlayBuffer()
{
    mBGSPlayBufferMutex.lock();

    // delete blisrc handler buffer
    if (mBliSrc)
    {
        mBliSrc->Close();
        delete mBliSrc;
        mBliSrc = NULL;
    }

    // delete blisrc converted buffer
    delete[] mBliOutputLinearBuffer;

    // delete internal ring buffer
    delete[] mRingBuf.pBufBase;

#ifdef DUMP_BGS_BLI_BUF
    if (pOutFile != NULL)
    {
        fclose(pOutFile);
    }
#endif

    mBGSPlayBufferMutex.unlock();
}

uint32_t BGSPlayBuffer::Write(char *buf, uint32_t num)
{
    // lock
    mBGSPlayBufferMutex.lock();

    SLOGV("%s(), num = %u", __FUNCTION__, num);

#ifdef DUMP_BGS_BLI_BUF
    fwrite(buf, sizeof(char), num, pOutFile);
#endif

    uint32_t leftCount = num;
    uint16_t dataCountInBuf = 0;
    uint32_t tryCount = 0;
    while (tryCount < 10) // max mLatency = 92, max sleep (10 * 10) ms here
    {
        // BLISRC: output buffer: buf => local buffer: mRingBuf
        if (leftCount > 0)
        {
            // get free space in ring buffer
            uint32_t outCount = RingBuf_getFreeSpace(&mRingBuf);

            // do conversion
            ASSERT(mBliSrc != NULL);
            uint32_t consumed = leftCount;
            mBliSrc->Process((int16_t *)buf, &leftCount, (int16_t *)mBliOutputLinearBuffer, &outCount);
            consumed -= leftCount;

            buf += consumed;
            SLOGV("%s(), buf consumed = %u, leftCount = %u, outCount = %u",
                  __FUNCTION__, consumed, leftCount, outCount);

            // copy converted data to ring buffer //TODO(Harvey): try to reduce additional one memcpy here
            RingBuf_copyFromLinear(&mRingBuf, mBliOutputLinearBuffer, outCount);
            SLOGV("%s(), pRead:%u, pWrite:%u, dataCount:%u", __FUNCTION__,
                  mRingBuf.pRead - mRingBuf.pBufBase, mRingBuf.pWrite - mRingBuf.pBufBase, RingBuf_getDataCount(&mRingBuf));
        }

        // leave while loop
        if (leftCount <= 0)
        {
            break;
        }
      
        // wait modem side to retrieve data
        status_t retval = mBGSPlayBufferCondition.waitRelative(mBGSPlayBufferMutex, milliseconds(10));
        dataCountInBuf = RingBuf_getDataCount(&mRingBuf);
        if (retval != NO_ERROR)
        {
            SLOGV("%s(), tryCount = %u, leftCount = %u, dataCountInBuf = %u",
                  __FUNCTION__, tryCount, leftCount, dataCountInBuf);
            tryCount++;
        }

    }

    // leave warning message if need
    if (leftCount != 0 || dataCountInBuf != 0)
    {
        ALOGW("%s(), still leftCount = %u, dataCountInBuf = %u.", __FUNCTION__, leftCount, dataCountInBuf);
    }

    // unlock
    mBGSPlayBufferMutex.unlock();

    return num - leftCount;
}

//*****************************************************************************************
//--------------------------for LAD Player------------------------------------------
//*****************************************************************************************
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "BGSPlayer"

BGSPlayer *BGSPlayer::mBGSPlayer = NULL;
BGSPlayer *BGSPlayer::GetInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mBGSPlayer == NULL)
    {
        mBGSPlayer = new BGSPlayer();
    }
    ASSERT(mBGSPlayer != NULL);
    return mBGSPlayer;
}

BGSPlayer::BGSPlayer()
{
    // initial all table entry to zero, means non of them are occupied
    mBGSPlayBuffer = NULL;

#ifdef DUMP_BGS_DATA
    pOutFile = NULL;
#endif
}

BGSPlayer::~BGSPlayer()
{
    DestroyBGSPlayBuffer();

#ifdef DUMP_BGS_DATA
    if (pOutFile != NULL) { fclose(pOutFile); }
#endif
}

void BGSPlayer::CreateBGSPlayBuffer(uint32_t sampleRate, uint32_t chNum, int32_t format)
{
    ALOGD("CreateBGSPlayBuffer sampleRate=%u ,ch=%u, format=%d", sampleRate, chNum, format);

    // protection
    ASSERT(format == AUDIO_FORMAT_PCM_16_BIT);

    // check if free ID
    ASSERT(mBGSPlayBuffer == NULL);

    // create BGSPlayBuffer
    mBGSPlayBuffer = new BGSPlayBuffer();
    mBGSPlayBuffer->InitBGSPlayBuffer(this, sampleRate, chNum, format);
}

uint32_t BGSPlayer::Write(void *buf, uint32_t num)
{
    ASSERT(mBGSPlayBuffer != NULL);
    return mBGSPlayBuffer->Write((char *)buf, num);
}

void BGSPlayer::DestroyBGSPlayBuffer()
{
    ASSERT(mBGSPlayBuffer != NULL);

    delete mBGSPlayBuffer;
    mBGSPlayBuffer = NULL;
}

bool BGSPlayer::Open(SpeechDriverInterface *pSpeechDriver, uint8_t uplink_gain, uint8_t downlink_gain)
{
    // get Speech Driver (to open/close BGS)
    mSpeechDriver = pSpeechDriver;

#ifdef DUMP_BGS_DATA
    struct tm *timeinfo;
    time_t rawtime;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char path[80];
    memset((void *)path, 0, 80);
    strftime(path, 80, "/sdcard/mtklog/audio_dump/%a_%b_%Y_%H_%M_%S_RecBGS.pcm", timeinfo);
    ALOGD("fopen path is : %s", path);
    pOutFile = fopen(path, "w");
    if (pOutFile == NULL)
    {
        ALOGD("Fail to Open %s", path);
    }
#endif

    //no stream is inside player, should return false
    ASSERT(mBGSPlayBuffer != NULL);

    //turn on background sound
    mSpeechDriver->BGSoundOn();

    //recover the UL gain
    //backup Background Sound UL and DL gain
    //bcs we set them to zero when normal recording
    //we need to set it back when phone call recording
    mSpeechDriver->BGSoundConfig(uplink_gain, downlink_gain);

    return true;
}

uint32_t BGSPlayer::PutDataToSpeaker(char *target_ptr, uint16_t num_data_request)
{
    uint16_t write_count = 0;

#ifndef BGS_USE_SINE_WAVE
    if (mBGSPlayBuffer == NULL)
    {
        ALOGW("%s(), mBGSPlayBuffer == NULL, return 0.", __FUNCTION__);
        return 0;
    }

    mBGSPlayBuffer->mBGSPlayBufferMutex.lock();

    // check data count in mBGSPlayBuffer
    uint16_t dataCountInBuf = RingBuf_getDataCount(&mBGSPlayBuffer->mRingBuf);
    if (dataCountInBuf == 0) // no data in buffer, just return 0
    {
        SLOGV("%s(), dataCountInBuf == 0, return 0.", __FUNCTION__);
        mBGSPlayBuffer->mBGSPlayBufferMutex.unlock();
        return 0;
    }

    write_count = (dataCountInBuf >= num_data_request) ? num_data_request : dataCountInBuf;

    // copy to share buffer
    RingBuf_copyToLinear(target_ptr, &mBGSPlayBuffer->mRingBuf, write_count);
    SLOGV("%s(), pRead:%u, pWrite:%u, dataCount:%u", __FUNCTION__,
          mBGSPlayBuffer->mRingBuf.pRead - mBGSPlayBuffer->mRingBuf.pBufBase,
          mBGSPlayBuffer->mRingBuf.pWrite - mBGSPlayBuffer->mRingBuf.pBufBase,
          RingBuf_getDataCount(&mBGSPlayBuffer->mRingBuf));

    mBGSPlayBuffer->mBGSPlayBufferCondition.signal();
    mBGSPlayBuffer->mBGSPlayBufferMutex.unlock();
#else
    static uint32_t i4Count = 0;
    uint32_t current_count = 0, remain_count = 0;
    char *tmp_ptr = NULL;

    remain_count = write_count = num_data_request;
    tmp_ptr = target_ptr;

    if (remain_count > (kSizeSinewaveTable - i4Count))
    {
        memcpy(tmp_ptr, table_1k_tone_16000_hz + (i4Count >> 1), kSizeSinewaveTable - i4Count);
        tmp_ptr += (kSizeSinewaveTable - i4Count);
        remain_count -= (kSizeSinewaveTable - i4Count);
        i4Count = 0;
    }
    while (remain_count > kSizeSinewaveTable)
    {
        memcpy(tmp_ptr, table_1k_tone_16000_hz, kSizeSinewaveTable);
        tmp_ptr += kSizeSinewaveTable;
        remain_count -= kSizeSinewaveTable;
    }
    if (remain_count > 0)
    {
        memcpy(tmp_ptr, table_1k_tone_16000_hz, remain_count);
        i4Count = remain_count;
    }
#endif

#ifdef DUMP_BGS_DATA
    fwrite(target_ptr, sizeof(char), write_count, pOutFile);
#endif

    return write_count;
}

bool BGSPlayer::Close()
{
    SLOGV("%s()", __FUNCTION__);

    // tell modem side to close BGS
    mSpeechDriver->BGSoundOff();

#ifdef DUMP_BGS_DATA
    if (pOutFile != NULL) { fclose(pOutFile); }
#endif

    return true;
}

}; // namespace android


