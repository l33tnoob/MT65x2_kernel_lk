#include <unistd.h>
#include <string.h>

#include <utils/threads.h>

#include "SpeechPcm2way.h"
#include "SpeechType.h"

/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/
#define AUDIO_INPUT_BUFFER_SIZE  (16384) // 16k
#define AUDIO_OUTPUT_BUFFER_SIZE (16384) // 16k

namespace android
{

/***********************************************************
*
*   PCM2WAY Interface -  Play2Way
*
***********************************************************/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "Play2Way"

#ifdef PLAY2WAY_USE_SINE_WAVE
static const uint16_t table_1k_tone_8000_hz[] =
{
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,

    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F
};
static const uint32_t kSizeSinewaveTable = sizeof(table_1k_tone_8000_hz);
#endif


Play2Way *Play2Way::mPlay2Way = NULL;
Play2Way *Play2Way::GetInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mPlay2Way == NULL)
    {
        mPlay2Way = new Play2Way();
    }
    ASSERT(mPlay2Way != NULL);
    return mPlay2Way;
}

Play2Way::Play2Way()
{
    // Internal Output Buffer Initialization
    memset((void *)&m_OutputBuf, 0, sizeof(m_OutputBuf));
    m_OutputBuf.pBufBase = new char[AUDIO_OUTPUT_BUFFER_SIZE];
    m_OutputBuf.bufLen   = AUDIO_OUTPUT_BUFFER_SIZE;
    m_OutputBuf.pRead    = m_OutputBuf.pBufBase;
    m_OutputBuf.pWrite   = m_OutputBuf.pBufBase;

    ASSERT(m_OutputBuf.pBufBase != NULL);
    memset(m_OutputBuf.pBufBase, 0, m_OutputBuf.bufLen);

    mPlay2WayStarted = false;

#ifdef DUMP_MODEM_PCM2WAY_DATA
    pPlay2WayDumpFile = NULL;
#endif

    pthread_mutex_init(&pPlay2Way_Mutex, NULL);
}

Play2Way::~Play2Way()
{
    //if (pLad != NULL) pLad->pCCCI->Play2WayLock();

    Play2Way_BufLock();
    if (m_OutputBuf.pBufBase != NULL)
    {
        delete[] m_OutputBuf.pBufBase;
        m_OutputBuf.pBufBase = NULL;
        m_OutputBuf.bufLen   = 0;
        m_OutputBuf.pRead    = NULL;
        m_OutputBuf.pWrite   = NULL;
    }
    Play2Way_BufUnlock();

    //if (pLad != NULL) pLad->pCCCI->Play2WayUnLock();
}

void Play2Way::Play2Way_BufLock()
{
    pthread_mutex_lock(&pPlay2Way_Mutex);
}

void Play2Way::Play2Way_BufUnlock()
{
    pthread_mutex_unlock(&pPlay2Way_Mutex);
}

int Play2Way::Start()
{
    ALOGD("%s()", __FUNCTION__);

    // Reset read and write pointer of Output buffer
    Play2Way_BufLock();

    m_OutputBuf.bufLen   = AUDIO_OUTPUT_BUFFER_SIZE;
    m_OutputBuf.pRead    = m_OutputBuf.pBufBase;
    m_OutputBuf.pWrite   = m_OutputBuf.pBufBase;

    Play2Way_BufUnlock();

#ifdef DUMP_MODEM_PCM2WAY_DATA
    if (pPlay2WayDumpFile == NULL) 
    {     
        AudiocheckAndCreateDirectory("/sdcard/mtklog/audio_dump/Play2Way.pcm");
        pPlay2WayDumpFile = fopen("/sdcard/mtklog/audio_dump/Play2Way.pcm", "wb"); 
    }
    if (pPlay2WayDumpFile == NULL) 
    { 
        ALOGW("Fail to Open pPlay2WayDumpFile"); 
    }
#endif

    mPlay2WayStarted = true;

    return true;
}

int Play2Way::Stop()
{
    ALOGD("%s()", __FUNCTION__);
    Play2Way_BufLock();

    mPlay2WayStarted = false;

    Play2Way_BufUnlock();

#ifdef DUMP_MODEM_PCM2WAY_DATA
    if (pPlay2WayDumpFile != NULL)
    {
        fclose(pPlay2WayDumpFile);
    }
    else
    {
        ALOGW("%s(), pPlay2WayDumpFile == NULL!!!!!", __FUNCTION__);
    }
#endif

    return true;
}

