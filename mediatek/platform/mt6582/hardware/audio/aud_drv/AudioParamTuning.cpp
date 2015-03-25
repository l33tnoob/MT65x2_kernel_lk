/*******************************************************************************
 *
 * Filename:
 * ---------
 * AudioParamTuning.cpp
 *
 * Project:
 * --------
 *   Android
 *
 * Description:
 * ------------
 *   This file implements the method for  handling param tuning.
 *
 * Author:
 * -------
 *   Donglei Ji (mtk80823)
 *******************************************************************************/

#include <unistd.h>
#include <sched.h>
#include <sys/prctl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <cutils/xlog.h>

#include "AudioParamTuning.h"
#include "AudioCustParam.h"
//#include "AudioUtility.h"

#include "AudioVolumeFactory.h"
#include "AudioAnalogControlFactory.h"
#include "AudioDigitalControlFactory.h"
#include "SpeechDriverInterface.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

//#define PLAYBUF_SIZE 6400l
#define PLAYBUF_SIZE 16384
#define A2M_SHARED_BUFFER_OFFSET  (1408)
#define WAV_HEADER_SIZE 44

// define in AudioMtkVolumeControler.cpp
#define  AUDIO_BUFFER_HW_GAIN_STEP (13)

#undef WRITE_RECORDDATA_ON_APSIDEDMNR

