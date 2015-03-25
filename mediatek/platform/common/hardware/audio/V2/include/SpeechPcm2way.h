#ifndef ANDROID_SPEECH_PCM2WAY_H
#define ANDROID_SPEECH_PCM2WAY_H

#include <pthread.h>

#include "AudioType.h"
#include "AudioUtility.h"

//#define DUMP_MODEM_PCM2WAY_DATA
//#define PLAY2WAY_USE_SINE_WAVE

namespace android
{
// for debug
//#define DUMP_MODEM_PCM2WAY_DATA
//#define PLAY2WAY_USE_SINE_WAVE

#define PCM2WAY_PLAY_BUFFER_NB_LEN (320) // Mono * PCM-16Bit *  8000(hz) * 20(ms) = 320 bytes
#define PCM2WAY_PLAY_BUFFER_WB_LEN (640) // Mono * PCM-16Bit * 16000(hz) * 20(ms) = 640 bytes

/***********************************************************
*   PCM2WAY Interface -  Play2Way
***********************************************************/

class Play2Way
{
    public:
        virtual ~Play2Way();

        static Play2Way    *GetInstance();

        int                 Start();
        int                 Stop();
        int                 Write(void *buffer, int size_bytes);
        int                 GetFreeBufferCount(void);
        uint32_t            PutDataToSpeaker(char *target_ptr, uint16_t num_data_request);

    private:
        Play2Way();

        void                Play2Way_BufLock();
        void                Play2Way_BufUnlock();

        static Play2Way    *mPlay2Way; // singleton

        bool                mPlay2WayStarted;
        RingBuf             m_OutputBuf;      // Internal Output Buffer for Put Data to Modem via Receive(Speaker)
        pthread_mutex_t     pPlay2Way_Mutex;  // Mutex to protect internal buffer

#ifdef DUMP_MODEM_PCM2WAY_DATA
        FILE               *pPlay2WayDumpFile;
#endif
};

/***********************************************************
*   PCM2WAY Interface -  Record2Way
***********************************************************/
class Record2Way
{
    public:
        virtual ~Record2Way();

        static Record2Way *GetInstance();

        int                 Start();
        int                 Stop();
        int                 Read(void *buffer, int size_bytes);
        int                 GetBufferDataCount(void);
        void                GetDataFromMicrophone(RingBuf ul_ring_buf);

    private:
        Record2Way();

        void                Record2Way_BufLock();
        void                Record2Way_BufUnlock();

        static Record2Way  *mRecord2Way; // singleton

        bool                m_Rec2Way_Started;
        RingBuf             m_InputBuf;     // Internal Input Buffer for Get From Microphone Data
        pthread_mutex_t     pRec2Way_Mutex; // Mutex to protect internal buffer

#ifdef DUMP_MODEM_PCM2WAY_DATA
        FILE               *pRecord2WayDumpFile;
#endif
};



}; // namespace android

#endif