int Play2Way::Write(void *buffer, int size_bytes)
{
    if (mPlay2WayStarted == false)
    {
        ALOGE("%s(), mPlay2WayStarted == false, return", __FUNCTION__);
        return 0;
    }

    Play2Way_BufLock();

    uint32_t num_free_space = RingBuf_getFreeSpace(&m_OutputBuf);
    if (size_bytes > num_free_space)
    {
        ALOGE("%s(), size_bytes(%u) > num_free_space(%u), drop", __FUNCTION__, size_bytes, num_free_space);
        Play2Way_BufUnlock();
        return 0;
    }

    RingBuf_copyFromLinear(&m_OutputBuf, (char *)buffer, size_bytes);

    Play2Way_BufUnlock();

    return size_bytes;
}

/** get free space of internal buffer */
int Play2Way::GetFreeBufferCount()
{
    int freeSpaceInpBuf = m_OutputBuf.bufLen - RingBuf_getDataCount(&m_OutputBuf);
    SLOGV("%s(), buf_cnt:%d, free_cnt:%d", __FUNCTION__, RingBuf_getDataCount(&m_OutputBuf), freeSpaceInpBuf);
    return freeSpaceInpBuf;
}


uint32_t Play2Way::PutDataToSpeaker(char *target_ptr, uint16_t num_data_request)
{
    SLOGV("%s(), pcm_dataRequest=%d", __FUNCTION__, num_data_request);

    Play2Way_BufLock();

#ifndef PLAY2WAY_USE_SINE_WAVE
    // check the output buffer data count
    int OutputBufDataCount = RingBuf_getDataCount(&m_OutputBuf);
    SLOGV("%s(), OutputBufDataCount=%d", __FUNCTION__, OutputBufDataCount);

    // if output buffer's data is not enough, fill it with zero to PCMdataToModemSize (ex: 320 bytes)
    if (OutputBufDataCount < num_data_request)
    {
        RingBuf_writeDataValue(&m_OutputBuf, 0, num_data_request - OutputBufDataCount);
        ALOGW("%s(), underflow OutBufSize:%d", __FUNCTION__, OutputBufDataCount);
    }

    // fill downlink data to share buffer
    RingBuf_copyToLinear(target_ptr, &m_OutputBuf, num_data_request);

    SLOGV("OutputBuf B:0x%p, R:%ld, W:%ld, L:%u", m_OutputBuf.pBufBase, m_OutputBuf.pRead - m_OutputBuf.pBufBase, m_OutputBuf.pWrite - m_OutputBuf.pBufBase, m_OutputBuf.bufLen);
#else
    static uint32_t i4Count = 0;
    uint32_t current_count = 0, remain_count = 0;
    char *tmp_ptr = NULL;

    remain_count = num_data_request;
    tmp_ptr = target_ptr;

    if (remain_count > (kSizeSinewaveTable - i4Count))
    {
        memcpy(tmp_ptr, table_1k_tone_8000_hz + (i4Count >> 1), kSizeSinewaveTable - i4Count);
        tmp_ptr += (kSizeSinewaveTable - i4Count);
        remain_count -= (kSizeSinewaveTable - i4Count);
        i4Count = 0;
    }
    //while (remain_count > kSizeSinewaveTable)
    if (remain_count > kSizeSinewaveTable)
    {
        memcpy(tmp_ptr, table_1k_tone_8000_hz, kSizeSinewaveTable);
        tmp_ptr += kSizeSinewaveTable;
        remain_count -= kSizeSinewaveTable;
    }
    if (remain_count > 0)
    {
        memcpy(tmp_ptr, table_1k_tone_8000_hz, remain_count);
        i4Count = remain_count;
    }
#endif

#ifdef DUMP_MODEM_PCM2WAY_DATA
    if (pPlay2WayDumpFile != NULL)
    {
        fwrite(target_ptr, sizeof(char), num_data_request, pPlay2WayDumpFile);
    }
    else
    {
        ALOGW("%s(), pPlay2WayDumpFile == NULL!!!!!", __FUNCTION__);
    }
#endif

    Play2Way_BufUnlock();
    return num_data_request;
}


/***********************************************************
*
*   PCM2WAY Interface -  Record2Way
*
***********************************************************/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "Record2Way"

Record2Way *Record2Way::mRecord2Way = NULL;

Record2Way *Record2Way::GetInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mRecord2Way == NULL)
    {
        mRecord2Way = new Record2Way();
    }
    return mRecord2Way;
}