#define LOG_TAG "AudioParamTuning"
namespace android
{
//digital gain map
static const float Volume_Mapping_Step = 256.0f;
uint32 MapVoiceVolumetoCustom(uint32 mode, uint32 gain)
{
    uint32 mappingGain = 0;
    if (gain > VOICE_VOLUME_MAX)
    {
        gain = VOICE_VOLUME_MAX;
    }

    float degradeDb = (VOICE_VOLUME_MAX - gain) / VOICE_ONEDB_STEP;
    mappingGain = (uint32)(Volume_Mapping_Step - (degradeDb * 4));
    SXLOGD("MapVoiceVolumetoCustom - gain:%d, mappingGain:%d", gain, mappingGain);

    return mappingGain;
}

const uint16_t digitOnly_quater_dB_tableForSpeech[264] =
{
    4096, 3980, 3867, 3757, /* 0   ~ -0.75   dB*/
    3645, 3547, 3446, 3349, /* -1  ~ -1.75   dB*/ // downlink begin (-1db == 3645 == E3D)
    3254, 3161, 3072, 2984, /* -2  ~ -2.75   dB*/
    2900, 2817, 2738, 2660, /* -3  ~ -3.75   dB*/
    2584, 2511, 2440, 2371, /* -4  ~ -4.75   dB*/
    2303, 2238, 2175, 2113, /* -5  ~ -5.75   dB*/
    2053, 1995, 1938, 1883, /* -6  ~ -6.75   dB*/
    1830, 1778, 1727, 1678, /* -7  ~ -7.75   dB*/
    1631, 1584, 1539, 1496, /* -8  ~ -8.75   dB*/
    1453, 1412, 1372, 1333, /* -9  ~ -9.75   dB*/
    1295, 1259, 1223, 1188, /* -10 ~ -10.75  dB*/
    1154, 1122, 1090, 1059, /* -11 ~ -11.75  dB*/
    1029, 1000, 971 , 944 , /* -12 ~ -12.75  dB*/
    917 , 891 , 866 , 841 , /* -13 ~ -13.75  dB*/
    817 , 794 , 772 , 750 , /* -14 ~ -14.75  dB*/
    728 , 708 , 688 , 668 , /* -15 ~ -15.75  dB*/
    649 , 631 , 613 , 595 , /* -16 ~ -16.75  dB*/
    579 , 562 , 546 , 531 , /* -17 ~ -17.75  dB*/
    516 , 501 , 487 , 473 , /* -18 ~ -18.75  dB*/
    460 , 447 , 434 , 422 , /* -19 ~ -19.75  dB*/
    410 , 398 , 387 , 376 , /* -20 ~ -20.75  dB*/
    365 , 355 , 345 , 335 , /* -21 ~ -21.75  dB*/
    325 , 316 , 307 , 298 , /* -22 ~ -22.75  dB*/
    290 , 282 , 274 , 266 , /* -23 ~ -23.75  dB*/
    258 , 251 , 244 , 237 , /* -24 ~ -24.75  dB*/
    230 , 224 , 217 , 211 , /* -25 ~ -25.75  dB*/
    205 , 199 , 194 , 188 , /* -26 ~ -26.75  dB*/
    183 , 178 , 173 , 168 , /* -27 ~ -27.75  dB*/
    163 , 158 , 154 , 150 , /* -28 ~ -28.75  dB*/
    145 , 141 , 137 , 133 , /* -29 ~ -29.75  dB*/
    130 , 126 , 122 , 119 , /* -30 ~ -30.75  dB*/
    115 , 112 , 109 , 106 , /* -31 ~ -31.75  dB*/
    103 , 100 , 97  , 94  , /* -32 ~ -32.75  dB*/
    92  , 89  , 87  , 84  , /* -33 ~ -33.75  dB*/
    82  , 79  , 77  , 75  , /* -34 ~ -34.75  dB*/
    73  , 71  , 69  , 67  , /* -35 ~ -35.75  dB*/
    65  , 63  , 61  , 60  , /* -36 ~ -36.75  dB*/
    58  , 56  , 55  , 53  , /* -37 ~ -37.75  dB*/
    52  , 50  , 49  , 47  , /* -38 ~ -38.75  dB*/
    46  , 45  , 43  , 42  , /* -39 ~ -39.75  dB*/
    41  , 40  , 39  , 38  , /* -40 ~ -40.75  dB*/
    37  , 35  , 34  , 33  , /* -41 ~ -41.75  dB*/
    33  , 32  , 31  , 30  , /* -42 ~ -42.75  dB*/
    29  , 28  , 27  , 27  , /* -43 ~ -43.75  dB*/
    26  , 25  , 24  , 24  , /* -44 ~ -44.75  dB*/
    23  , 22  , 22  , 21  , /* -45 ~ -45.75  dB*/
    21  , 20  , 19  , 19  , /* -46 ~ -46.75  dB*/
    18  , 18  , 17  , 17  , /* -47 ~ -47.75  dB*/
    16  , 16  , 15  , 15  , /* -48 ~ -48.75  dB*/
    15  , 14  , 14  , 13  , /* -49 ~ -49.75  dB*/
    13  , 13  , 12  , 12  , /* -50 ~ -50.75  dB*/
    12  , 11  , 11  , 11  , /* -51 ~ -51.75  dB*/
    10  , 10  , 10  , 9   , /* -52 ~ -52.75  dB*/
    9   , 9   , 9   , 8   , /* -53 ~ -53.75  dB*/
    8   , 8   , 8   , 7   , /* -54 ~ -54.75  dB*/
    7   , 7   , 7   , 7   , /* -55 ~ -55.75  dB*/
    6   , 6   , 6   , 6   , /* -56 ~ -56.75  dB*/
    6   , 6   , 5   , 5   , /* -57 ~ -57.75  dB*/
    5   , 5   , 5   , 5   , /* -58 ~ -58.75  dB*/
    5   , 4   , 4   , 4   , /* -59 ~ -59.75  dB*/
    4   , 4   , 4   , 4   , /* -60 ~ -60.75  dB*/
    4   , 4   , 3   , 3   , /* -61 ~ -61.75  dB*/
    3   , 3   , 3   , 3   , /* -62 ~ -62.75  dB*/
    3   , 3   , 3   , 3   , /* -63 ~ -63.75  dB*/
    3   , 3   , 2   , 2   , /* -64 ~ -64.75  dB*/
    2   , 2   , 2   , 2   , /* -65 ~ -65.75  dB*/
};

static void *Play_PCM_With_SpeechEnhance_Routine(void *arg)
{

    SXLOGD("Play_PCM_With_SpeechEnhance_Routine in +");
    AudioParamTuning *pAUDParamTuning = (AudioParamTuning *)arg;

    if (pAUDParamTuning == NULL)
    {
        SXLOGE("Play_PCM_With_SpeechEnhance_Routine pAUDParamTuning = NULL arg = %x", arg);
        return 0;
    }

    uint32 PCM_BUF_SIZE = pAUDParamTuning->m_bWBMode ? (2 * PLAYBUF_SIZE) : (PLAYBUF_SIZE);
    unsigned long sleepTime = (PLAYBUF_SIZE / 320) * 20 * 1000;
    // open AudioRecord
    pthread_mutex_lock(&pAUDParamTuning->mPPSMutex);

    // Adjust thread priority
    prctl(PR_SET_NAME, (unsigned long)"PlaybackWithSphEnRoutine", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
    //Prepare file pointer
    FILE *pFd = pAUDParamTuning->m_pInputFile;                 //file for input

    // ----start the loop --------
    pAUDParamTuning->m_bPPSThreadExit = false;
    char *tmp = new char[PLAYBUF_SIZE];

    int numOfBytesPlayed = 0;
    int playBufFreeCnt = 0;
    int cntR = 0;

    fread(tmp, sizeof(char), WAV_HEADER_SIZE, pFd);
    memset(tmp, 0, PCM_BUF_SIZE);

    SXLOGD("pthread_cond_signal(&pAUDParamTuning->mPPSExit_Cond), buffer size=%d", PCM_BUF_SIZE);
    pthread_cond_signal(&pAUDParamTuning->mPPSExit_Cond); // wake all thread
    pthread_mutex_unlock(&pAUDParamTuning->mPPSMutex);

    while ((!pAUDParamTuning->m_bPPSThreadExit) && pFd)
    {
        pthread_mutex_lock(&pAUDParamTuning->mPlayBufMutex);
        playBufFreeCnt = pAUDParamTuning->mPlay2WayInstance->GetFreeBufferCount() - 1;
        cntR = fread(tmp, sizeof(char), playBufFreeCnt, pFd);
        pAUDParamTuning->mPlay2WayInstance->Write(tmp, cntR);
        numOfBytesPlayed += cntR;
        SXLOGV(" Playback buffer, free:%d, read from :%d, total play:%d", playBufFreeCnt, cntR, numOfBytesPlayed);
        pthread_mutex_unlock(&pAUDParamTuning->mPlayBufMutex);

        if (cntR < playBufFreeCnt)
        {
            SXLOGD("File reach the end");
            usleep(sleepTime); ////wait to all data is played
            break;
        }

        usleep(sleepTime / 2);
    }

    // free buffer
    if (tmp != NULL)
    {
        delete[] tmp;
        tmp = NULL;
    }

    if (!pAUDParamTuning->m_bPPSThreadExit)
    {
        pAUDParamTuning->m_bPPSThreadExit = true;
        pAUDParamTuning->enableModemPlaybackVIASPHPROC(false);
        AudioTasteTuningStruct sRecoveryParam;
        sRecoveryParam.cmd_type = (unsigned short)AUD_TASTE_STOP;
        sRecoveryParam.wb_mode  = pAUDParamTuning->m_bWBMode;
        pAUDParamTuning->updataOutputFIRCoffes(&sRecoveryParam);
    }

    //exit thread
    SXLOGD("playbackRoutine pthread_mutex_lock");
    pthread_mutex_lock(&pAUDParamTuning->mPPSMutex);
    SXLOGD("pthread_cond_signal(&pAUDParamTuning->mPPSExit_Cond)");
    pthread_cond_signal(&pAUDParamTuning->mPPSExit_Cond); // wake all thread
    pthread_mutex_unlock(&pAUDParamTuning->mPPSMutex);
    return 0;
}


#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
#ifdef DMNR_TUNNING_AT_MODEMSIDE
static void *DMNR_Play_Rec_Routine(void *arg)
{
    SXLOGD("DMNR_Play_Rec_Routine in +");
    AudioParamTuning *pDMNRTuning = (AudioParamTuning *)arg;
    if (pDMNRTuning == NULL)
    {
        SXLOGE("DMNR_Play_Rec_Routine pDMNRTuning = NULL arg = %x", arg);
        return 0;
    }

    uint32 PCM_BUF_SIZE = pDMNRTuning->m_bWBMode ? 640 : 320;
    unsigned long sleepTime = (PLAYBUF_SIZE / PCM_BUF_SIZE) * 20 * 1000;

    pthread_mutex_lock(&pDMNRTuning->mDMNRMutex);

    // Adjust thread priority
    prctl(PR_SET_NAME, (unsigned long)"DualMicCalibrationRoutine", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);

    //Prepare file pointer
    FILE *pInFp = pDMNRTuning->m_pInputFile;      //file for input
    FILE *pOutFp = pDMNRTuning->m_pOutputFile;    //file for input

    // ----start the loop --------
    char *tmp = new char[PLAYBUF_SIZE];
    pDMNRTuning->m_bDMNRThreadExit = false;
    int cntR = 0;
    int cntW = 0;
    int numOfBytesPlay = 0;
    int numOfBytesRec = 0;

    int playBufFreeCnt = 0;
    int recBufDataCnt = 0;

    SXLOGD("pthread_cond_signal(&pDMNRTuning->mDMNRExit_Cond)");
    pthread_cond_signal(&pDMNRTuning->mDMNRExit_Cond); // wake all thread
    pthread_mutex_unlock(&pDMNRTuning->mDMNRMutex);

    while ((!pDMNRTuning->m_bDMNRThreadExit) && pOutFp)
    {
        //handling playback buffer
        pthread_mutex_lock(&pDMNRTuning->mPlayBufMutex);
        if (pInFp)
        {
            playBufFreeCnt = pDMNRTuning->mPlay2WayInstance->GetFreeBufferCount() - 1;
            cntR = fread(tmp, sizeof(char), playBufFreeCnt, pInFp);
            pDMNRTuning->mPlay2WayInstance->Write(tmp, cntR);
            numOfBytesPlay += cntR;
            SXLOGV(" Playback buffer, free:%d, read from :%d, total play:%d", playBufFreeCnt, cntR, numOfBytesPlay);
        }
        pthread_mutex_unlock(&pDMNRTuning->mPlayBufMutex);

        // handling record buffer
        pthread_mutex_lock(&pDMNRTuning->mRecBufMutex);
        recBufDataCnt = pDMNRTuning->mRec2WayInstance->GetBufferDataCount();
        pDMNRTuning->mRec2WayInstance->Read(tmp, recBufDataCnt);
        cntW = fwrite((void *)tmp, sizeof(char), recBufDataCnt, pOutFp);
        numOfBytesRec += cntW;
        SXLOGV(" Record buffer, available:%d, write to file:%d, total rec:%d", recBufDataCnt, cntW, numOfBytesRec);
        pthread_mutex_unlock(&pDMNRTuning->mRecBufMutex);

        usleep(sleepTime / 2);
    }

    // free buffer
    delete[] tmp;
    tmp = NULL;

    //exit thread
    SXLOGD("VmRecordRoutine pthread_mutex_lock");
    pthread_mutex_lock(&pDMNRTuning->mDMNRMutex);
    SXLOGD("pthread_cond_signal(&mDMNRExit_Cond)");
    pthread_cond_signal(&pDMNRTuning->mDMNRExit_Cond); // wake all thread
    pthread_mutex_unlock(&pDMNRTuning->mDMNRMutex);

    return 0;
}
#else
static int PCM_decode_data(WAVEHDR *wavHdr,  char *in_buf, int block_size, char *out_buf, int *out_size)
{
    int i, j;
    uint16_t *ptr_d;
    uint8_t  *ptr_s;
    int readlen = 0;
    int writelen = 0;

    uint16_t channels = wavHdr->NumChannels;
    uint16_t bits_per_sample = wavHdr->BitsPerSample;

    ptr_s = (uint8_t *)in_buf;
    ptr_d = (uint16_t *)out_buf;
    readlen = block_size;
    *out_size = 0;

    switch (bits_per_sample)
    {
        case 8:
            if (channels == 2)
            {
                for (i = 0; i < readlen; i++)
                {
                    *(ptr_d + j) = (uint16_t)(*(ptr_s + i) - 128) << 8;
                    j++;
                }
            }
            else
            {
                for (i = 0; i < readlen; i++)
                {
                    *(ptr_d + j) = (uint16_t)(*(ptr_s + i) - 128) << 8;
                    *(ptr_d + j + 1) =  *(ptr_d + j);
                    j += 2;
                }
            }
            writelen = (j << 1);
            break;
        case 16:
            if (channels == 2)
            {
                for (i = 0; i < readlen; i += 2)
                {
                    *(ptr_d + j) = *(ptr_s + i) + ((uint16_t)(*(ptr_s + i + 1)) << 8);
                    j++;
                }
            }
            else
            {
                for (i = 0; i < readlen; i += 2)
                {
                    *(ptr_d + j) = *(ptr_s + i) + ((uint16_t)(*(ptr_s + i + 1)) << 8);
                    *(ptr_d + j + 1) = *(ptr_d + j);
                    j += 2;
                }
            }
            writelen = (j << 1);
            break;
        default:
            ptr_d = (uint16_t *)(out_buf);
            break;
    }
    *out_size = writelen;
    return true;
}
static void PCM_Apply_DigitalDb(char *out_buf, int out_size, int table_index)
{
    short *pcmValue = (short *)out_buf;
    for (int i = 0; i < out_size / 2; i++)
    {
        *pcmValue = *pcmValue * (digitOnly_quater_dB_tableForSpeech[4 * table_index] / 4096.0);
        pcmValue ++;
    }
}
static void *DMNR_Play_Rec_ApSide_Routine(void *arg)
{
    SXLOGD("DMNR_Play_Rec_ApSide_Routine in +");
    AudioParamTuning *pDMNRTuning = (AudioParamTuning *)arg;
    if (pDMNRTuning == NULL)
    {
        SXLOGE("DMNR_Play_Rec_ApSide_Routine pDMNRTuning = NULL arg = %x", arg);
        return 0;
    }

    pthread_mutex_lock(&pDMNRTuning->mDMNRMutex);

    // Adjust thread priority
    prctl(PR_SET_NAME, (unsigned long)"DualMicCalibrationAtApSideRoutine", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);

    //Prepare file pointer
    FILE *pInFp = pDMNRTuning->m_pInputFile;      //file for input(use audiomtkstreamout to play)
    FILE *pOutFp = pDMNRTuning->m_pOutputFile;    //file for output(use audiomtkstreamin to record)

    // ----start the loop --------
    pDMNRTuning->m_bDMNRThreadExit = false;

    SXLOGD("pthread_cond_signal(&pDMNRTuning->mDMNRExit_Cond)");
    pthread_cond_signal(&pDMNRTuning->mDMNRExit_Cond); // wake all thread
    pthread_mutex_unlock(&pDMNRTuning->mDMNRMutex);

    android_audio_legacy::AudioStreamOut *streamOutput = NULL;
    android_audio_legacy::AudioStreamIn *streamInput = NULL;

    WAVEHDR waveHeader;
    memset(&waveHeader, 0, sizeof(WAVEHDR));

    char *inBuffer = NULL; //for playback
    char *outBuffer = NULL;
    uint32 readBlockLen;
    int playbackDb_index = 0;

    char readBuffer[1024] = {0};//for record

    if (pInFp) //open output stream for playback
    {
        //config output format channel= 2 , bits_per_sample=16
        FILE_FORMAT fileType = pDMNRTuning->playbackFileFormat();

        if (fileType == WAVE_FORMAT)
        {
            fread(&waveHeader, WAV_HEADER_SIZE, 1, pInFp);
        }
        else if (fileType == UNSUPPORT_FORMAT)
        {
            SXLOGW("[Dual-Mic] playback file format is not support");
            return 0;
        }
        uint32 sampleRate = waveHeader.SampleRate;
        uint32 channels = android_audio_legacy::AudioSystem::CHANNEL_OUT_STEREO;
        int format, status;

        if (waveHeader.BitsPerSample == 8 || waveHeader.BitsPerSample == 16)
        {
            format = android_audio_legacy::AudioSystem::PCM_16_BIT;
        }
        else
        {
            format = android_audio_legacy::AudioSystem::PCM_16_BIT;
        }

        //create output stream
        streamOutput = pDMNRTuning->getStreamManager()->openOutputStream(0, &format, &channels, &sampleRate, &status);

        uint32 hwBufferSize = streamOutput->bufferSize(); //16k bytes

        if (waveHeader.NumChannels == 1)
        {
            switch (waveHeader.BitsPerSample)
            {
                case 8:
                    readBlockLen = hwBufferSize >> 2;
                    break;
                case 16:
                    readBlockLen = hwBufferSize >> 1;
                    break;
                default:
                    readBlockLen = 0;
                    break;
            }
        }
        else
        {
            switch (waveHeader.BitsPerSample)
            {
                case 8:
                    readBlockLen = hwBufferSize >> 1;
                    break;
                case 16:
                    readBlockLen = hwBufferSize;
                    break;
                default:
                    readBlockLen = 0;
                    break;
            }
        }
        inBuffer = new char[readBlockLen];
        outBuffer = new char[hwBufferSize];
        playbackDb_index = pDMNRTuning->getPlaybackDb();
        SXLOGD("readBlockLen = %d, hwBufferSize = %d,playbackDb_index = %d \n", readBlockLen, hwBufferSize, playbackDb_index);
    }

    if (pOutFp) //open input stream for record
    {
#ifdef MTK_AUDIO_HD_REC_SUPPORT
        AUDIO_HD_RECORD_SCENE_TABLE_STRUCT hdRecordSceneTable;
        GetHdRecordSceneTableFromNV(&hdRecordSceneTable);
        if (hdRecordSceneTable.num_voice_rec_scenes > 0)  //value=0;
        {
            int32 HDRecScene = 1;//1:cts verifier offset
            pDMNRTuning->getSpeechEnhanceInfoInst()->SetHDRecScene(HDRecScene);
        }
#endif
        uint32_t device = AUDIO_DEVICE_IN_BUILTIN_MIC;
        int format = AUDIO_FORMAT_PCM_16_BIT;
        uint32_t channel = AUDIO_CHANNEL_IN_STEREO;
        uint32_t sampleRate = 16000;
        status_t status = 0;
        streamInput = pDMNRTuning->getStreamManager()->openInputStream(device, &format, &channel, &sampleRate, &status, (android_audio_legacy::AudioSystem::audio_in_acoustics)0);
        android::AudioParameter paramInputSource = android::AudioParameter();
        paramInputSource.addInt(android::String8(android::AudioParameter::keyInputSource), android_audio_legacy::AUDIO_SOURCE_MIC);
        streamInput->setParameters(paramInputSource.toString());

        android::AudioParameter paramDeviceIn = android::AudioParameter();
        paramDeviceIn.addInt(android::String8(android::AudioParameter::keyRouting), AUDIO_DEVICE_IN_BUILTIN_MIC);
        streamInput->setParameters(paramDeviceIn.toString());
    }

    while (!pDMNRTuning->m_bDMNRThreadExit)
    {
        //handling playback buffer
        pthread_mutex_lock(&pDMNRTuning->mPlayBufMutex);
        if (pInFp && !feof(pInFp))
        {
            int readdata = 0, writedata = 0, out_size = 0;
            memset(inBuffer, 0, sizeof(inBuffer));
            memset(outBuffer, 0, sizeof(outBuffer));
            readdata = fread(inBuffer, readBlockLen, 1, pInFp);
            PCM_decode_data(&waveHeader, inBuffer, readBlockLen, outBuffer, &out_size);
            PCM_Apply_DigitalDb(outBuffer, out_size, playbackDb_index);
            writedata = streamOutput->write(outBuffer, out_size);
#if 0
            char filename[] = "/sdcard/xxx.pcm";
            FILE *fp = fopen(filename, "ab+");
            fwrite(outBuffer, writedata, 1, fp);
            fclose(fp);
#endif
        }
        pthread_mutex_unlock(&pDMNRTuning->mPlayBufMutex);

        // handling record buffer
        pthread_mutex_lock(&pDMNRTuning->mRecBufMutex);
        if (pOutFp)
        {
            memset(readBuffer, 0, sizeof(readBuffer));
            int nRead = streamInput->read(readBuffer, 1024);
#ifdef WRITE_RECORDDATA_ON_APSIDEDMNR
            fwrite(readBuffer, 1, nRead, pOutFp);
#endif
        }
        pthread_mutex_unlock(&pDMNRTuning->mRecBufMutex);
    }

    if (pInFp)
    {
        streamOutput->standby();
        pDMNRTuning->getStreamManager()->closeOutputStream(streamOutput);
        if (inBuffer)
        {
            delete[] inBuffer;
            inBuffer = NULL;
        }
        if (outBuffer)
        {
            delete[] outBuffer;
            outBuffer = NULL;
        }
    }

    if (pOutFp)
    {
        streamInput->standby();
        pDMNRTuning->getStreamManager()->closeInputStream(streamInput);
    }

    //exit thread
    pthread_mutex_lock(&pDMNRTuning->mDMNRMutex);
    SXLOGD("pthread_cond_signal(&mDMNRExit_Cond)");
    pthread_cond_signal(&pDMNRTuning->mDMNRExit_Cond); // wake all thread
    pthread_mutex_unlock(&pDMNRTuning->mDMNRMutex);

    return 0;
}
#endif
#endif

AudioParamTuning *AudioParamTuning::UniqueTuningInstance = 0;

AudioParamTuning *AudioParamTuning::getInstance()
{
    if (UniqueTuningInstance == 0)
    {
        SXLOGD("create AudioParamTuning instance --");
        UniqueTuningInstance = new AudioParamTuning();
        SXLOGD("create AudioParamTuning instance ++");
    }

    return UniqueTuningInstance;
}

AudioParamTuning::AudioParamTuning() :
    mMode(0),
    mSideTone(0xFFFFFF40),
    m_bPlaying(false),
    m_bWBMode(false),
    m_bPPSThreadExit(false),
    m_pInputFile(NULL)
{
    SXLOGD("AudioParamTuning in +");
    // create volume instance
    mAudioVolumeInstance = AudioVolumeFactory::CreateAudioVolumeController();
    mAudioVolumeInstance->initCheck();

    // create digital control instnace
    mAudioDigitalInstance  = AudioDigitalControlFactory::CreateAudioDigitalControl();

    // create analog control instnace
    mAudioAnalogInstance  = AudioAnalogControlFactory::CreateAudioAnalogControl();

    // create audio resource manager instance
    mAudioResourceManager = AudioResourceManager::getInstance();

    // create speech driver instance
    mSpeechDriverFactory = SpeechDriverFactory::GetInstance();

    mSphPhonecallCtrl = SpeechPhoneCallController::GetInstance();

    memset(mOutputVolume, 0, MODE_NUM * sizeof(uint32));
    memset(m_strInputFileName, 0, FILE_NAME_LEN_MAX * sizeof(char));

    int ret = pthread_mutex_init(&mP2WMutex, NULL);
    if (ret != 0)
    {
        SXLOGE("Failed to initialize pthread mP2WMutex!");
    }

    ret = pthread_mutex_init(&mPPSMutex, NULL);
    if (ret != 0)
    {
        SXLOGE("Failed to initialize mPPSMutex!");
    }

    ret = pthread_mutex_init(&mPlayBufMutex, NULL);
    if (ret != 0)
    {
        SXLOGE("Failed to initialize mPlayBufMutex!");
    }

    ret = pthread_cond_init(&mPPSExit_Cond, NULL);
    if (ret != 0)
    {
        SXLOGE("Failed to initialize mPPSExit_Cond!");
    }

#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
    m_bDMNRPlaying = false;
    m_bDMNRThreadExit = false;
    m_pOutputFile = NULL;

    mPlay2WayInstance = 0;
    mRec2WayInstance = 0;

    memset(m_strOutFileName, 0, FILE_NAME_LEN_MAX * sizeof(char));

    AUDIO_VER1_CUSTOM_VOLUME_STRUCT VolumeCustomParam;//volume custom data
    GetVolumeVer1ParamFromNV(&VolumeCustomParam);

    mDualMicTool_micGain[0] = VolumeCustomParam.audiovolume_mic[VOLUME_NORMAL_MODE][3];
    if (mDualMicTool_micGain[0] > UPLINK_GAIN_MAX)
    {
        mDualMicTool_micGain[0] = UPLINK_GAIN_MAX;
    }

    mDualMicTool_micGain[1] = VolumeCustomParam.audiovolume_mic[VOLUME_SPEAKER_MODE][3];
    if (mDualMicTool_micGain[1] > UPLINK_GAIN_MAX)
    {
        mDualMicTool_micGain[1] = UPLINK_GAIN_MAX;
    }

    mDualMicTool_receiverGain = VolumeCustomParam.audiovolume_sph[VOLUME_NORMAL_MODE][CUSTOM_VOLUME_STEP - 1];
    if (mDualMicTool_receiverGain > MAX_VOICE_VOLUME)
    {
        mDualMicTool_receiverGain = MAX_VOICE_VOLUME;
    }

    mDualMicTool_headsetGain = VolumeCustomParam.audiovolume_sph[VOLUME_HEADSET_MODE][3];
    if (mDualMicTool_headsetGain > MAX_VOICE_VOLUME)
    {
        mDualMicTool_headsetGain = MAX_VOICE_VOLUME;
    }
#ifndef DMNR_TUNNING_AT_MODEMSIDE
    mAudioMtkStreamManager = AudioMTKStreamManager::getInstance();
    mAudioSpeechEnhanceInfoInstance = AudioSpeechEnhanceInfo::getInstance();
    mPlaybackDb_index = 0;
#endif
    ret = pthread_mutex_init(&mDMNRMutex, NULL);
    if (ret != 0)
    {
        SXLOGE("Failed to initialize mDMNRMutex!");
    }

    ret = pthread_mutex_init(&mRecBufMutex, NULL);
    if (ret != 0)
    {
        SXLOGE("Failed to initialize mRecBufMutex!");
    }

    ret = pthread_cond_init(&mDMNRExit_Cond, NULL);
    if (ret != 0)
    {
        SXLOGE("Failed to initialize mDMNRExit_Cond!");
    }

    SXLOGD("AudioYusuParamTuning: default mic gain-mormal:%d;handsfree:%d, receiver gain:%d, headset Gain:%d", mDualMicTool_micGain[0], mDualMicTool_micGain[1], mDualMicTool_receiverGain, mDualMicTool_headsetGain);
#endif

}

AudioParamTuning::~AudioParamTuning()
{
    SXLOGD("~AudioParamTuning in +");
}

//for taste tool
bool AudioParamTuning::isPlaying()
{
    SXLOGV("isPlaying - playing:%d", m_bPlaying);
    bool ret = false;
    pthread_mutex_lock(&mP2WMutex);
#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
    SXLOGV("isPlaying - DMNR playing:%d", m_bDMNRPlaying);
    ret = (m_bPlaying | m_bDMNRPlaying) ? true : false;
#else
    ret = m_bPlaying;
#endif
    pthread_mutex_unlock(&mP2WMutex);
    return ret;
}

uint32 AudioParamTuning::getMode()
{
    SXLOGD("getMode - mode:%d", mMode);
    pthread_mutex_lock(&mP2WMutex);
    uint32 ret = mMode;
    pthread_mutex_unlock(&mP2WMutex);
    return ret;
}

status_t AudioParamTuning::setMode(uint32 mode)
{
    SXLOGD("setMode - mode:%d", mode);
    pthread_mutex_lock(&mP2WMutex);
    mMode = mode;
    pthread_mutex_unlock(&mP2WMutex);
    return NO_ERROR;
}

status_t AudioParamTuning::setPlaybackFileName(const char *fileName)
{
    SXLOGD("setPlaybackFileName in +");
    pthread_mutex_lock(&mP2WMutex);
    if (fileName != NULL && strlen(fileName) < FILE_NAME_LEN_MAX - 1)
    {
        SXLOGD("input file name:%s", fileName);
        memset(m_strInputFileName, 0, FILE_NAME_LEN_MAX);
        strcpy(m_strInputFileName, fileName);
    }
    else
    {
        SXLOGE("input file name NULL or too long!");
        pthread_mutex_unlock(&mP2WMutex);
        return BAD_VALUE;
    }
    pthread_mutex_unlock(&mP2WMutex);
    return NO_ERROR;
}

status_t AudioParamTuning::setDLPGA(uint32 gain)
{
    SXLOGD("setDLPGA in +");
    uint32 outputDev = 0;

    if (gain > MAX_VOICE_VOLUME)
    {
        SXLOGE("setDLPGA gain error  gain=%x", gain);
        return BAD_VALUE;
    }

    pthread_mutex_lock(&mP2WMutex);
    mOutputVolume[mMode] = gain;
    SXLOGD("setDLPGA mode=%d, gain=%d, lad volume=0x%x", mMode, gain, mOutputVolume[mMode]);

    if (m_bPlaying)
    {
        SXLOGD("setDLPGA lad_Volume=%x", mOutputVolume[mMode]);
        setSphVolume(mMode, mOutputVolume[mMode]);
    }

    pthread_mutex_unlock(&mP2WMutex);
    return NO_ERROR;
}

void AudioParamTuning::updataOutputFIRCoffes(AudioTasteTuningStruct *pCustParam)
{
    SXLOGD("updataOutputFIRCoffes in +");

    int ret = 0;
    unsigned short mode = pCustParam->phone_mode;
    unsigned short cmdType = pCustParam->cmd_type;

    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();

    pthread_mutex_lock(&mP2WMutex);

    if (m_bPlaying && mode == mMode)
    {
        pSpeechDriver->PCM2WayOff(); // trun off PCM2Way
        mAudioResourceManager->StopInputDevice();
        mAudioResourceManager->StopOutputDevice();
        usleep(10 * 1000); //wait to make sure all message is processed
    }

    if (pCustParam->wb_mode)
    {
#if defined(MTK_WB_SPEECH_SUPPORT)
        AUDIO_CUSTOM_WB_PARAM_STRUCT sCustWbParam;
        GetWBSpeechParamFromNVRam(&sCustWbParam);
        if (cmdType && sCustWbParam.speech_mode_wb_para[mode][7] != pCustParam->dlDigitalGain)
        {
            SXLOGD("updataOutputFIRCoffes mode=%d, ori dlDG gain=%d, new dlDG gain=%d", mode, sCustWbParam.speech_mode_wb_para[mode][7], pCustParam->dlDigitalGain);
            sCustWbParam.speech_mode_wb_para[mode][7] = pCustParam->dlDigitalGain;
        }
        ret = pSpeechDriver->SetWBSpeechParameters(&sCustWbParam);
#endif
    }
    else
    {
        AUDIO_CUSTOM_PARAM_STRUCT sCustParam;
        AUDIO_PARAM_MED_STRUCT sCustMedParam;
        unsigned short index = pCustParam->slected_fir_index;
        unsigned short dlGain = pCustParam->dlDigitalGain;
        GetNBSpeechParamFromNVRam(&sCustParam);
        GetMedParamFromNV(&sCustMedParam);

        if ((cmdType == (unsigned short)AUD_TASTE_START || cmdType == (unsigned short)AUD_TASTE_INDEX_SETTING) && sCustMedParam.select_FIR_output_index[mode] != index)
        {
            SXLOGD("updataOutputFIRCoffes mode=%d, old index=%d, new index=%d", mode, sCustMedParam.select_FIR_output_index[mode], index);
            //save  index to MED with different mode.
            sCustMedParam.select_FIR_output_index[mode] = index;

            SXLOGD("updataOutputFIRCoffes ori sph_out_fir[%d][0]=%d, ori sph_out_fir[%d][44]", mode, sCustParam.sph_out_fir[mode][0], mode, sCustParam.sph_out_fir[mode][44]);
            //copy med data into audio_custom param
            memcpy((void *)sCustParam.sph_out_fir[mode], (void *)sCustMedParam.speech_output_FIR_coeffs[mode][index], sizeof(sCustParam.sph_out_fir[index]));
            SXLOGD("updataOutputFIRCoffes new sph_out_fir[%d][0]=%d, new sph_out_fir[%d][44]", mode, sCustParam.sph_out_fir[mode][0], mode, sCustParam.sph_out_fir[mode][44]);
            SetNBSpeechParamToNVRam(&sCustParam);
            SetMedParamToNV(&sCustMedParam);
        }

        if ((cmdType == (unsigned short)AUD_TASTE_START || cmdType == (unsigned short)AUD_TASTE_DLDG_SETTING) && sCustParam.speech_mode_para[mode][7] != dlGain)
        {
            SXLOGD("updataOutputFIRCoffes mode=%d, old dlDGGain=%d, new dlDGGain=%d", mode, sCustParam.speech_mode_para[mode][7], dlGain);
            sCustParam.speech_mode_para[mode][7] = dlGain;
        }
        SXLOGD("updataOutputFIRCoffes  sph_out_fir[%d][0]=%d, sph_out_fir[%d][44]", mode, sCustParam.sph_out_fir[mode][0], mode, sCustParam.sph_out_fir[mode][44]);
        ret = pSpeechDriver->SetNBSpeechParameters(&sCustParam);
    }

    if (m_bPlaying && mode == mMode)
    {
        mAudioResourceManager->setUlInputDevice(AUDIO_DEVICE_IN_BUILTIN_MIC);
        mAudioResourceManager->StartInputDevice();

        switch (mMode)
        {
            case SPEECH_MODE_NORMAL:
            {
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_EARPIECE);
                mAudioResourceManager->StartOutputDevice();
                mAudioVolumeInstance->ApplySideTone(EarPiece_SideTone_Gain); // in 0.5dB
                pSpeechDriver->SetSpeechMode(AUDIO_DEVICE_IN_BUILTIN_MIC, AUDIO_DEVICE_OUT_EARPIECE);
                break;
            }
            case SPEECH_MODE_EARPHONE:
            {
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET);
                mAudioResourceManager->StartOutputDevice();
                mAudioVolumeInstance->ApplySideTone(Headset_SideTone_Gain);
                pSpeechDriver->SetSpeechMode(AUDIO_DEVICE_IN_BUILTIN_MIC, AUDIO_DEVICE_OUT_WIRED_HEADSET);
                break;
            }
            case SPEECH_MODE_LOUD_SPEAKER:
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_SPEAKER);
                mAudioResourceManager->StartOutputDevice();
                mAudioVolumeInstance->ApplySideTone(LoudSpk_SideTone_Gain);
                pSpeechDriver->SetSpeechMode(AUDIO_DEVICE_IN_BUILTIN_MIC, AUDIO_DEVICE_OUT_SPEAKER);
                break;
            default:
                break;
        }
        setSphVolume(mMode, mOutputVolume[mMode]);

        sph_enh_mask_struct_t sphMask;
        sphMask.main_func = SPH_ENH_MAIN_MASK_ALL;
        sphMask.dynamic_func = SPH_ENH_DYNAMIC_MASK_VCE;
        pSpeechDriver->SetSpeechEnhancementMask(sphMask);
        pSpeechDriver->PCM2WayOn(m_bWBMode); // start PCM 2 way
        pSpeechDriver->SetSpeechEnhancement(true);
    }
    pthread_mutex_unlock(&mP2WMutex);
}

status_t AudioParamTuning::enableModemPlaybackVIASPHPROC(bool bEnable, bool bWB)//need record path?
{

    SXLOGD("enableModemPlaybackVIASPHPROC bEnable:%d, bWBMode:%d", bEnable, bWB);
    int ret = 0;

    // 3 sec for creat thread timeout
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + 3;
    timeout.tv_nsec = now.tv_usec * 1000;

    // get speech driver interface
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();

    if (mRec2WayInstance == 0)
    {
        mRec2WayInstance = Record2Way::GetInstance();
    }
    if (mPlay2WayInstance == 0)
    {
        mPlay2WayInstance = Play2Way::GetInstance();
    }

    if (bEnable && (isPlaying() == false))
    {
        pthread_mutex_lock(&mP2WMutex);
        m_pInputFile = fopen(m_strInputFileName, "rb");
        if (m_pInputFile == NULL)
        {
            m_pInputFile = fopen("/mnt/sdcard2/test.wav", "rb");
            if (m_pInputFile == NULL)
            {
                SXLOGD("open input file fail!!");
                pthread_mutex_unlock(&mP2WMutex);
                return BAD_VALUE;
            }
        }
        m_bWBMode = bWB;
        // Enable the audio power - afe/adc/analog
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ADC, true);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true);

        mAudioResourceManager->setUlInputDevice(AUDIO_DEVICE_IN_BUILTIN_MIC);
        // pretend to be phone call state
        mAudioResourceManager->SetAudioMode((mSpeechDriverFactory->GetActiveModemIndex() == MODEM_1) ? AUDIO_MODE_IN_CALL : ((mSpeechDriverFactory->GetActiveModemIndex() == MODEM_2) ? AUDIO_MODE_IN_CALL_2 : AUDIO_MODE_IN_CALL_EXTERNAL));
        switch (mMode)
        {
            case SPEECH_MODE_NORMAL:
            {
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_EARPIECE);
                mAudioVolumeInstance->ApplySideTone(EarPiece_SideTone_Gain);// in 0.5dB
                pSpeechDriver->SetSpeechMode(AUDIO_DEVICE_IN_BUILTIN_MIC, AUDIO_DEVICE_OUT_EARPIECE);
                break;
            }
            case SPEECH_MODE_EARPHONE:
            {
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET);
                mAudioVolumeInstance->ApplySideTone(Headset_SideTone_Gain);
                pSpeechDriver->SetSpeechMode(AUDIO_DEVICE_IN_WIRED_HEADSET, AUDIO_DEVICE_OUT_WIRED_HEADSET);
                break;
            }
            case SPEECH_MODE_LOUD_SPEAKER:
                mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_SPEAKER);
                mAudioVolumeInstance->ApplySideTone(LoudSpk_SideTone_Gain);
                pSpeechDriver->SetSpeechMode(AUDIO_DEVICE_IN_BUILTIN_MIC, AUDIO_DEVICE_OUT_SPEAKER);
                break;
            default:
                break;
        }

        mSphPhonecallCtrl->OpenModemSpeechControlFlow(mAudioResourceManager->GetAudioMode());

        setSphVolume(mMode, mOutputVolume[mMode]);
        // start pcm2way
        mRec2WayInstance->Start();
        mPlay2WayInstance->Start();

        SXLOGD("open taste_threadloop thread~");
        pthread_mutex_lock(&mPPSMutex);
        ret = pthread_create(&mTasteThreadID, NULL, Play_PCM_With_SpeechEnhance_Routine, (void *)this);
        if (ret != 0)
        {
            SXLOGE("Play_PCM_With_SpeechEnhance_Routine thread pthread_create error!!");
            pthread_mutex_unlock(&mP2WMutex);
            return UNKNOWN_ERROR;
        }

        SXLOGD("+mPPSExit_Cond wait");
        ret = pthread_cond_timedwait(&mPPSExit_Cond, &mPPSMutex, &timeout);
        SXLOGD("-mPPSExit_Cond receive ret=%d", ret);
        pthread_mutex_unlock(&mPPSMutex);
        usleep(100 * 1000);

        m_bPlaying = true;
        sph_enh_mask_struct_t sphMask;
        sphMask.main_func = SPH_ENH_MAIN_MASK_ALL;
        sphMask.dynamic_func = SPH_ENH_DYNAMIC_MASK_VCE;
        pSpeechDriver->SetSpeechEnhancementMask(sphMask);
        pSpeechDriver->PCM2WayOn(m_bWBMode); // start PCM 2 way
        pSpeechDriver->SetSpeechEnhancement(true);
        pthread_mutex_unlock(&mP2WMutex);
    }
    else if ((!bEnable) && m_bPlaying)
    {
        pthread_mutex_lock(&mP2WMutex);
        pthread_mutex_lock(&mPPSMutex);
        if (!m_bPPSThreadExit)
        {
            m_bPPSThreadExit = true;
            SXLOGD("+mPPSExit_Cond wait");
            ret = pthread_cond_timedwait(&mPPSExit_Cond, &mPPSMutex, &timeout);
            SXLOGD("-mPPSExit_Cond receive ret=%d", ret);
        }
        pthread_mutex_unlock(&mPPSMutex);

        mAudioResourceManager->StopOutputDevice();
        mAudioResourceManager->StopInputDevice();

        pSpeechDriver->PCM2WayOff();

        mRec2WayInstance->Stop();
        mPlay2WayInstance->Stop();
        usleep(200 * 1000); //wait to make sure all message is processed

        m_bPlaying = false;
        // Disable the audio power - afe/adc/analog
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ADC, false);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);

        mSphPhonecallCtrl->CloseModemSpeechControlFlow(mAudioResourceManager->GetAudioMode());
        mAudioResourceManager->SetAudioMode(AUDIO_MODE_NORMAL);
        if (m_pInputFile) { fclose(m_pInputFile); }
        m_pInputFile = NULL;

        pthread_mutex_unlock(&mP2WMutex);
    }
    else
    {
        SXLOGD("The Audio Taste Tool State is error, bEnable=%d, playing=%d", bEnable, m_bPlaying);
        return BAD_VALUE;
    }

    return NO_ERROR;
}