Record2Way::Record2Way()
{
    // Internal Input Buffer Initialization
    memset((void *)&m_InputBuf, 0, sizeof(RingBuf));
    m_InputBuf.pBufBase = new char[AUDIO_INPUT_BUFFER_SIZE];
    m_InputBuf.bufLen   = AUDIO_INPUT_BUFFER_SIZE;
    m_InputBuf.pRead    = m_InputBuf.pBufBase;
    m_InputBuf.pWrite   = m_InputBuf.pBufBase;

    ASSERT(m_InputBuf.pBufBase != NULL);
    memset(m_InputBuf.pBufBase, 0, m_InputBuf.bufLen);

    m_Rec2Way_Started = false;

#ifdef DUMP_MODEM_PCM2WAY_DATA
    pRecord2WayDumpFile = NULL;
#endif
    
    pthread_mutex_init(&pRec2Way_Mutex, NULL);
}

Record2Way::~Record2Way()
{
    //if (pLad != NULL) pLad->pCCCI->Record2WayLock();

    Record2Way_BufLock();
    if (m_InputBuf.pBufBase != NULL)
    {
        delete []m_InputBuf.pBufBase;
        m_InputBuf.pBufBase = NULL;
        m_InputBuf.bufLen   = 0;
        m_InputBuf.pRead    = NULL;
        m_InputBuf.pWrite   = NULL;
    }
    Record2Way_BufUnlock();

    //if (pLad != NULL) pLad->pCCCI->Record2WayUnLock();
}

void Record2Way::Record2Way_BufLock()
{
    pthread_mutex_lock(&pRec2Way_Mutex);
}

void Record2Way::Record2Way_BufUnlock()
{
    pthread_mutex_unlock(&pRec2Way_Mutex);
}

int Record2Way::Start()
{
    ALOGD("%s()", __FUNCTION__);
    Record2Way_BufLock();

#ifdef DUMP_MODEM_PCM2WAY_DATA
    if (pRecord2WayDumpFile == NULL)
    {
        AudiocheckAndCreateDirectory("/sdcard/mtklog/audio_dump/Record2Way.pcm");
        pRecord2WayDumpFile = fopen("/sdcard/mtklog/audio_dump/Record2Way.pcm", "wb");
    }
    if (pRecord2WayDumpFile == NULL) 
    { 
        ALOGW("Fail to Open pRecord2WayDumpFile"); 
    }
#endif

    m_Rec2Way_Started = true;

    // Reset read and write pointer of Input buffer
    m_InputBuf.bufLen = AUDIO_INPUT_BUFFER_SIZE;
    m_InputBuf.pRead  = m_InputBuf.pBufBase;
    m_InputBuf.pWrite = m_InputBuf.pBufBase;

    Record2Way_BufUnlock();
    return true;
}

int Record2Way::Stop()
{
    ALOGD("%s()", __FUNCTION__);
    Record2Way_BufLock();

    m_Rec2Way_Started = false;

    Record2Way_BufUnlock();

#ifdef DUMP_MODEM_PCM2WAY_DATA
    if (pRecord2WayDumpFile != NULL)
    {
        fclose(pRecord2WayDumpFile);
    }
    else
    {
        ALOGW("%s(), pRecord2WayDumpFile == NULL!!!!!", __FUNCTION__);
    }
#endif

    return true;
}

#define READ_DATA_FROM_MODEM_FAIL_CNT 10

int Record2Way::Read(void *buffer, int size_bytes)
{
    int ret = 0;
    int InputBuf_dataCnt = 0;
    int ReadDataAgain = 0;
    int consume_byte = size_bytes;
    char *buf = (char *)buffer;

    if (m_Rec2Way_Started == false)
    {
        ALOGD("Record2Way_Read, m_Rec2Way_Started=false");
        return 0;
    }

    // if internal input buffer has enough data for this read, do it and return
    Record2Way_BufLock();
    InputBuf_dataCnt = RingBuf_getDataCount(&m_InputBuf);
    if (InputBuf_dataCnt >= consume_byte)
    {
        RingBuf_copyToLinear(buf, &m_InputBuf, consume_byte);
        Record2Way_BufUnlock();
        return consume_byte;
    }
    Record2Way_BufUnlock();


    // if internal input buffer is not enough, keep on trying
    while (ReadDataAgain++ < READ_DATA_FROM_MODEM_FAIL_CNT)
    {
        if (ReadDataAgain > (READ_DATA_FROM_MODEM_FAIL_CNT - 1))
        {
            ALOGW("Record2Way_Read, fail, No data from modem: %d (%d)", ReadDataAgain, InputBuf_dataCnt);
        }
        // Interrupt period of pcm2way driver is 20ms.
        // If wait too long time (150 ms),
        //  -- Modem side has problem, the no interrupt is issued.
        //  -- pcm2way driver is stop. So AP can't read the data from modem.

        //wait some time then get data again from modem.
        usleep(15 * 1000);
        //Read data from modem again
        Record2Way_BufLock();
        InputBuf_dataCnt = RingBuf_getDataCount(&m_InputBuf);
        if (InputBuf_dataCnt >= consume_byte)
        {
            RingBuf_copyToLinear((char *)buf, &m_InputBuf, consume_byte);
            Record2Way_BufUnlock();
            return consume_byte;
        }
        Record2Way_BufUnlock();
    }

    ALOGD("Record2Way_Read, Modem fail");
    return 0;
}