FILE_FORMAT AudioParamTuning::playbackFileFormat()
{
    SXLOGD("playbackFileFormat - playback file name:%s", m_strInputFileName);
    FILE_FORMAT ret = UNSUPPORT_FORMAT;
    char *pFileSuffix = m_strInputFileName;

    strsep(&pFileSuffix, ".");
    if (pFileSuffix != NULL)
    {
        if (strcmp(pFileSuffix, "pcm") == 0 || strcmp(pFileSuffix, "PCM") == 0)
        {
            SXLOGD("playbackFileFormat - playback file format is pcm");
            ret = PCM_FORMAT;
        }
        else if (strcmp(pFileSuffix, "wav") == 0 || strcmp(pFileSuffix, "WAV") == 0)
        {
            SXLOGD("playbackFileFormat - playback file format is wav");
            ret = WAVE_FORMAT;
        }
        else
        {
            SXLOGD("playbackFileFormat - playback file format is unsupport");
            ret = UNSUPPORT_FORMAT;
        }
    }

    return ret;
}

#if defined(MTK_DUAL_MIC_SUPPORT) || defined(MTK_AUDIO_HD_REC_SUPPORT)
// For DMNR Tuning
status_t AudioParamTuning::setRecordFileName(const char *fileName)
{
    SXLOGD("setRecordFileName in+");
    pthread_mutex_lock(&mP2WMutex);
    if (fileName != NULL && strlen(fileName) < FILE_NAME_LEN_MAX - 1)
    {
        SXLOGD("input file name:%s", fileName);
        memset(m_strOutFileName, 0, FILE_NAME_LEN_MAX);
        strcpy(m_strOutFileName, fileName);
    }
    else
    {
        SXLOGE("input file name NULL or too long!");
        pthread_mutex_unlock(&mP2WMutex);
        return BAD_VALUE;
    }

    pthread_mutex_unlock(&mP2WMutex);
    return NO_ERROR;
}