int Record2Way::GetBufferDataCount()
{
    Record2Way_BufLock();
    int InputBuf_dataCnt = RingBuf_getDataCount(&m_InputBuf);
    Record2Way_BufUnlock();

    return InputBuf_dataCnt;
}

void Record2Way::GetDataFromMicrophone(RingBuf ul_ring_buf)
{
    int InpBuf_freeSpace = 0;
    int ShareBuf_dataCnt = 0;

    Record2Way_BufLock();

    // get free space of internal input buffer
    InpBuf_freeSpace = RingBuf_getFreeSpace(&m_InputBuf);
    SLOGV("%s(), input_Buf data_cnt:%d, freeSpace:%d", __FUNCTION__, RingBuf_getDataCount(&m_InputBuf), InpBuf_freeSpace);

    // get data count in share buffer
    ShareBuf_dataCnt = RingBuf_getDataCount(&ul_ring_buf);
    SLOGV("%s(), share_Buf data_count:%d", __FUNCTION__, ShareBuf_dataCnt);

#ifdef DUMP_MODEM_PCM2WAY_DATA
    char linear_buffer[ShareBuf_dataCnt];

    char *pM2AShareBufEnd = ul_ring_buf.pBufBase + ul_ring_buf.bufLen;
    if (ul_ring_buf.pRead + ShareBuf_dataCnt <= pM2AShareBufEnd)
    {
        memcpy(linear_buffer, ul_ring_buf.pRead, ShareBuf_dataCnt);
    }
    else
    {
        uint32 r2e = pM2AShareBufEnd - ul_ring_buf.pRead;
        memcpy(linear_buffer, ul_ring_buf.pRead, r2e);
        memcpy((void *)(linear_buffer + r2e), ul_ring_buf.pBufBase, ShareBuf_dataCnt - r2e);
    }
    if (pRecord2WayDumpFile != NULL)
    {
        fwrite(linear_buffer, sizeof(char), ShareBuf_dataCnt, pRecord2WayDumpFile);
    }
    else
    {
        ALOGW("%s(), pRecord2WayDumpFile == NULL!!!!!", __FUNCTION__);
    }
#endif

    // check the data count in share buffer
    if (ShareBuf_dataCnt > 320)
    {
        SLOGV("%s(), ul_ring_buf size(%d) > 320", __FUNCTION__, ShareBuf_dataCnt);
    }

    // check free space for internal input buffer
    if (ShareBuf_dataCnt > InpBuf_freeSpace)
    {
        SLOGV("%s(), uplink buffer full", __FUNCTION__);
        Record2Way_BufUnlock();
        return;
    }

    // copy data from modem share buffer to internal input buffer
    RingBuf_copyEmpty(&m_InputBuf, &ul_ring_buf);

    SLOGV("%s(), InputBuf B:0x%p, R:%ld, W:%ld, L:%u", __FUNCTION__, m_InputBuf.pBufBase, m_InputBuf.pRead - m_InputBuf.pBufBase, m_InputBuf.pWrite - m_InputBuf.pBufBase, m_InputBuf.bufLen);
    SLOGV("%s(), M2A_ShareBuf B:0x%p, R:%ld, W:%ld, L:%u", __FUNCTION__, ul_ring_buf.pBufBase, ul_ring_buf.pRead - ul_ring_buf.pBufBase, ul_ring_buf.pWrite - ul_ring_buf.pBufBase, ul_ring_buf.bufLen);

    Record2Way_BufUnlock();
}

} // end of namespace android