status_t AudioParamTuning::setDMNRGain(unsigned short type, unsigned short value)
{
    SXLOGD("setDMNRGain: type=%d, gain=%d", type, value);
    status_t ret = NO_ERROR;

    if (value < 0)
    {
        return BAD_VALUE;
    }

    pthread_mutex_lock(&mP2WMutex);
    switch (type)
    {
        case AUD_MIC_GAIN:
            mDualMicTool_micGain[0] = (value > UPLINK_GAIN_MAX) ? UPLINK_GAIN_MAX : value;
            break;
        case AUD_RECEIVER_GAIN:
            mDualMicTool_receiverGain = (value > MAX_VOICE_VOLUME) ? MAX_VOICE_VOLUME : value;
            break;
        case AUD_HS_GAIN:
            mDualMicTool_headsetGain = (value > MAX_VOICE_VOLUME) ? MAX_VOICE_VOLUME : value;
            break;
        case AUD_MIC_GAIN_HF:
            mDualMicTool_micGain[1] = (value > UPLINK_GAIN_MAX) ? UPLINK_GAIN_MAX : value;
            break;
        default:
            SXLOGW("setDMNRGain unknown type");
            ret = BAD_VALUE;
            break;
    }
    pthread_mutex_unlock(&mP2WMutex);
    return ret;
}

status_t AudioParamTuning::getDMNRGain(unsigned short type, unsigned short *value)
{
    SXLOGD("getDMNRGain: type=%d", type);
    status_t ret = NO_ERROR;

    pthread_mutex_lock(&mP2WMutex);
    switch (type)
    {
        case AUD_MIC_GAIN:
            *value = mDualMicTool_micGain[0]; // normal mic
            break;
        case AUD_RECEIVER_GAIN:
            *value = mDualMicTool_receiverGain;
            break;
        case AUD_HS_GAIN:
            *value = mDualMicTool_headsetGain;
            break;
        case AUD_MIC_GAIN_HF:
            *value = mDualMicTool_micGain[1]; //handsfree mic
            break;
        default:
            SXLOGW("getDMNRGain unknown type");
            ret = BAD_VALUE;
            break;
    }
    pthread_mutex_unlock(&mP2WMutex);
    return ret;
}

#ifdef DMNR_TUNNING_AT_MODEMSIDE
status_t AudioParamTuning::enableDMNRModem2Way(bool bEnable, bool bWBMode, unsigned short outputDevice, unsigned short workMode)
{
    SXLOGD("enableDMNRModem2Way bEnable:%d, wb mode:%d, work mode:%d", bEnable, bWBMode, workMode);

    // 3 sec for timeout
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + 3;
    timeout.tv_nsec = now.tv_usec * 1000;
    int ret;

    // get speech driver interface
    SpeechDriverInterface *pSpeechDriver = mSpeechDriverFactory->GetSpeechDriver();

    if (mRec2WayInstance == 0)
    {
        mRec2WayInstance = Record2Way::GetInstance();
    }
    if ((!workMode) && (mPlay2WayInstance == 0))
    {
        mPlay2WayInstance = Play2Way::GetInstance();
    }

    if (bEnable && (isPlaying() == false))
    {
        pthread_mutex_lock(&mP2WMutex);
        // open output file
        if (!workMode)
        {
            m_pInputFile = fopen(m_strInputFileName, "rb");
            SXLOGD("[Dual-Mic] open input file filename:%s", m_strInputFileName);
            if (m_pInputFile == NULL)
            {
                SXLOGW("[Dual-Mic] open input file fail!!");
                pthread_mutex_unlock(&mP2WMutex);
                return BAD_VALUE;
            }
            FILE_FORMAT fileType = playbackFileFormat();
            char waveHeader[WAV_HEADER_SIZE];
            if (fileType == WAVE_FORMAT)
            {
                fread(waveHeader, sizeof(char), WAV_HEADER_SIZE, m_pInputFile);
            }
            else if (fileType == UNSUPPORT_FORMAT)
            {
                SXLOGW("[Dual-Mic] playback file format is not support");
                pthread_mutex_unlock(&mP2WMutex);
                return BAD_VALUE;
            }
        }

        m_pOutputFile = fopen(m_strOutFileName, "wb");
        SXLOGD("[Dual-Mic] open output file filename:%s", m_strOutFileName);
        if (m_pOutputFile == NULL)
        {
            SXLOGW("[Dual-Mic] open output file fail!!");
            fclose(m_pInputFile);
            pthread_mutex_unlock(&mP2WMutex);
            return BAD_VALUE;
        }

        // do basic setting to modem side
        m_bWBMode = bWBMode;
        // Enable the audio power - afe/adc/analog
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ADC, true);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true);

        //set input devices
        mAudioResourceManager->setUlInputDevice(AUDIO_DEVICE_IN_BUILTIN_MIC);

        //set output device
        if ((!workMode) && (outputDevice == P2W_RECEIVER_OUT))
        {
            mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_EARPIECE);
        }
        else if (!workMode)
        {
            mAudioResourceManager->setDlOutputDevice(AUDIO_DEVICE_OUT_WIRED_HEADSET);
        }

        // connect modem and PCM interface
        openModemDualMicCtlFlow(bWBMode, (const bool)workMode);

        // start pcm2way
        mRec2WayInstance->Start();
        if (!workMode)
        {
            mPlay2WayInstance->Start();
        }

        // open buffer thread
        SXLOGD("open DMNR_Tuning_threadloop thread~");
        pthread_mutex_lock(&mDMNRMutex);
        ret = pthread_create(&mDMNRThreadID, NULL, DMNR_Play_Rec_Routine, (void *)this);
        if (ret != 0)
        {
            SXLOGE("DMNR_threadloop pthread_create error!!");
        }

        SXLOGD("+mDMNRExit_Cond wait");
        ret = pthread_cond_timedwait(&mDMNRExit_Cond, &mDMNRMutex, &timeout);
        SXLOGD("-mDMNRExit_Cond receive ret=%d", ret);
        pthread_mutex_unlock(&mDMNRMutex);

        m_bDMNRPlaying = true;
        usleep(10 * 1000);

        // really enable the process
        pSpeechDriver->DualMicPCM2WayOn(bWBMode, (const bool)workMode);

        // start input device
        if (IsAudioSupportFeature(AUDIO_SUPPORT_DMIC))
        {

            mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_IN_DIGITAL_MIC, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        }
        else
        {
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::MUX_IN_PREAMP_L);
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_ADC2, AudioAnalogType::MUX_IN_PREAMP_R);

#if defined(MTK_DUAL_MIC_SUPPORT)  // base on dual or single mic select mic input source.
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_L, AudioAnalogType::MUX_IN_MIC1);
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_R, AudioAnalogType::MUX_IN_MIC3);
#else
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_L, AudioAnalogType::MUX_IN_MIC1);
            mAudioAnalogInstance->AnalogSetMux(AudioAnalogType::DEVICE_IN_PREAMP_R, AudioAnalogType::MUX_IN_MIC1);
#endif
            mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        }
        // set input device gain
        uint16_t swAGCGain = mAudioVolumeInstance->MappingToDigitalGain(mDualMicTool_micGain[0]);
        uint16_t degradedBGain = mAudioVolumeInstance->MappingToPGAGain(mDualMicTool_micGain[0]);
        SXLOGD("ApplyMicGain DegradedBGain:%d, swAGCGain:%d", degradedBGain, swAGCGain);

        mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPL, degradedBGain);
        mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_MICAMPR, degradedBGain);
        mAudioVolumeInstance->ApplyMdUlGain(swAGCGain);

        //start output device and set output gain
        if ((!workMode) && (outputDevice == P2W_RECEIVER_OUT))
        {
            mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_OUT_EARPIECER, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
            setSphVolume(VOLUME_NORMAL_MODE, mDualMicTool_receiverGain);
            SXLOGD("Play+Rec set dual mic, receiver gain: %d", mDualMicTool_receiverGain);
        }
        else if (!workMode)
        {
            mAudioAnalogInstance->AnalogOpen(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
            setSphVolume(VOLUME_HEADSET_MODE, mDualMicTool_headsetGain);
            SXLOGD("Play+Rec set dual mic, headset gain: %d", mDualMicTool_headsetGain);
        }

        pthread_mutex_unlock(&mP2WMutex);
    }
    else if (!bEnable && m_bDMNRPlaying)
    {
        pthread_mutex_lock(&mP2WMutex);
        //stop buffer thread
        SXLOGD("close DMNR_tuning_threadloop");
        pthread_mutex_lock(&mDMNRMutex);
        if (!m_bDMNRThreadExit)
        {
            m_bDMNRThreadExit = true;
            SXLOGD("+mDMNRExit_Cond wait");
            ret = pthread_cond_timedwait(&mDMNRExit_Cond, &mDMNRMutex, &timeout);
            SXLOGD("-mDMNRExit_Cond receive ret=%d", ret);
        }
        pthread_mutex_unlock(&mDMNRMutex);

        if (!workMode)
        {
            // Stop output device
            uint32_t output_device = mAudioResourceManager->getDlOutputDevice();
            if (output_device == AUDIO_DEVICE_OUT_EARPIECE)
            {
                mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_OUT_EARPIECER, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
            }
            else if (output_device == AUDIO_DEVICE_OUT_WIRED_HEADSET)
            {
                mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
            }
        }
        // stop input device
        if (IsAudioSupportFeature(AUDIO_SUPPORT_DMIC))
        {
            mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_IN_DIGITAL_MIC, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        }
        else
        {
            mAudioAnalogInstance->AnalogClose(AudioAnalogType::DEVICE_IN_ADC1, AudioAnalogType::DEVICE_PLATFORM_MACHINE);
        }


        pSpeechDriver->DualMicPCM2WayOff();

        mRec2WayInstance->Stop();
        if (!workMode)
        {
            mPlay2WayInstance->Stop();
        }
        //wait to make sure all message is processed
        usleep(200 * 1000);

        m_bDMNRPlaying = false;
        // Enable the audio power - afe/adc/analog
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ADC, false);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);

        closeModemDualMicCtlFlow((const bool)workMode);

        if (m_pInputFile) { fclose(m_pInputFile); }
        if (m_pOutputFile) { fclose(m_pOutputFile); }
        m_pInputFile = NULL;
        m_pOutputFile = NULL;
        pthread_mutex_unlock(&mP2WMutex);
    }
    else
    {
        SXLOGD("The DMNR Tuning State is error, bEnable=%d, playing=%d", bEnable, m_bPlaying);
        return BAD_VALUE;
    }

    return NO_ERROR;
}
#else
status_t AudioParamTuning::setPlaybackVolume(uint32 mode, uint32 gain)
{
    SXLOGV("setPlaybackVolume in +");
    int32 degradeDb = (DEVICE_VOLUME_STEP - gain) / VOICE_ONEDB_STEP;
    int voiceAnalogRange = DEVICE_MAX_VOLUME - DEVICE_MIN_VOLUME;
    ALOGD("gain:%ld,degradeDb:%ld,voiceAnalogRange:%ld", gain, degradeDb, voiceAnalogRange);
    switch (mode)
    {
        case VOLUME_NORMAL_MODE:
            if (gain <= AUDIO_BUFFER_HW_GAIN_STEP)
            {
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTL, gain);
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTR, gain);
                mPlaybackDb_index = 0;//digitOnly_quater_dB_tableForSpeech index
            }
            else
            {
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTL, voiceAnalogRange);
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTR, voiceAnalogRange);
                degradeDb -= voiceAnalogRange;
                mPlaybackDb_index = degradeDb;
            }
            break;
        case VOLUME_HEADSET_MODE:
            if (gain <= AUDIO_BUFFER_HW_GAIN_STEP)
            {
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, gain);
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, gain);
                mPlaybackDb_index = 0;
            }
            else
            {
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, voiceAnalogRange);
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, voiceAnalogRange);
                degradeDb -= voiceAnalogRange;
                mPlaybackDb_index = degradeDb;
            }
            break;
        case VOLUME_SPEAKER_MODE:
            if (gain <= AUDIO_BUFFER_HW_GAIN_STEP)
            {
#ifdef USING_EXTAMP_HP
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, gain);
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, gain);
#else
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_SPKL, gain);
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_SPKR, gain);
#endif
                mPlaybackDb_index = 0;
            }
            else
            {
                voiceAnalogRange = DEVICE_AMP_MAX_VOLUME - DEVICE_AMP_MIN_VOLUME;
#ifdef USING_EXTAMP_HP
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, voiceAnalogRange);
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, voiceAnalogRange);
#else
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_SPKL, voiceAnalogRange);
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_SPKR, voiceAnalogRange);
#endif
                degradeDb -= voiceAnalogRange;
                mPlaybackDb_index = degradeDb;
            }
            break;
        case VOLUME_HEADSET_SPEAKER_MODE:
            // nothing to do
            break;
        default:
            break;
    }
    SXLOGV("setPlaybackVolume in -");
    return NO_ERROR;
}

status_t AudioParamTuning::enableDMNRAtApSide(bool bEnable, bool bWBMode, unsigned short outputDevice, unsigned short workMode)
{
    SXLOGD("enableDMNRAtApSide bEnable:%d, wb mode:%d, outputDevice:%d,work mode:%d", bEnable, bWBMode, outputDevice, workMode);

    // 3 sec for timeout
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + 3;
    timeout.tv_nsec = now.tv_usec * 1000;
    int ret;

    if (bEnable && (isPlaying() == false))
    {
        pthread_mutex_lock(&mP2WMutex);
        // open input file for playback
        if ((workMode == RECPLAY_MODE) || (workMode == RECPLAY_HF_MODE))
        {
            m_pInputFile = fopen(m_strInputFileName, "rb");
            SXLOGD("[Dual-Mic] open input file filename:%s", m_strInputFileName);
            if (m_pInputFile == NULL)
            {
                SXLOGW("[Dual-Mic] open input file fail!!");
                pthread_mutex_unlock(&mP2WMutex);
                return BAD_VALUE;
            }
        }

        m_pOutputFile = fopen(m_strOutFileName, "wb");
        SXLOGD("[Dual-Mic] open output file filename:%s", m_strOutFileName);
        if (m_pOutputFile == NULL)
        {
            SXLOGW("[Dual-Mic] open output file fail!!");
            fclose(m_pInputFile);
            pthread_mutex_unlock(&mP2WMutex);
            return BAD_VALUE;
        }

        m_bWBMode = bWBMode;
        // Enable the audio power - afe/adc/analog
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ADC, true);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true);

        //set input devices
        mAudioResourceManager->setUlInputDevice(AUDIO_DEVICE_IN_BUILTIN_MIC);
        //set MIC gain
        if (workMode > RECONLY_MODE)
        {
            mAudioSpeechEnhanceInfoInstance-> SetAPTuningMode(HANDSFREE_MODE_DMNR);
            mAudioVolumeInstance->SetMicGainTuning(Handfree_Mic, mDualMicTool_micGain[1]);
        }
        else
        {
            mAudioSpeechEnhanceInfoInstance-> SetAPTuningMode(NORMAL_MODE_DMNR);
            mAudioVolumeInstance->SetMicGainTuning(Normal_Mic, mDualMicTool_micGain[0]);
        }

        //set output and output gain in dB
        if ((workMode == RECPLAY_MODE) || (workMode == RECPLAY_HF_MODE))
        {
            uint32 dev = outputDevice == OUTPUT_DEVICE_RECEIVER ? AUDIO_DEVICE_OUT_EARPIECE : AUDIO_DEVICE_OUT_WIRED_HEADSET;
            uint32 volume = outputDevice == OUTPUT_DEVICE_RECEIVER ? mDualMicTool_receiverGain : mDualMicTool_headsetGain;
            uint32 mode = outputDevice == OUTPUT_DEVICE_RECEIVER ? VOLUME_NORMAL_MODE : VOLUME_HEADSET_MODE;
            mAudioResourceManager->setDlOutputDevice(dev);
            setPlaybackVolume(mode, volume);
            SXLOGD("Play+Rec set dual mic at ap side, dev:0x%x, mode:%d, gain:%d", dev, mode, volume);
        }

        // open buffer thread
        SXLOGD("open DMNR Tuning At Ap side threadloop thread~");
        pthread_mutex_lock(&mDMNRMutex);
        ret = pthread_create(&mDMNRThreadID, NULL, DMNR_Play_Rec_ApSide_Routine, (void *)this);
        if (ret != 0)
        {
            SXLOGE("DMNR Tuning At Ap side pthread_create error!!");
        }

        SXLOGD("+mDMNRExit_Cond wait");
        ret = pthread_cond_timedwait(&mDMNRExit_Cond, &mDMNRMutex, &timeout);
        SXLOGD("-mDMNRExit_Cond receive ret=%d", ret);
        pthread_mutex_unlock(&mDMNRMutex);

        m_bDMNRPlaying = true;
        usleep(10 * 1000);

        pthread_mutex_unlock(&mP2WMutex);
    }
    else if (!bEnable && m_bDMNRPlaying)
    {
        pthread_mutex_lock(&mP2WMutex);
        //stop buffer thread
        SXLOGD("close DMNR Tuning At Ap side");
        pthread_mutex_lock(&mDMNRMutex);
        if (!m_bDMNRThreadExit)
        {
            m_bDMNRThreadExit = true;
            SXLOGD("+mDMNRExit_Cond wait");
            ret = pthread_cond_timedwait(&mDMNRExit_Cond, &mDMNRMutex, &timeout);
            SXLOGD("-mDMNRExit_Cond receive ret=%d", ret);
        }
        pthread_mutex_unlock(&mDMNRMutex);

        // Stop PMIC digital/analog part
        if ((workMode == RECPLAY_MODE) || (workMode == RECPLAY_HF_MODE))
        {
            mAudioResourceManager->StopOutputDevice();
        }
        mAudioResourceManager->StopInputDevice();

        //wait to make sure all message is processed
        usleep(200 * 1000);
        //set back MIC gain
        AUDIO_VER1_CUSTOM_VOLUME_STRUCT VolumeCustomParam;//volume custom data
        GetVolumeVer1ParamFromNV(&VolumeCustomParam);

        uint32 voldB = VolumeCustomParam.audiovolume_mic[VOLUME_NORMAL_MODE][3];
        voldB = voldB > UPLINK_GAIN_MAX ? UPLINK_GAIN_MAX : voldB;
        mAudioVolumeInstance->SetMicGainTuning(Normal_Mic, voldB);

        voldB = VolumeCustomParam.audiovolume_mic[VOLUME_SPEAKER_MODE][3];
        voldB = voldB > UPLINK_GAIN_MAX ? UPLINK_GAIN_MAX : voldB;
        mAudioVolumeInstance->SetMicGainTuning(Handfree_Mic, voldB);

        mAudioSpeechEnhanceInfoInstance-> SetAPTuningMode(TUNING_MODE_NONE);

        m_bDMNRPlaying = false;
        // Enable the audio power - afe/adc/analog
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ADC, false);
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);

        if (m_pInputFile) { fclose(m_pInputFile); }
        if (m_pOutputFile) { fclose(m_pOutputFile); }
        m_pInputFile = NULL;
        m_pOutputFile = NULL;
        pthread_mutex_unlock(&mP2WMutex);
    }
    else
    {
        SXLOGD("The DMNR Tuning State is error, bEnable=%d, playing=%d", bEnable, m_bPlaying);
        return BAD_VALUE;
    }

    return NO_ERROR;
}
#endif
#endif

status_t AudioParamTuning::setSphVolume(uint32 mode, uint32 gain)
{
    SXLOGV("setSphVolume in +");
    int32 degradeDb = (DEVICE_VOLUME_STEP - MapVoiceVolumetoCustom(mode, gain)) / VOICE_ONEDB_STEP;
    int voiceAnalogRange = DEVICE_MAX_VOLUME - DEVICE_MIN_VOLUME;

    switch (mode)
    {
        case VOLUME_NORMAL_MODE:
            if (degradeDb <= AUDIO_BUFFER_HW_GAIN_STEP)
            {
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTL, degradeDb);
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTR, degradeDb);
                mAudioVolumeInstance->ApplyMdDlGain(0);
            }
            else
            {
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTL, voiceAnalogRange);
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HSOUTR, voiceAnalogRange);
                degradeDb -= voiceAnalogRange;
                mAudioVolumeInstance->ApplyMdDlGain(degradeDb);
            }
            break;
        case VOLUME_HEADSET_MODE:
            if (degradeDb <= AUDIO_BUFFER_HW_GAIN_STEP)
            {
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, degradeDb);
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, degradeDb);
                mAudioVolumeInstance->ApplyMdDlGain(0);
            }
            else
            {
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, voiceAnalogRange);
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, voiceAnalogRange);
                degradeDb -= voiceAnalogRange;
                mAudioVolumeInstance->ApplyMdDlGain(degradeDb);
            }
            break;
        case VOLUME_SPEAKER_MODE:
            if (degradeDb <= AUDIO_BUFFER_HW_GAIN_STEP)
            {
#ifdef USING_EXTAMP_HP
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, degradeDb);
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, degradeDb);
#else
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_SPKL, degradeDb);
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_SPKR, degradeDb);
#endif
                mAudioVolumeInstance->ApplyMdDlGain(0);
            }
            else
            {
                voiceAnalogRange = DEVICE_AMP_MAX_VOLUME - DEVICE_AMP_MIN_VOLUME;
#ifdef USING_EXTAMP_HP
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTL, voiceAnalogRange);
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_HPOUTR, voiceAnalogRange);
#else

                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_SPKL, voiceAnalogRange);
                mAudioAnalogInstance->SetAnalogGain(AudioAnalogType::VOLUME_SPKR, voiceAnalogRange);
#endif
                degradeDb -= voiceAnalogRange;
                mAudioVolumeInstance->ApplyMdDlGain(degradeDb);
            }
            break;
        case VOLUME_HEADSET_SPEAKER_MODE:
            // nothing to do
            break;
        default:
            break;
    }

    return NO_ERROR;
}

status_t AudioParamTuning::openModemDualMicCtlFlow(bool bWB, bool bRecOnly)
{
    // get speech driver instance
    int sampleRate = 16000;
    const modem_index_t    modem_index   = mSpeechDriverFactory->GetActiveModemIndex();

    AudioDigitalType::InterConnectionOutput modem_pcm_tx_lch = (modem_index == MODEM_1) ? AudioDigitalType::O17 : AudioDigitalType::O07;
    AudioDigitalType::InterConnectionOutput modem_pcm_tx_rch = (modem_index == MODEM_1) ? AudioDigitalType::O18 : AudioDigitalType::O08;

    // connect UL path
    mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I03, modem_pcm_tx_lch); // ADC_I2S_IN_L   -> MODEM_PCM_TX_L
    mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I04, modem_pcm_tx_rch); // ADC_I2S_IN_R   -> MODEM_PCM_TX_R

    if (!bRecOnly)
    {
        // connect DL path
        AudioDigitalType::InterConnectionInput  modem_pcm_rx = (modem_index == MODEM_1) ? AudioDigitalType::I14 : AudioDigitalType::I09;
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, modem_pcm_rx, AudioDigitalType::O03); // MODEM_PCM_RX   -> DAC_I2S_OUT_L
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::Connection, modem_pcm_rx, AudioDigitalType::O04); // MODEM_PCM_RX   -> DAC_I2S_OUT_R
    }

    // start ADC setting
    AudioDigtalI2S AdcI2SIn;
    AdcI2SIn.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
    AdcI2SIn.mBuffer_Update_word = 8;
    AdcI2SIn.mFpga_bit_test = 0;
    AdcI2SIn.mFpga_bit = 0;
    AdcI2SIn.mloopback = 0;
    AdcI2SIn.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
    AdcI2SIn.mI2S_FMT = AudioDigtalI2S::I2S;
    AdcI2SIn.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
    AdcI2SIn.mI2S_SAMPLERATE = sampleRate;
    AdcI2SIn.mI2S_EN = false;
    mAudioDigitalInstance->SetI2SAdcIn(&AdcI2SIn);

    if (!bRecOnly)
    {
        AudioDigtalI2S dac_i2s_out_attribute;
        memset((void *)&dac_i2s_out_attribute, 0, sizeof(dac_i2s_out_attribute));

        dac_i2s_out_attribute.mLR_SWAP = AudioDigtalI2S::NO_SWAP;
        dac_i2s_out_attribute.mI2S_SAMPLERATE = sampleRate;
        dac_i2s_out_attribute.mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
        dac_i2s_out_attribute.mI2S_FMT = AudioDigtalI2S::I2S;
        dac_i2s_out_attribute.mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
        mAudioDigitalInstance->SetI2SDacOut(&dac_i2s_out_attribute);
    }

    mAudioDigitalInstance->SetI2SAdcEnable(true);
    mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_IN_ADC, sampleRate);
    mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_OUT_DAC, sampleRate);
    if (!bRecOnly)
    {
        mAudioDigitalInstance->SetI2SDacEnable(true);
    }

    // set MODEM_PCM
    AudioDigitalPCM modem_pcm_attribute;
    memset((void *)&modem_pcm_attribute, 0, sizeof(modem_pcm_attribute));

    // modem 2 only
    if (modem_index == MODEM_2)
    {
        // TODO: only config internal modem here.. Add external modem setting by project config!!
        modem_pcm_attribute.mExtModemSel          = AudioDigitalPCM::MODEM_2_USE_INTERNAL_MODEM;
        modem_pcm_attribute.mSlaveModeSel         = AudioDigitalPCM::SALVE_MODE;
        modem_pcm_attribute.mAsyncFifoSel         = AudioDigitalPCM::SLAVE_USE_ASRC;

        modem_pcm_attribute.mExtendBckSyncTypeSel = AudioDigitalPCM::BCK_CYCLE_SYNC; // short sync
        modem_pcm_attribute.mExtendBckSyncLength  = 0;
    }
    else if (modem_index == MODEM_EXTERNAL)
    {
        // TODO: only config internal modem here.. Add external modem setting by project config!!
        modem_pcm_attribute.mExtModemSel          = AudioDigitalPCM::MODEM_2_USE_INTERNAL_MODEM;
        modem_pcm_attribute.mSlaveModeSel         = AudioDigitalPCM::SALVE_MODE;
        modem_pcm_attribute.mAsyncFifoSel         = AudioDigitalPCM::SLAVE_USE_ASRC;

        modem_pcm_attribute.mExtendBckSyncTypeSel = AudioDigitalPCM::BCK_CYCLE_SYNC; // short sync
        modem_pcm_attribute.mExtendBckSyncLength  = 0;
    }

    // here modem_1 & modem_2 use the same config, but register field offset are not the same
    modem_pcm_attribute.mVbt16kModeSel      = AudioDigitalPCM::VBT_16K_MODE_DISABLE;

    modem_pcm_attribute.mSingelMicSel       = AudioDigitalPCM::DUAL_MIC_ON_TX;
    modem_pcm_attribute.mTxLchRepeatSel     = AudioDigitalPCM::TX_LCH_NO_REPEAT;

    modem_pcm_attribute.mPcmWordLength      = AudioDigitalPCM::PCM_16BIT;
    modem_pcm_attribute.mPcmModeWidebandSel = (sampleRate == 8000) ? AudioDigitalPCM::PCM_MODE_8K : AudioDigitalPCM::PCM_MODE_16K;
    modem_pcm_attribute.mPcmFormat          = AudioDigitalPCM::PCM_MODE_B;
    modem_pcm_attribute.mModemPcmOn         = false;

    mAudioDigitalInstance->SetModemPcmConfig(modem_index, &modem_pcm_attribute);
    mAudioDigitalInstance->SetModemPcmEnable(modem_index, true);

    mAudioDigitalInstance->SetAfeEnable(true);

    return NO_ERROR;
}

status_t AudioParamTuning::closeModemDualMicCtlFlow(bool bRecOnly)
{
    // get speech driver instance
    const modem_index_t modem_index = mSpeechDriverFactory->GetActiveModemIndex();

    // set MODEM_PCM
    mAudioDigitalInstance->SetModemPcmEnable(modem_index, false);

    if (!bRecOnly)
    {
        mAudioDigitalInstance->SetI2SDacEnable(false);
    }

    // disable ADC
    mAudioDigitalInstance->SetI2SAdcEnable(false);

    if (!bRecOnly)
    {
        // disconnect DL path
        AudioDigitalType::InterConnectionInput  modem_pcm_rx = (modem_index == MODEM_1) ? AudioDigitalType::I14 : AudioDigitalType::I09;
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, modem_pcm_rx, AudioDigitalType::O03); // MODEM_PCM_RX -> DAC_I2S_OUT_L
        mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, modem_pcm_rx, AudioDigitalType::O04); // MODEM_PCM_RX -> DAC_I2S_OUT_R
    }

    // disconnect UL path
    AudioDigitalType::InterConnectionOutput modem_pcm_tx_lch = (modem_index == MODEM_1) ? AudioDigitalType::O17 : AudioDigitalType::O07;
    AudioDigitalType::InterConnectionOutput modem_pcm_tx_rch = (modem_index == MODEM_1) ? AudioDigitalType::O18 : AudioDigitalType::O08;
    mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I03, modem_pcm_tx_lch); // ADC_I2S_IN_L -> MODEM_PCM_TX_L
    mAudioDigitalInstance->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I04, modem_pcm_tx_rch); // ADC_I2S_IN_R -> MODEM_PCM_TX_R

    // AFE_ON = false
    mAudioDigitalInstance->SetAfeEnable(false);
    return NO_ERROR;
}
};
