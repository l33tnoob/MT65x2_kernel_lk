#include "AudioBTCVSDControl.h"
#include "AudioLoopbackController.h"
#include <utils/Log.h>

#define LOG_TAG  "AudioBTCVSDControl"

//#define VERY_VERBOSE_LOGGING
#ifdef VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif


namespace android
{

#ifdef BT_SW_CVSD

#define BTSCO_CVSD_SAMPLERATE_DOMAIN 64000
#define BTSCO_MSBC_SAMPLERATE_DOMAIN 16000
#define BTSCO_CVSD_PLC_SAMPLERATE 8000
#define BTSCO_CVSD_CHANNEL_NUM 1
#define BTSCO_MSBC_CHANNEL_NUM 1

#ifdef EXT_MODEM_BT_CVSD
#define BTSCO_EXTMD_SAMPLERATE 8000
#endif

static const int32_t  btsco_FilterCoeff_64K[4] = {0x07F54A0C, 0xF017CB31, 0x07F2EB36, 0x7F41C0BE};
static const int32_t  btsco_FilterCoeff_8K[4] = {0x07AB676D, 0xF0BB80B9, 0x079933A1, 0x79F9C5B0};
BTSCO_CVSD_Context *AudioBTCVSDControl::mBTSCOCVSDContext = NULL;


AudioBTCVSDControl *AudioBTCVSDControl::UniqueAudioBTCVSDControl = NULL;

AudioBTCVSDControl *AudioBTCVSDControl::getInstance()
{
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (UniqueAudioBTCVSDControl == NULL)
    {
        ALOGD("+AudioBTCVSDControl");
        UniqueAudioBTCVSDControl = new AudioBTCVSDControl();
        UniqueAudioBTCVSDControl->BTmode = BT_SCO_MODE_CVSD;
        ALOGD("-AudioBTCVSDControl");
    }
    ALOGD("AudioBTCVSDControl getInstance()");
    return UniqueAudioBTCVSDControl;
}

void AudioBTCVSDControl::freeInstance()
{
    if (UniqueAudioBTCVSDControl != NULL)
    {
        delete UniqueAudioBTCVSDControl;
    }
    ALOGD("AudioBTCVSDControl freeInstance()");
}

AudioBTCVSDControl::AudioBTCVSDControl()
{
    mExtMDBTSCORunning = false;
    ALOGD("AudioBTCVSDControl constructor");
}

AudioBTCVSDControl::~AudioBTCVSDControl()
{
    ALOGD("AudioBTCVSDControl destructor");

}

void AudioBTCVSDControl::BT_SCO_CVSD_Init(void)
{
    mBTSCOCVSDContext = NULL;
    mBTSCOCVSDContext = (BTSCO_CVSD_Context *)new char[sizeof(BTSCO_CVSD_Context)];
    ASSERT(mBTSCOCVSDContext);
    memset((void *)mBTSCOCVSDContext, 0, sizeof(BTSCO_CVSD_Context));
#if 0  //set to 1 for WB BTSCO test
    ALOGD("BT_SCO_CVSD_Init() !!!force to WB BTSCO(test code)!!!");
    BT_SCO_SetMode(1);
#endif
    ALOGD("BT_SCO_CVSD_Init() allocate mBTSCOCVSDContext");
}

void AudioBTCVSDControl::BT_SCO_CVSD_DeInit(void)
{
    if (mBTSCOCVSDContext)
    {
        delete []mBTSCOCVSDContext;
        mBTSCOCVSDContext = NULL;
        ALOGD("BT_SCO_CVSD_DeInit() release mBTSCOCVSDContext");
    }
}

void AudioBTCVSDControl::BT_SCO_SetMode(uint32_t mode)
{

    if (mode == 1)
    {
        BTmode = BT_SCO_MODE_MSBC;
    }
    else
    {
        BTmode = BT_SCO_MODE_CVSD;
    }
    ALOGD("BT_SCO_SetMode, mode=%d, BTmode=%d", mode, BTmode);

    if (mBTSCOCVSDContext)
    {
        mBTSCOCVSDContext->fIsWideBand = BTmode;
    }
}

bool AudioBTCVSDControl::BT_SCO_isWideBand(void)
{
    return BTmode;
}

uint32_t AudioBTCVSDControl::BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MODULE uModule)
{
    uint32_t uSize;
    switch (uModule)
    {
        case BT_SCO_MOD_CVSD_ENCODE:
            uSize = (uint32_t)CVSD_ENC_GetBufferSize();
            break;
        case BT_SCO_MOD_CVSD_DECODE:
            uSize = (uint32_t)CVSD_DEC_GetBufferSize();
            break;
        case BT_SCO_MOD_FILTER_TX:
        case BT_SCO_MOD_FILTER_RX:
            uSize = (uint32_t)Audio_IIRHPF_GetBufferSize(1);
            break;
        case BT_SCO_MOD_PLC_NB:
        case BT_SCO_MOD_PLC_WB:
            uSize = (uint32_t)g711plc_GetMemorySize_v2();
            break;
        case BT_SCO_MOD_CVSD_TX_SRC:
            //BLI_GetMemSize(mBTSCOCVSDContext->pTX->uSampleRate, mBTSCOCVSDContext->pTX->uChannelNumber, BTSCO_CVSD_SAMPLERATE_DOMAIN, BTSCO_CVSD_CHANNEL_NUM, &uSize);
            uSize = 0;
            break;
        case BT_SCO_MOD_CVSD_RX_SRC1:
            //BLI_GetMemSize(BTSCO_CVSD_SAMPLERATE_DOMAIN, BTSCO_CVSD_CHANNEL_NUM, BTSCO_CVSD_PLC_SAMPLERATE, BTSCO_CVSD_CHANNEL_NUM, &uSize);
            uSize = 0;
            break;
        case BT_SCO_MOD_CVSD_RX_SRC2:
            //BLI_GetMemSize(8000, 1, mBTSCOCVSDContext->pRX->uSampleRate, mBTSCOCVSDContext->pRX->uChannelNumber, &uSize);
            uSize = 0;
            break;
#if defined(__MSBC_CODEC_SUPPORT__)
        case BT_SCO_MOD_MSBC_ENCODE:
            uSize = (uint32_t)MSBC_ENC_GetBufferSize();
            break;
        case BT_SCO_MOD_MSBC_DECODE:
            uSize = (uint32_t)MSBC_DEC_GetBufferSize();
            break;
        case BT_SCO_MOD_MSBC_TX_SRC:
            //BLI_GetMemSize(mBTSCOCVSDContext->pTX->uSampleRate, mBTSCOCVSDContext->pTX->uChannelNumber, BTSCO_MSBC_SAMPLERATE_DOMAIN, BTSCO_MSBC_CHANNEL_NUM, &uSize);
            uSize = 0;
            break;
        case BT_SCO_MOD_MSBC_RX_SRC:
            //BLI_GetMemSize(BTSCO_MSBC_SAMPLERATE_DOMAIN, BTSCO_MSBC_CHANNEL_NUM, BTSCO_EXTMD_SAMPLERATE, BTSCO_MSBC_CHANNEL_NUM, &uSize);
            uSize = 0;
            break;
#endif
        default:
            ASSERT(0);
    }
    uSize = (uSize + 3) & ~3 ;
    ALOGD("BT_SCO_GetMemorySize_4ByteAlign uModule=%d, uSize=%d", uModule, uSize);
    return uSize;
}

void AudioBTCVSDControl::BT_SCO_TX_DestroyModule(void)
{
    if (mBTSCOCVSDContext->pTX)
    {
        if (mBTSCOCVSDContext->pTX->pSRCHandle)
        {
            mBTSCOCVSDContext->pTX->pSRCHandle->Close();
            delete mBTSCOCVSDContext->pTX->pSRCHandle;
            mBTSCOCVSDContext->pTX->pSRCHandle = NULL;
        }
    }
}

void AudioBTCVSDControl::BT_SCO_RX_DestroyModule(void)
{
    if (mBTSCOCVSDContext->pRX)
    {
        if (mBTSCOCVSDContext->pRX->pSRCHandle_1)
        {
            mBTSCOCVSDContext->pRX->pSRCHandle_1->Close();
            delete mBTSCOCVSDContext->pRX->pSRCHandle_1;
            mBTSCOCVSDContext->pRX->pSRCHandle_1 = NULL;
        }
        if (mBTSCOCVSDContext->pRX->pSRCHandle_2)
        {
            mBTSCOCVSDContext->pRX->pSRCHandle_2->Close();
            delete mBTSCOCVSDContext->pRX->pSRCHandle_2;
            mBTSCOCVSDContext->pRX->pSRCHandle_2 = NULL;
        }
    }
}


void AudioBTCVSDControl::BT_SCO_InitialModule(BT_SCO_MODULE uModule, uint8_t *pBuf)
{
    ASSERT(pBuf);
    switch (uModule)
    {
        case BT_SCO_MOD_CVSD_ENCODE:
            mBTSCOCVSDContext->pTX->pEncHandle = CVSD_ENC_Init((int8_t *)pBuf);
            break;
        case BT_SCO_MOD_CVSD_DECODE:
            mBTSCOCVSDContext->pRX->pDecHandle = CVSD_DEC_Init((int8_t *)pBuf);
            break;
        case BT_SCO_MOD_FILTER_TX:
            mBTSCOCVSDContext->pTX->pHPFHandle = Audio_IIRHPF_Init((int8_t *)pBuf, btsco_FilterCoeff_8K, 1);
            break;
        case BT_SCO_MOD_FILTER_RX:
            mBTSCOCVSDContext->pRX->pHPFHandle = Audio_IIRHPF_Init((int8_t *)pBuf, btsco_FilterCoeff_64K, 1);
            break;
        case BT_SCO_MOD_PLC_NB:
            g711plc_construct_v2((void *)pBuf, BTSCO_CVSD_PLC_SAMPLERATE);
            mBTSCOCVSDContext->pRX->pPLCHandle = (void *)pBuf;
            break;
        case BT_SCO_MOD_CVSD_TX_SRC:
            ALOGD("BT_SCO_InitialModule BT_SCO_MOD_CVSD_TX_SRC source: uSampleRate=%d, uChannelNumber=%d", mBTSCOCVSDContext->pTX->uSampleRate, mBTSCOCVSDContext->pTX->uChannelNumber);
            mBTSCOCVSDContext->pTX->pSRCHandle = new MtkAudioSrc(mBTSCOCVSDContext->pTX->uSampleRate, mBTSCOCVSDContext->pTX->uChannelNumber, BTSCO_CVSD_SAMPLERATE_DOMAIN, BTSCO_CVSD_CHANNEL_NUM, SRC_IN_Q1P15_OUT_Q1P15);
            mBTSCOCVSDContext->pTX->pSRCHandle->Open();
            ALOGD("BT_SCO_InitialModule BT_SCO_MOD_CVSD_TX_SRC pTX->pSRCHandle=0x%x", mBTSCOCVSDContext->pTX->pSRCHandle);
            break;
        case BT_SCO_MOD_CVSD_RX_SRC1:
            ALOGD("BT_SCO_InitialModule BT_SCO_MOD_CVSD_RX_SRC1 target: uSampleRate=%d, uChannelNumber=%d", mBTSCOCVSDContext->pRX->uSampleRate, mBTSCOCVSDContext->pRX->uChannelNumber);
            mBTSCOCVSDContext->pRX->pSRCHandle_1 = new MtkAudioSrc(BTSCO_CVSD_SAMPLERATE_DOMAIN, BTSCO_CVSD_CHANNEL_NUM, mBTSCOCVSDContext->pRX->uSampleRate, mBTSCOCVSDContext->pRX->uChannelNumber, SRC_IN_Q1P15_OUT_Q1P15);
            mBTSCOCVSDContext->pRX->pSRCHandle_1->Open();
            ALOGD("BT_SCO_InitialModule BT_SCO_MOD_CVSD_RX_SRC1 pRX->pSRCHandle_1=0x%x", mBTSCOCVSDContext->pRX->pSRCHandle_1);
            break;
        case BT_SCO_MOD_CVSD_RX_SRC2:
            mBTSCOCVSDContext->pRX->pSRCHandle_2 = new MtkAudioSrc(8000, 1, mBTSCOCVSDContext->pRX->uSampleRate, mBTSCOCVSDContext->pRX->uChannelNumber, SRC_IN_Q1P15_OUT_Q1P15);
            mBTSCOCVSDContext->pRX->pSRCHandle_2->Open();
            break;
#if defined(__MSBC_CODEC_SUPPORT__)
        case BT_SCO_MOD_MSBC_ENCODE:
            mBTSCOCVSDContext->pTX->pEncHandle = MSBC_ENC_Init((int8_t *)pBuf);
            break;
        case BT_SCO_MOD_MSBC_DECODE:
            mBTSCOCVSDContext->pRX->pDecHandle = MSBC_DEC_Init((int8_t *)pBuf);
            break;
        case BT_SCO_MOD_PLC_WB:
            g711plc_construct_v2((void *)pBuf, BTSCO_MSBC_SAMPLERATE_DOMAIN);
            mBTSCOCVSDContext->pRX->pPLCHandle = (void *)pBuf;
            break;
        case BT_SCO_MOD_MSBC_TX_SRC:
            if (mExtMDBTSCORunning == true)
            {
                mBTSCOCVSDContext->pTX->pSRCHandle = new MtkAudioSrc(BTSCO_EXTMD_SAMPLERATE, BTSCO_MSBC_CHANNEL_NUM, BTSCO_MSBC_SAMPLERATE_DOMAIN, BTSCO_MSBC_CHANNEL_NUM, SRC_IN_Q1P15_OUT_Q1P15);
                mBTSCOCVSDContext->pTX->pSRCHandle->Open();
            }
            else
            {
                mBTSCOCVSDContext->pTX->pSRCHandle = new MtkAudioSrc(mBTSCOCVSDContext->pTX->uSampleRate, mBTSCOCVSDContext->pTX->uChannelNumber, BTSCO_MSBC_SAMPLERATE_DOMAIN, BTSCO_MSBC_CHANNEL_NUM, SRC_IN_Q1P15_OUT_Q1P15);
                mBTSCOCVSDContext->pTX->pSRCHandle->Open();
            }
            break;
        case BT_SCO_MOD_MSBC_RX_SRC:
            mBTSCOCVSDContext->pRX->pSRCHandle_1 = new MtkAudioSrc(BTSCO_MSBC_SAMPLERATE_DOMAIN, BTSCO_MSBC_CHANNEL_NUM, BTSCO_EXTMD_SAMPLERATE, BTSCO_MSBC_CHANNEL_NUM, SRC_IN_Q1P15_OUT_Q1P15);
            mBTSCOCVSDContext->pRX->pSRCHandle_1->Open();
            break;
#endif
        default:
            ASSERT(0);
    }
}


void AudioBTCVSDControl::BT_SCO_SET_TXState(BT_SCO_STATE state)
{
    ALOGD("BT_SCO_SET_TXState state=%d", state);
    mBTSCOCVSDContext->uTXState = state;
}

void AudioBTCVSDControl::BT_SCO_SET_RXState(BT_SCO_STATE state)
{
    ALOGD("BT_SCO_SET_RXState state=%d", state);
    mBTSCOCVSDContext->uRXState = state;
}


void AudioBTCVSDControl::BT_SCO_TX_Open(void)
{
    uint32 uTxMemSize = 0;
    char *pAllocMemory;
    ALOGD("BT_SCO_TX_Open(+)");

    uTxMemSize += (sizeof(BT_SCO_TX) + 3)& ~0x3;

    mBTSCOCVSDContext->pTX = (BT_SCO_TX *)new char[uTxMemSize];
    ASSERT(mBTSCOCVSDContext->pTX);
    memset((void *)mBTSCOCVSDContext->pTX, 0, uTxMemSize);

    mBTCVSDTXOutBuf = (uint8_t *)new char[BTSCO_CVSD_TX_OUTBUF_SIZE];
    ALOGD("mBTSCOCVSDContext->uTXState=0x%x", mBTSCOCVSDContext->uTXState);
    ASSERT(mBTSCOCVSDContext->uTXState == BT_SCO_TXSTATE_IDLE);

    mTXSRCPCMDumpFile = NULL;
    mTXSRCPCMDumpFile = AudioOpendumpPCMFile("/sdcard/mtklog/audio_dump/CVSDTXOut.pcm", "CVSDTXOut.pcm.dump");

    ALOGD("BT_SCO_TX_Open(-)");
}

void AudioBTCVSDControl::BT_SCO_TX_Close(void)
{
    ALOGD("BT_SCO_TX_Close(+)");

    if (mBTCVSDTXOutBuf)
    {
        delete []mBTCVSDTXOutBuf;
        mBTCVSDTXOutBuf = NULL;
        ALOGD("BT_SCO_TX_Close() release mBTCVSDTXOutBuf");
    }

    if (mBTSCOCVSDContext->pTX)
    {
        delete []mBTSCOCVSDContext->pTX;
        mBTSCOCVSDContext->pTX = NULL;
        ALOGD("BT_SCO_TX_Close() release mBTSCOCVSDContext->pTX");
    }

    if (mTXSRCPCMDumpFile)
    {
        AudioCloseDumpPCMFile(mTXSRCPCMDumpFile);;
    }

    ALOGD("BT_SCO_TX_Close(-)");
}

void AudioBTCVSDControl::btsco_AllocMemory_TX_CVSD(void)
{
    uint32_t uTotalMemory = 0;
    uint8_t  *pBuf = NULL;
    ALOGD("BT_SCO_TX_Start() (+)");
    ASSERT(mBTSCOCVSDContext->uTXState == BT_SCO_TXSTATE_READY);
    if (mBTSCOCVSDContext->pTX)
    {
        uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_CVSD_ENCODE);
        uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_CVSD_TX_SRC);
        if (mBTSCOCVSDContext->pTX->fEnableFilter)
        {
            uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_FILTER_TX);
        }
    }

    pBuf = new uint8_t[uTotalMemory];
    mBTSCOCVSDContext->pTXWorkingMemory = pBuf;
    ASSERT(mBTSCOCVSDContext->pTXWorkingMemory);

    if (mBTSCOCVSDContext->pTX)
    {
        BT_SCO_InitialModule(BT_SCO_MOD_CVSD_ENCODE, pBuf);
        pBuf += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_CVSD_ENCODE);
        BT_SCO_InitialModule(BT_SCO_MOD_CVSD_TX_SRC, pBuf);
        pBuf += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_CVSD_TX_SRC);
        if (mBTSCOCVSDContext->pTX->fEnableFilter)
        {
            BT_SCO_InitialModule(BT_SCO_MOD_FILTER_TX, pBuf);
            pBuf += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_FILTER_TX);
        }
    }
    ALOGD("btsco_AllocMemory_TX_CVSD %d", uTotalMemory);
}

#if defined(__MSBC_CODEC_SUPPORT__)
void AudioBTCVSDControl::btsco_AllocMemory_TX_MSBC(void)
{
    uint32_t uTotalMemory = 0;
    uint8_t  *pBuf = NULL;

    ASSERT(mBTSCOCVSDContext->uTXState == BT_SCO_TXSTATE_READY);
    if (mBTSCOCVSDContext->pTX)
    {
        uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_MSBC_ENCODE);
        uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_MSBC_TX_SRC);
        if (mBTSCOCVSDContext->pTX->fEnableFilter)
        {
            uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_FILTER_TX);
        }
    }

    pBuf = new uint8_t[uTotalMemory];
    mBTSCOCVSDContext->pTXWorkingMemory = pBuf;
    ASSERT(mBTSCOCVSDContext->pTXWorkingMemory);

    if (mBTSCOCVSDContext->pTX)
    {
        BT_SCO_InitialModule(BT_SCO_MOD_MSBC_ENCODE, pBuf);
        pBuf += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_MSBC_ENCODE);
        BT_SCO_InitialModule(BT_SCO_MOD_MSBC_TX_SRC, pBuf);
        pBuf += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_MSBC_TX_SRC);
        if (mBTSCOCVSDContext->pTX->fEnableFilter)
        {
            BT_SCO_InitialModule(BT_SCO_MOD_FILTER_TX, pBuf);
            pBuf += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_FILTER_TX);
        }
    }

    ALOGD("btsco_AllocMemory_TX_MSBC %d", uTotalMemory);
}
#endif

void AudioBTCVSDControl::BT_SCO_TX_Start(void)
{

    ALOGD("BT_SCO_TX_Start() (+)");

    mBTSCOCVSDContext->fIsWideBand = BTmode;

    if (mBTSCOCVSDContext->fIsWideBand)
    {
#if defined(__MSBC_CODEC_SUPPORT__)
        btsco_AllocMemory_TX_MSBC();
#else
        ASSERT(0);
#endif
    }
    else
    {
        btsco_AllocMemory_TX_CVSD();
    }

    ALOGD("BT_SCO_TX_Start() (-)");
}


void AudioBTCVSDControl::BT_SCO_TX_Stop(void)
{
    ALOGD("BT_SCO_TX_Stop(+)");
    BT_SCO_TX_DestroyModule();

    if (mBTSCOCVSDContext->pTXWorkingMemory)
    {
        delete []mBTSCOCVSDContext->pTXWorkingMemory;
        mBTSCOCVSDContext->pTXWorkingMemory = NULL;
    }
    ALOGD("BT_SCO_TX_Stop(-)");
}

void AudioBTCVSDControl::BT_SCO_TX_Begin(int mFd2, uint32_t uSampleRate, uint32_t uChannelNumber)
{
    BT_SCO_TX_Open(); //Allocate btsco_cvsd_tx_outbuf and working buffer
    ALOGD("ioctl mFd2=0x%x, cmd=0x%x", mFd2, ALLOCATE_FREE_BTCVSD_BUF);
    ::ioctl(mFd2, ALLOCATE_FREE_BTCVSD_BUF, 0); //allocate TX working buffers in kernel
    BT_SCO_SET_TXState(BT_SCO_TXSTATE_INIT);
    BT_SCO_TX_SetHandle(NULL, NULL, uSampleRate, uChannelNumber, 0);
    BT_SCO_SET_TXState(BT_SCO_TXSTATE_READY);
    BT_SCO_TX_Start();
    BT_SCO_SET_TXState(BT_SCO_TXSTATE_RUNNING);
    ::ioctl(mFd2, SET_BTCVSD_STATE, BT_SCO_TXSTATE_RUNNING); //set state to kernel, ISR will not do anything until state is set to RUNNING

}

void AudioBTCVSDControl::BT_SCO_TX_End(int mFd2)
{
    BT_SCO_SET_TXState(BT_SCO_TXSTATE_ENDING);
    ::ioctl(mFd2, SET_BTCVSD_STATE, BT_SCO_TXSTATE_ENDING); //set kernel state to ENDING  for push remaining TX datat to BT HW
    BT_SCO_TX_Stop();
    BT_SCO_TX_Close();
    BT_SCO_SET_TXState(BT_SCO_TXSTATE_IDLE);
    ::ioctl(mFd2, SET_BTCVSD_STATE, BT_SCO_TXSTATE_IDLE);
    ::ioctl(mFd2, ALLOCATE_FREE_BTCVSD_BUF, 1); //free TX working buffers in kernel
}

int AudioBTCVSDControl::BT_SCO_TX_SetHandle(void(*pCallback)(void *pData), void *pData, uint32_t uSampleRate, uint32_t uChannelNumber, uint32_t uEnableFilter)
{
    ASSERT(mBTSCOCVSDContext->uTXState == BT_SCO_TXSTATE_INIT);
    if (uChannelNumber != 1 && uChannelNumber != 2)
    {
        return -1;
    }
    if (uSampleRate != 8000  && uSampleRate != 11025 && uSampleRate != 12000 &&
        uSampleRate != 16000 && uSampleRate != 22050 && uSampleRate != 24000 &&
        uSampleRate != 32000 && uSampleRate != 44100 && uSampleRate != 48000)
    {
        return -2;
    }

    ASSERT(mBTSCOCVSDContext->pTX);
    mBTSCOCVSDContext->pTX->pCallback    = pCallback;
    mBTSCOCVSDContext->pTX->uSampleRate  = (uint16_t)uSampleRate;
    mBTSCOCVSDContext->pTX->uChannelNumber = (uint8_t)uChannelNumber;
    mBTSCOCVSDContext->pTX->pUserData    = pData;
    mBTSCOCVSDContext->pTX->fEnableFilter  = uEnableFilter;

    return 0;
}

int AudioBTCVSDControl::BT_SCO_RX_SetHandle(void(*pCallback)(void *pData), void *pData, uint32_t uSampleRate, uint32_t uChannelNumber, uint32_t uEnableFilter)
{
    ASSERT(mBTSCOCVSDContext->uRXState == BT_SCO_RXSTATE_INIT);
    if (uChannelNumber != 1 && uChannelNumber != 2)
    {
        return -1;
    }
    if (uSampleRate != 8000  && uSampleRate != 11025 && uSampleRate != 12000 &&
        uSampleRate != 16000 && uSampleRate != 22050 && uSampleRate != 24000 &&
        uSampleRate != 32000 && uSampleRate != 44100 && uSampleRate != 48000)
    {
        return -2;
    }

    if (uSampleRate == 8000)
    {
        mBTSCOCVSDContext->pRX->fEnablePLC = true;
    }

    ASSERT(mBTSCOCVSDContext->pRX);
    mBTSCOCVSDContext->pRX->pCallback     = pCallback;
    mBTSCOCVSDContext->pRX->uSampleRate   = (uint16_t)uSampleRate;
    mBTSCOCVSDContext->pRX->uChannelNumber = (uint8_t)uChannelNumber;
    mBTSCOCVSDContext->pRX->pUserData     = pData;
    mBTSCOCVSDContext->pRX->fEnableFilter  = uEnableFilter;
    if (mBTSCOCVSDContext->pRX->uSampleRate != 8000 || mBTSCOCVSDContext->pRX->uChannelNumber != 1)
    {
        mBTSCOCVSDContext->pRX->fEnableSRC2 = true;
    }

    return 0;
}



/*
btsco_process_RX_CVSD(void *inbuf, uint32 *insize, void *outbuf, uint32 *outsize, void *workbuf, const uint32 workbufsize, uint8_t packetvalid)

void *inbuf                                         : inbuf of CVSD bitstream for decode
uint32 *insize                                     :  in: CVSD bitstream length for decode (i.e. SCO_RX_PLC_SIZE),
                                                out: consumed length of CVSD bitstream
void *outbuf                                     : outbuf of CVSD decoded pcm data (I.e. PcmBuf_8k)
uint32 *outsize                                 :  in: desired output length of CVSD decoded pcm data (i.e. SCO_RX_PCM8K_BUF_SIZE)
                                                                   out: practical output length of CVSD decoded pcm data
void *workbuf                                  : working buf during CVSD decode (i.e. PcmBuf_64k)
const uint32 workbufsize              : working buf size during CVSD decode (i.e. SCO_RX_PCM64K_BUF_SIZE)
uint8_t packetvalid           : Is this SCO_RX_PLC_SIZE packet valid (without packet loss)
*/

void AudioBTCVSDControl::btsco_process_RX_CVSD(void *inbuf, uint32_t *insize, void *outbuf, uint32_t *outsize, void *workbuf, const uint32_t workbufsize, uint8_t packetvalid)
{
    uint16_t *pDst, *pSrc, i;
    int32_t iOutSample = 0, iInByte = 0, consumed;

    consumed = *insize;
    iOutSample = workbufsize >> 1;
    ALOGVV("btsco_process_RX_CVSD() CVSD_DEC_Process(+) *insize=%d, iOutSample=%d", *insize, iOutSample);
    CVSD_DEC_Process(mBTSCOCVSDContext->pRX->pDecHandle, (char *)inbuf, (int *)insize, (short *)workbuf, (int *)&iOutSample);
    ALOGVV("btsco_process_RX_CVSD() CVSD_DEC_Process(-) remaining=%d, iOutSample=%d", *insize, iOutSample);

    if (iOutSample != (SCO_RX_PCM64K_BUF_SIZE >> 1))
    {
        ALOGE("ERROR!!!btsco_process_RX_CVSD() iOutSample!=(SCO_RX_PCM64K_BUF_SIZE>>1)!!!!,iOutSample=%d", iOutSample);
    }

    consumed -= *insize;
    *insize = consumed;
    {
        uint32_t uOutByte = 0, uInByte = 0, uConsumeByte = 0;
        uInByte = iOutSample << 1; //should be SCO_RX_PCM64K_BUF_SIZE!!!
        uOutByte = *outsize;

        ALOGVV("btsco_process_RX_CVSD() BLI_Convert(+) uInByte=%d, uOutByte=%d", uInByte, uOutByte);

        uConsumeByte = uInByte;
        mBTSCOCVSDContext->pRX->pSRCHandle_1->Process((int16_t *)workbuf, &uInByte, (int16_t *)outbuf, &uOutByte);
        uConsumeByte -= uInByte;

        ALOGVV("btsco_process_RX_CVSD() BLI_Convert(-) remaining=%d, uConsumeByte=%d, uOutByte=%d", uInByte, uConsumeByte, uOutByte);
        ASSERT(uConsumeByte == workbufsize);
        *outsize = uOutByte;
    }

    if (mBTCVSDRXDumpFile)
    {
        fwrite((void *)outbuf, 1, *outsize, mBTCVSDRXDumpFile);
    }

    if (AudioLoopbackController::GetInstance()->IsAPBTLoopbackWithCodec() == true)
    {
        mBTSCOCVSDContext->pRX->fEnablePLC = false;
        if (packetvalid == 0)
        {
            ALOGD("btsco_process_RX_CVSD(), packet lost, in loopback mode, no PLC!!!");
        }
    }

    if (mBTSCOCVSDContext->pRX->fEnablePLC)
    {
        //do PLC
        if (packetvalid)
        {
            //packet not lost
            g711plc_addtohistory_v2(mBTSCOCVSDContext->pRX->pPLCHandle, (short *)outbuf, 0);
        }
        else
        {
            //packet lost

            ALOGD("btsco_process_RX_CVSD(), packet lost, do PLC!!!");
#if 0
            for (i = 0; i < SCO_RX_PLC_SIZE; i++)
            {
                ALOGD("%x ", *((char *)inbuf + i)); //should be 0x55
            }
#endif

            g711plc_dofe_v2(mBTSCOCVSDContext->pRX->pPLCHandle, (short *)outbuf, 0);
        }
    }
    if (mBTSCOCVSDContext->pRX->fEnableFilter)
    {
        //do filter
        int32_t iInSample = *outsize >> 1;
        int32_t iOutSample = *outsize >> 1;
        Audio_IIRHPF_Process(mBTSCOCVSDContext->pRX->pHPFHandle, (uint16_t *)outbuf, (uint16_t *)&iInSample, (uint16_t *)outbuf, (uint16_t *)&iOutSample);
        *outsize = iOutSample << 1;
    }

    ALOGVV("btsco_process_RX_CVSD() consumed=%d, *outsize=%d", *insize, *outsize);
}

/*
btsco_process_RX_MSBC(void *inbuf, uint32 *insize, void *outbuf, uint32 *outsize, void *workbuf, const uint32 workbufsize, uint8_t packetvalid)

void *inbuf                                         : inbuf of MSBC bitstream for decode
uint32 *insize                                     :  in: MSBC bitstream length for decode,
                                                out: consumed length of MSBC bitstream
void *outbuf                                     : outbuf of CVSD decoded pcm data (I.e. PcmBuf_8k)
uint32 *outsize                                 :  in: desired output length of MSBC decoded pcm data
                                                                   out: practical output length of MSBC decoded pcm data
void *workbuf                                  : working buf during MSBC decode
const uint32 workbufsize              : working buf size during MSBC decode
uint8_t packetvalid           : Is this SCO_RX_PLC_SIZE packet valid (without packet loss)
*/

#if defined(__MSBC_CODEC_SUPPORT__)

void AudioBTCVSDControl::btsco_process_RX_MSBC(void *inbuf, uint32_t *insize, void *outbuf, uint32_t *outsize, void *workbuf, const uint32_t workbufsize, uint8_t packetvalid)
{
    uint16_t *pDst, *pSrc, i;
    int32_t iOutSample = 0, iInByte = 0, consumed;
    uint32_t index1 = 0, index2 = 0, dwBtEv3HalfBad = 0;
    uint8_t *pSrc8;
    int32_t status = 0;

    pSrc8 = (uint8_t *)inbuf;
    iInByte = MSBC_PACKET_SIZE_BYTE >> 1;
    iOutSample = MSBC_PCM_FRAME_BYTE >> 1;

    // Put into packet buffer
    if (*insize < iInByte)
    {
        *insize = 0;
        *outsize = 0;
        return;
    }
    index1 = mBTSCOCVSDContext->pRX->iPacket_w & SCO_RX_PACKET_MASK;
    memcpy(mBTSCOCVSDContext->pRX->PacketBuf[index1], inbuf, iInByte);
    mBTSCOCVSDContext->pRX->PacketValid[index1] = packetvalid;
    mBTSCOCVSDContext->pRX->iPacket_w++;

    if (((mBTSCOCVSDContext->pRX->iPacket_w - mBTSCOCVSDContext->pRX->iPacket_r) < 2) || (*outsize < MSBC_PCM_FRAME_BYTE))
    {
        //*insize = iInByte;
        *outsize = 0;
        return;
    }

    index1 = mBTSCOCVSDContext->pRX->iPacket_r & SCO_RX_PACKET_MASK;
    index2 = (mBTSCOCVSDContext->pRX->iPacket_r + 1) & SCO_RX_PACKET_MASK;
    mBTSCOCVSDContext->pRX->iPacket_r++;  // Consume 30 bytes

    if ((mBTSCOCVSDContext->pRX->PacketBuf[index1][0] == 0x1) && ((mBTSCOCVSDContext->pRX->PacketBuf[index1][1] & 0xF) == 0x8))
    {
        int32_t iInByteTemp = MSBC_BTSTREAM_FRAME_BYTE;

        //ALOGD("btsco_process_RX_MSBC() index1=%d, index2=%d",index1,index2);

        // Copy to bitstream buffer for decoding
        memcpy(mBTSCOCVSDContext->pRX->EntirePacket, &mBTSCOCVSDContext->pRX->PacketBuf[index1][2], 28);
        memcpy(mBTSCOCVSDContext->pRX->EntirePacket + 28, &mBTSCOCVSDContext->pRX->PacketBuf[index2][0], 29);

        ALOGD("btsco_process_RX_MSBC() MSBC_DEC_Process(+)");
        status = MSBC_DEC_Process(mBTSCOCVSDContext->pRX->pDecHandle, (char *)mBTSCOCVSDContext->pRX->EntirePacket, &iInByteTemp, (int16_t *)workbuf, &iOutSample);
        ALOGD("btsco_process_RX_MSBC() MSBC_DEC_Process(-) status=%d, iOutSample=%d", status, iOutSample);
        ASSERT((iOutSample == MSBC_PCM_FRAME_BYTE >> 1) || (iOutSample == 0));


        if (mBTCVSDRXDumpFile)
        {
            fwrite((void *)workbuf, 1, iOutSample << 1, mBTCVSDRXDumpFile);
        }

        mBTSCOCVSDContext->pRX->iPacket_r++;   // Consume another 30 bytes for entire packet
    }
    else if (mBTSCOCVSDContext->pRX->PacketValid[index1] == 1)
    {
        //wrong header, but packet not lost, sync to next 30 byte block (should only happenes on packetsize 30)
        status = -1;
        ALOGD("btsco_process_RX_MSBC() wrong header, but packet not lost, sync to next 30 byte block!!!");
    }
    else
    {
        status = -1;

        mBTSCOCVSDContext->pRX->iPacket_r++;   // Consume another 30 bytes for entire packet
        ALOGD("btsco_process_RX_MSBC() wrong header, packet invalid!!!");
    }

    if (status == MSBC_BTSTREAM_FRAME_BYTE)
    {
        if (mBTSCOCVSDContext->pRX->PacketValid[index1] == 1 && mBTSCOCVSDContext->pRX->PacketValid[index2] == 0)
        {
            dwBtEv3HalfBad = 1;
        }
        else
        {
            dwBtEv3HalfBad = 0;
        }

        ALOGD("btsco_process_RX_MSBC() dwBtEv3HalfBad = %d", dwBtEv3HalfBad);
    }

    if (mBTSCOCVSDContext->pRX->fEnablePLC)
    {
        if ((status == MSBC_BTSTREAM_FRAME_BYTE) && (mBTSCOCVSDContext->pRX->PacketValid[index1] == 1) && (mBTSCOCVSDContext->pRX->PacketValid[index2] == 1))
        {
            //packet not lost
            g711plc_addtohistory_v2(mBTSCOCVSDContext->pRX->pPLCHandle, (short *)workbuf, 0);
        }
        else
        {
            //packet lost
            ALOGD("btsco_process_RX_MSBC(), packet lost, do PLC!!!");
            g711plc_dofe_v2(mBTSCOCVSDContext->pRX->pPLCHandle, (short *)workbuf, dwBtEv3HalfBad);
        }
    }

    if (mBTSCOCVSDContext->pRX->fEnableFilter)
    {
        ALOGD("btsco_process_RX_MSBC() fEnableFilter");
        //do filter
        int32_t iInSampleTemp = iOutSample;
        int32_t iOutSampleTemp = iOutSample;
        Audio_IIRHPF_Process(mBTSCOCVSDContext->pRX->pHPFHandle, (uint16_t *)workbuf, (uint16_t *)&iInSampleTemp, (uint16_t *)workbuf, (uint16_t *)&iOutSampleTemp);
        iOutSample = iOutSampleTemp;
    }

    if (mExtMDBTSCORunning == true) //SRC from 16K to 8K
    {
        uint32_t uOutByte = 0, uInByte = 0, uConsumeByte = 0;
        uInByte = iOutSample << 1; //should be MSBC_PCM_FRAME_BYTE!!!
        uOutByte = *outsize >> 1; //MSBC_PCM_FRAME_BYTE/2

        ALOGVV("btsco_process_RX_MSBC() BLI_Convert(+) uInByte=%d, uOutByte=%d", uInByte, uOutByte);

        uConsumeByte = uInByte;
        mBTSCOCVSDContext->pRX->pSRCHandle_1->Process((int16_t *)workbuf, &uInByte, (int16_t *)outbuf, &uOutByte);
        uConsumeByte -= uInByte;

        ALOGVV("btsco_process_RX_MSBC() BLI_Convert(-) remaining=%d, uConsumeByte=%d, uOutByte=%d", uInByte, uConsumeByte, uOutByte);
        ASSERT(uConsumeByte == (iOutSample << 1));
        *outsize = uOutByte; //MSBC_PCM_FRAME_BYTE/2 because of 16kto8k SRC
    }
    else
    {
        memcpy(outbuf, workbuf, iOutSample << 1);
        //*insize = iInByte;
        *outsize = iOutSample << 1;
    }

    ALOGD("btsco_process_RX_MSBC() consumed=%d, *outsize=%d", *insize, *outsize);
}

#endif

/*
btsco_process_TX_CVSD(void *inbuf, uint32 *insize, void *outbuf, uint32 *outsize, void *workbuf, const uint32 workbufsize, uint32 src_fs_s)

void *inbuf                                         : inbuf of pcm data(for SRC to 64k)
uint32 *insize                                     : in: inbuf length of pcm data(for SRC to 64k)
                                    out: consumed inbuf length of pcm data (i.e. SRC consumed buf length)
void *outbuf                                      : outbuf of CVSD encoded bitstream
uint32 *outsize                                 : in: desired output length of CVSD encoded bitstream (i.e. SCO_TX_ENCODE_SIZE)
                                                        out: practical output length of CVSD encoded bitstream
void *workbuf                                  : workingbuf during CVSD encode (i.e. PcmBuf_64k)
const uint32 workbufsize              : workingbuf size during CVSD encode (i.e. SCO_RX_PCM64K_BUF_SIZE)
unint32 src_fs_s                               : SRC source sample rate(for SRC to 64k)
*/

void AudioBTCVSDControl::btsco_process_TX_CVSD(void *inbuf, uint32_t *insize, void *outbuf, uint32_t *outsize, void *workbuf, const uint32_t workbufsize, uint32_t src_fs_s)
{
    uint32_t src_outsize, iInSample, iOutSample, iOutByte, uConsumeByte, i;

    if (*insize != 0 && *outsize != 0)
    {
        src_outsize = workbufsize;
        ALOGVV("btsco_process_TX_CVSD() BLI_Convert *insize=%d, *outsize=%d", *insize, *outsize);

        uConsumeByte = *insize;
        mBTSCOCVSDContext->pTX->pSRCHandle->Process((int16_t *)inbuf, insize, (int16_t *)workbuf, &src_outsize);
        uConsumeByte -= *insize;

        ALOGVV("btsco_process_TX_CVSD() BLI_Convert consumed=%d, remaining=%d, src_outsize=%d", uConsumeByte, *insize, src_outsize);

        if (mTXSRCPCMDumpFile)
        {
            fwrite((void *)workbuf, 1, src_outsize, mTXSRCPCMDumpFile);
        }

        *insize = uConsumeByte;
        iInSample = src_outsize >> 1;

        if (mBTSCOCVSDContext->pTX->fEnableFilter)
        {
            //filter
            iInSample = src_outsize >> 1;
            iOutSample = src_outsize >> 1;
            Audio_IIRHPF_Process(mBTSCOCVSDContext->pTX->pHPFHandle, (uint16_t *)workbuf, (uint16_t *)&iInSample, (uint16_t *)workbuf, (uint16_t *)&iOutSample);
            iInSample = iOutSample;
        }
        //encode
        iOutByte = SCO_TX_ENCODE_SIZE;
        //uint32_t iGain = 0x7FFF;

        CVSD_ENC_Process(mBTSCOCVSDContext->pTX->pEncHandle, (short *)workbuf, (int *)&iInSample, (char *)outbuf, (int *)&iOutByte);
        ALOGVV("CVSD_ENC_Process BLI_Convert iInSample=%d, iOutByte=%d", iInSample, iOutByte);

#ifdef TXOUT_RXIN_TEST // use sequence number to replace ENC out
        for (i = 0; i < iOutByte; i++)
        {
            *((char *)outbuf + i) = i;
        }
#endif

        *outsize = iOutByte;
    }
    else
    {
        *insize = 0;
        *outsize = 0;
    }
}

/*
btsco_process_TX_MSBC(void *inbuf, uint32 *insize, void *outbuf, uint32 *outsize, void *workbuf, const uint32 workbufsize, uint32 src_fs_s)

void *inbuf                                         : inbuf of pcm data
uint32 *insize                                     : in: inbuf length of pcm data(for SRC to 64k)
                                    out: consumed inbuf length of pcm data (i.e. SRC consumed buf length)
void *outbuf                                      : outbuf of CVSD encoded bitstream
uint32 *outsize                                 : in: desired output length of mSBC encoded bitstream
                                                        out: practical output length of mSBC encoded bitstream
void *workbuf                                  : workingbuf during CVSD encode (i.e. PcmBuf_64k)
const uint32 workbufsize              : workingbuf size during mSBC encode
unint32 src_fs_s                               : SRC source sample rate
*/

#if defined(__MSBC_CODEC_SUPPORT__)

static const uint8_t btsco_MsbcHeader[4] = {0x08, 0x38, 0xc8, 0xf8};

void AudioBTCVSDControl::btsco_process_TX_MSBC(void *inbuf, uint32_t *insize, void *outbuf, uint32_t *outsize, void *workbuf, const uint32_t workbufsize, uint32_t src_fs_s)
{
    uint32_t src_outsize, iInSample, iOutSample, iOutByte, uConsumeByte, i, status, index;

    if (*insize != 0 && *outsize != 0)
    {
        src_outsize = MSBC_PCM_FRAME_BYTE;
        uConsumeByte = *insize;
        mBTSCOCVSDContext->pTX->pSRCHandle->Process((int16_t *)inbuf, insize, (int16_t *)workbuf, &src_outsize);
        uConsumeByte -= *insize;

        ALOGD("btsco_process_TX_MSBC() BLI_Convert consumed=%d, remaining=%d, src_outsize=%d", uConsumeByte, *insize, src_outsize);

        if (mTXSRCPCMDumpFile)
        {
            fwrite((void *)workbuf, 1, src_outsize, mTXSRCPCMDumpFile);
        }

        *insize = uConsumeByte;

        char *pOutput8 = (char *)outbuf;

        //*insize = MSBC_PCM_FRAME_BYTE;
        //iInSample = MSBC_PCM_FRAME_BYTE >> 1;
        iInSample = src_outsize >> 1;

        if (mBTSCOCVSDContext->pTX->fEnableFilter)
        {
            //filter
            iInSample = src_outsize >> 1;
            iOutSample = src_outsize >> 1;
            Audio_IIRHPF_Process(mBTSCOCVSDContext->pTX->pHPFHandle, (uint16_t *)workbuf, (uint16_t *)&iInSample, (uint16_t *)workbuf, (uint16_t *)&iOutSample);
            ASSERT(iInSample == iOutSample);
        }
        else
        {
            //memcpy((uint16_t *)workbuf, (uint16_t *)inbuf, MSBC_PCM_FRAME_BYTE);
        }

        //encode
        iOutByte = MSBC_BTSTREAM_FRAME_BYTE;

        status = MSBC_ENC_Process(mBTSCOCVSDContext->pTX->pEncHandle, (short *)workbuf, (int *)&iInSample, pOutput8 + 2, (int *)&iOutByte); //out 57 bytes
        ALOGD("MSBC_ENC_Process iInSample=%d, iOutByte=%d, status=%d", iInSample, iOutByte, status);
        ASSERT(iOutByte == MSBC_BTSTREAM_FRAME_BYTE);

        index = mBTSCOCVSDContext->pTX->iPacket_w;
        pOutput8[0 ] = 0x01; //header
        pOutput8[1 ] = btsco_MsbcHeader[index & 0x3]; //header
        pOutput8[59] = 0; //header
        mBTSCOCVSDContext->pTX->iPacket_w++;

#ifdef TXOUT_RXIN_TEST // use sequence number to replace ENC out
        for (i = 0; i < iOutByte; i++)
        {
            *((char *)outbuf + i) = i;
        }
#endif

        *outsize = SCO_TX_ENCODE_SIZE;
    }
    else
    {
        *insize = 0;
        *outsize = 0;
    }
}

#endif

void AudioBTCVSDControl::BT_SCO_RX_Open(void)
{
    uint32 uRxMemSize = 0;
    char *pAllocMemory;

    ALOGD("BT_SCO_RX_Open(+) mBTSCOCVSDContext->uRXState=%d", mBTSCOCVSDContext->uRXState);

    ASSERT(mBTSCOCVSDContext->uRXState == BT_SCO_RXSTATE_IDLE);

    uRxMemSize += (sizeof(BT_SCO_RX) + 3)& ~0x3;

    mBTSCOCVSDContext->pRX = (BT_SCO_RX *)new char[uRxMemSize];
    ASSERT(mBTSCOCVSDContext->pRX);
    memset((void *)mBTSCOCVSDContext->pRX, 0, uRxMemSize);

    mBTCVSDRXTempInBuf = (uint8_t *)new char[BTSCO_CVSD_RX_TEMPINPUTBUF_SIZE];
    mBTCVSDRXInBuf = (uint8_t *)new char[BTSCO_CVSD_RX_INBUF_SIZE];

    mBTCVSDRXDumpFile = NULL;
    mBTCVSDRXDumpFile = AudioOpendumpPCMFile("/sdcard/mtklog/audio_dump/CVSDRXOut.pcm", "CVSDRXOut.pcm.dump");

    ALOGD("BT_SCO_RX_Open(-)");
}

void AudioBTCVSDControl::BT_SCO_RX_Close(void)
{
    ALOGD("BT_SCO_RX_Close(+)");

    if (mBTCVSDRXTempInBuf)
    {
        delete []mBTCVSDRXTempInBuf;
        mBTCVSDRXTempInBuf = NULL;
        ALOGD("BT_SCO_RX_Close() release mBTCVSDRXTempInBuf");
    }

    if (mBTCVSDRXInBuf)
    {
        delete []mBTCVSDRXInBuf;
        mBTCVSDRXInBuf = NULL;
        ALOGD("BT_SCO_RX_Close() release mBTCVSDRXInBuf");
    }

    if (mBTSCOCVSDContext->pRX)
    {
        delete []mBTSCOCVSDContext->pRX;
        mBTSCOCVSDContext->pRX = NULL;
        ALOGD("BT_SCO_RX_Close(-) release mBTSCOCVSDContext->pRX");
    }

    if (mBTCVSDRXDumpFile)
    {
        AudioCloseDumpPCMFile(mBTCVSDRXDumpFile);
        ALOGD("ClosePcmDumpFile mBTCVSDRXDumpFile");
    }

    ALOGD("BT_SCO_RX_Close(-)");
}

void AudioBTCVSDControl::BT_SCO_RX_Begin(int mFd2)
{
    BT_SCO_RX_Open();
    ::ioctl(mFd2, ALLOCATE_FREE_BTCVSD_BUF, 2); //allocate RX working buffers in kernel
    BT_SCO_SET_RXState(BT_SCO_RXSTATE_INIT);
    BT_SCO_RX_SetHandle(NULL, NULL, BTSCO_CVSD_PLC_SAMPLERATE, BTSCO_CVSD_CHANNEL_NUM, 0);
    BT_SCO_SET_RXState(BT_SCO_RXSTATE_READY);
    BT_SCO_RX_Start();
    BT_SCO_SET_RXState(BT_SCO_RXSTATE_RUNNING);
    ::ioctl(mFd2, SET_BTCVSD_STATE, BT_SCO_RXSTATE_RUNNING); //set state to kernel
}

void AudioBTCVSDControl::BT_SCO_RX_End(int mFd2)
{
    BT_SCO_RX_Stop();
    BT_SCO_SET_RXState(BT_SCO_RXSTATE_ENDING);
    ::ioctl(mFd2, SET_BTCVSD_STATE, BT_SCO_RXSTATE_ENDING); //set kernel state
    BT_SCO_RX_Close();
    BT_SCO_SET_RXState(BT_SCO_RXSTATE_IDLE);
    ::ioctl(mFd2, SET_BTCVSD_STATE, BT_SCO_RXSTATE_IDLE);
    ::ioctl(mFd2, ALLOCATE_FREE_BTCVSD_BUF, 3); //free RX working buffers in kernel
}

uint8_t *AudioBTCVSDControl::BT_SCO_RX_GetCVSDOutBuf(void)
{
    return mBTSCOCVSDContext->pRX->PcmBuf_8k;
}

uint8_t *AudioBTCVSDControl::BT_SCO_RX_GetMSBCOutBuf(void)
{
    return mBTSCOCVSDContext->pRX->PcmBuf_mSBC;
}

uint8_t *AudioBTCVSDControl::BT_SCO_RX_GetCVSDWorkBuf(void)
{
    return mBTSCOCVSDContext->pRX->PcmBuf_64k;
}

void AudioBTCVSDControl::btsco_AllocMemory_RX_CVSD(void)
{
    uint32_t uTotalMemory = 0;
    uint8_t  *pBuf = NULL;
    ALOGD("btsco_AllocMemory_RX_CVSD(+)");
    ASSERT(mBTSCOCVSDContext->uRXState == BT_SCO_RXSTATE_READY);

    if (mBTSCOCVSDContext->pRX)
    {
        //uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_PCM_RINGBUF_RX);
        uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_CVSD_DECODE);
        uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_CVSD_RX_SRC1);
        if (mBTSCOCVSDContext->pRX->fEnableFilter)
        {
            uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_FILTER_RX);
        }
        if (mBTSCOCVSDContext->pRX->fEnablePLC)
        {
            uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_PLC_NB);
        }
        if (mBTSCOCVSDContext->pRX->fEnableSRC2)
        {
            uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_CVSD_RX_SRC2);
        }
    }

    pBuf = new uint8_t[uTotalMemory];
    mBTSCOCVSDContext->pRXWorkingMemory = pBuf;
    ASSERT(mBTSCOCVSDContext->pRXWorkingMemory);

    if (mBTSCOCVSDContext->pRX)
    {
        BT_SCO_InitialModule(BT_SCO_MOD_CVSD_DECODE, pBuf);
        pBuf += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_CVSD_DECODE);
        BT_SCO_InitialModule(BT_SCO_MOD_CVSD_RX_SRC1, pBuf);
        pBuf += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_CVSD_RX_SRC1);
        if (mBTSCOCVSDContext->pRX->fEnableFilter)
        {
            BT_SCO_InitialModule(BT_SCO_MOD_FILTER_RX, pBuf);
            pBuf += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_FILTER_RX);
        }
        if (mBTSCOCVSDContext->pRX->fEnablePLC)
        {
            BT_SCO_InitialModule(BT_SCO_MOD_PLC_NB, pBuf);
            pBuf += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_PLC_NB);
        }
        if (mBTSCOCVSDContext->pRX->fEnableSRC2)
        {
            BT_SCO_InitialModule(BT_SCO_MOD_CVSD_RX_SRC2, pBuf);
            pBuf += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_CVSD_RX_SRC2);
        }
    }
}

#if defined(__MSBC_CODEC_SUPPORT__)
void AudioBTCVSDControl::btsco_AllocMemory_RX_MSBC(void)
{
    uint32_t uTotalMemory = 0;
    uint8_t  *pBuf = NULL;

    ASSERT(mBTSCOCVSDContext->uRXState == BT_SCO_RXSTATE_READY);

    if (mBTSCOCVSDContext->pRX)
    {
        uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_MSBC_DECODE);
        uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_MSBC_RX_SRC);
        if (mBTSCOCVSDContext->pRX->fEnableFilter)
        {
            uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_FILTER_RX);
        }
        if (mBTSCOCVSDContext->pRX->fEnablePLC)
        {
            uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_PLC_WB);
        }
        if (mBTSCOCVSDContext->pRX->fEnableSRC2)
        {
            uTotalMemory += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_CVSD_RX_SRC2);
        }
    }

    pBuf = new uint8_t[uTotalMemory];
    mBTSCOCVSDContext->pRXWorkingMemory = pBuf;
    ASSERT(mBTSCOCVSDContext->pRXWorkingMemory);

    if (mBTSCOCVSDContext->pRX)
    {
        BT_SCO_InitialModule(BT_SCO_MOD_MSBC_DECODE, pBuf);
        pBuf += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_MSBC_DECODE);
        BT_SCO_InitialModule(BT_SCO_MOD_MSBC_RX_SRC, pBuf);
        pBuf += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_MSBC_RX_SRC);
        if (mBTSCOCVSDContext->pRX->fEnableFilter)
        {
            BT_SCO_InitialModule(BT_SCO_MOD_FILTER_RX, pBuf);
            pBuf += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_FILTER_RX);
        }
        if (mBTSCOCVSDContext->pRX->fEnablePLC)
        {
            BT_SCO_InitialModule(BT_SCO_MOD_PLC_WB, pBuf);
            pBuf += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_PLC_WB);
        }
        if (mBTSCOCVSDContext->pRX->fEnableSRC2)
        {
            BT_SCO_InitialModule(BT_SCO_MOD_CVSD_RX_SRC2, pBuf);
            pBuf += BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MOD_CVSD_RX_SRC2);
        }
    }
}
#endif

void AudioBTCVSDControl::BT_SCO_RX_Start(void)
{
    ALOGD("BT_SCO_RX_Start(+)");

    mBTSCOCVSDContext->fIsWideBand = BTmode;

    if (mBTSCOCVSDContext->fIsWideBand)
    {
#if defined(__MSBC_CODEC_SUPPORT__)
        btsco_AllocMemory_RX_MSBC();
#else
        ASSERT(0);
#endif
    }
    else
    {
        btsco_AllocMemory_RX_CVSD();
    }

    ALOGD("BT_SCO_RX_Start(-)");
}

void AudioBTCVSDControl::BT_SCO_RX_Stop(void)
{
    ALOGD("BT_SCO_RX_Stop(+)");

    BT_SCO_RX_DestroyModule();

    if (mBTSCOCVSDContext->pRXWorkingMemory)
    {
        delete []mBTSCOCVSDContext->pRXWorkingMemory;
        mBTSCOCVSDContext->pRXWorkingMemory = NULL;
    }
    ALOGD("BT_SCO_RX_Stop(-)");
}

uint8_t *AudioBTCVSDControl::BT_SCO_RX_GetCVSDTempInBuf(void)
{
    return mBTCVSDRXTempInBuf;
}

void AudioBTCVSDControl::BT_SCO_RX_SetCVSDTempInBuf(uint8_t *addr)
{
    mBTCVSDRXTempInBuf = addr;
}


uint8_t *AudioBTCVSDControl::BT_SCO_RX_GetCVSDInBuf(void)
{
    return mBTCVSDRXInBuf;
}

void AudioBTCVSDControl::BT_SCO_RX_SetCVSDInBuf(uint8_t *addr)
{
    mBTCVSDRXInBuf = addr;
}

uint8_t *AudioBTCVSDControl::BT_SCO_TX_GetCVSDOutBuf(void)
{
    return mBTCVSDTXOutBuf;
}

void AudioBTCVSDControl::BT_SCO_TX_SetCVSDOutBuf(uint8_t *addr)
{
    mBTCVSDTXOutBuf = addr;
}

uint8_t *AudioBTCVSDControl::BT_SCO_TX_GetCVSDWorkBuf(void)
{
    return mBTSCOCVSDContext->pTX->PcmBuf_64k;
}

uint32_t AudioBTCVSDControl::Audio_IIRHPF_GetBufferSize(int a)
{
    return 1024;
}

void *AudioBTCVSDControl::Audio_IIRHPF_Init(int8_t *pBuf, const int32_t *btsco_FilterCoeff_8K, int a)
{
    return NULL;
}

void AudioBTCVSDControl::Audio_IIRHPF_Process(void *handle, uint16_t *inbuf, uint16_t *InSample, uint16_t *outbuf, uint16_t *OutSample)
{

}

#ifdef EXT_MODEM_BT_CVSD

void AudioBTCVSDControl::BT_SCO_ExtMD_ULBuf_Open(void)
{
    ALOGD("BT_SCO_ExtMD_ULBuf_Open(+)");
    mExtMDbtscoULBuf = (uint8_t *)new char[BTSCO_CVSD_RX_INBUF_SIZE * 2 * 2];
    memset(mExtMDbtscoULBuf, 0, BTSCO_CVSD_RX_INBUF_SIZE * 2 * 2);
    mExtMDbtscoULWTmpBuf = (uint8_t *)new char[BTSCO_CVSD_RX_INBUF_SIZE * 2];
    memset(mExtMDbtscoULWTmpBuf, 0, BTSCO_CVSD_RX_INBUF_SIZE * 2);
    mExtMDbtscoULWTmpBuf2 = (uint8_t *)new char[BTSCO_CVSD_RX_INBUF_SIZE * 2 * 2]; //for duplicate 1ch to 2ch to AFE DL1 mem intf
    memset(mExtMDbtscoULWTmpBuf2, 0, BTSCO_CVSD_RX_INBUF_SIZE * 2 * 2);
    ALOGD("BT_SCO_ExtMD_ULBuf_Open(-)");
}

void AudioBTCVSDControl::BT_SCO_ExtMD_ULBuf_Close(void)
{
    if (mExtMDbtscoULBuf)
    {
        delete []mExtMDbtscoULBuf;
        mExtMDbtscoULBuf = NULL;
        ALOGD("BT_SCO_ExtMD_ULBuf_Close() release mExtMDbtscoULBuf");
    }

    if (mExtMDbtscoULWTmpBuf)
    {
        delete []mExtMDbtscoULWTmpBuf;
        mExtMDbtscoULWTmpBuf = NULL;
        ALOGD("BT_SCO_ExtMD_ULBuf_Close() release mExtMDbtscoULWTmpBuf");
    }

    if (mExtMDbtscoULWTmpBuf2)
    {
        delete []mExtMDbtscoULWTmpBuf2;
        mExtMDbtscoULWTmpBuf2 = NULL;
        ALOGD("BT_SCO_ExtMD_ULBuf_Close() release mExtMDbtscoULWTmpBuf2");
    }
}

void AudioBTCVSDControl::BT_SCO_ExtMD_DLBuf_Open(void)
{
    ALOGD("BT_SCO_ExtMD_DLBuf_Open(+)");
    mExtMDbtscoDLBuf = (uint8_t *)new char[BTSCO_CVSD_TX_OUTBUF_SIZE * 2 * 2];
    memset(mExtMDbtscoDLBuf, 0, BTSCO_CVSD_TX_OUTBUF_SIZE * 2 * 2);
    ALOGD("BT_SCO_ExtMD_DLBuf_Open(-)");
}

void AudioBTCVSDControl::BT_SCO_ExtMD_DLBuf_Close(void)
{
    if (mExtMDbtscoDLBuf)
    {
        delete []mExtMDbtscoDLBuf;
        mExtMDbtscoDLBuf = NULL;
        ALOGD("BT_SCO_ExtMD_DLBuf_Close() release mExtMDbtscoDLBuf");
    }
}

bool AudioBTCVSDControl::BT_SCO_ExtMDULBufLock(void)
{
    ALOGV("BT_SCO_ExtMDULBufLock (1)");
    mLockUL.lock();
    ALOGV("BT_SCO_ExtMDULBufLock (2)");
    return true;
}

bool AudioBTCVSDControl::BT_SCO_ExtMDULBufUnLock(void)
{
    ALOGV("BT_SCO_ExtMDULBufUnLock (1)");
    mLockUL.unlock();
    ALOGV("BT_SCO_ExtMDULBufUnLock (2)");
    return true;
}

bool AudioBTCVSDControl::BT_SCO_ExtMDDLBufLock(void)
{
    ALOGV("BT_SCO_ExtMDDLBufLock (1)");
    mLockDL.lock();
    ALOGV("BT_SCO_ExtMDDLBufLock (2)");
    return true;
}

bool AudioBTCVSDControl::BT_SCO_ExtMDDLBufUnLock(void)
{
    ALOGV("BT_SCO_ExtMDDLBufUnLock (1)");
    mLockDL.unlock();
    ALOGV("BT_SCO_ExtMDDLBufUnLock (2)");
    return true;
}

void AudioBTCVSDControl::BT_SCO_ExtMDInitBuf(EXTMD_BTSCO_DIRECTION direction)
{
    if (direction == ExtMD_BTSCO_UL)
    {
        mULRingBuf.pBufBase = (char *)mExtMDbtscoULBuf;
        //if (mBTSCOCVSDContext->fIsWideBand == true)
        if (0)
        {
            mULRingBuf.bufLen = BTSCO_CVSD_RX_INBUF_SIZE * 2 * 2 * 2;
        }
        else
        {
            mULRingBuf.bufLen = BTSCO_CVSD_RX_INBUF_SIZE * 2 * 2;
        }
        mULRingBuf.pRead = mULRingBuf.pBufBase + (mULRingBuf.bufLen >> 1) - 2;
        mULRingBuf.pWrite = mULRingBuf.pBufBase;
    }
    else if (direction == ExtMD_BTSCO_DL)
    {
        mDLRingBuf.pBufBase = (char *)mExtMDbtscoDLBuf;
        //if (mBTSCOCVSDContext->fIsWideBand == true)
        if (0)
        {
            mDLRingBuf.bufLen = BTSCO_CVSD_TX_OUTBUF_SIZE * 2 * 2 * 2;
        }
        else
        {
            mDLRingBuf.bufLen = BTSCO_CVSD_TX_OUTBUF_SIZE * 2 * 2;
        }
        mDLRingBuf.pRead = mDLRingBuf.pBufBase + (mDLRingBuf.bufLen >> 1) - 2;
        mDLRingBuf.pWrite = mDLRingBuf.pBufBase;
    }
}
uint32_t AudioBTCVSDControl::BT_SCO_ExtMDGetBufSpace(EXTMD_BTSCO_DIRECTION direction)
{
    int count = 0;

    if (direction == ExtMD_BTSCO_UL)
    {
        count = mULRingBuf.pRead - mULRingBuf.pWrite;
        if (count < 0) { count += mULRingBuf.bufLen; }
    }
    else if (direction == ExtMD_BTSCO_DL)
    {
        count = mDLRingBuf.pRead - mDLRingBuf.pWrite;
        if (count < 0) { count += mDLRingBuf.bufLen; }
    }

    return count;
}

uint32_t AudioBTCVSDControl::BT_SCO_ExtMDGetBufCount(EXTMD_BTSCO_DIRECTION direction)
{
    int count = 0;

    if (direction == ExtMD_BTSCO_UL)
    {
        count = mULRingBuf.pWrite - mULRingBuf.pRead;
        if (count <= 0) { count += mULRingBuf.bufLen; }
    }
    else if (direction == ExtMD_BTSCO_DL)
    {
        count = mDLRingBuf.pWrite - mDLRingBuf.pRead;
        if (count <= 0) { count += mDLRingBuf.bufLen; }
    }

    return count;
}

void AudioBTCVSDControl::BT_SCO_ExtMDWriteDataToRingBuf(uint8_t *buf, uint32_t size, EXTMD_BTSCO_DIRECTION direction)
{
    RingBuf *pRingBuf = NULL;

    if (direction == ExtMD_BTSCO_UL)
    {
        pRingBuf = &mULRingBuf;
    }
    else if (direction == ExtMD_BTSCO_DL)
    {
        pRingBuf = &mDLRingBuf;
    }

    ASSERT(pRingBuf != NULL);

    uint32_t end = (uint32_t)(pRingBuf->pBufBase + pRingBuf->bufLen);
    uint32_t size1, size2;
    ALOGD("BT_SCO_ExtMDWriteDataToRingBuf end=0x%x,pRingBuf->pWrite=0x%x, size=%d, direction=%d", end, pRingBuf->pWrite, size, direction);
    if (size <= (end - (uint32_t)pRingBuf->pWrite)) //copy once
    {
        memcpy(pRingBuf->pWrite, buf, size);
        pRingBuf->pWrite += size;

        if ((uint32_t)pRingBuf->pWrite >= end)
        {
            pRingBuf->pWrite -= pRingBuf->bufLen;
        }
    }
    else
    {
        size1 = end - (uint32_t)pRingBuf->pWrite;
        size2 = size - size1;
        memcpy(pRingBuf->pWrite, buf, size1);
        memcpy(pRingBuf->pBufBase, buf + size1, size2);
        pRingBuf->pWrite = pRingBuf->pBufBase + size2;
    }
}

void AudioBTCVSDControl::BT_SCO_ExtMDReadDataFromRingBuf(uint8_t *buf, uint32_t size, EXTMD_BTSCO_DIRECTION direction)
{
    RingBuf *pRingBuf = NULL;

    if (direction == ExtMD_BTSCO_UL)
    {
        pRingBuf = &mULRingBuf;
    }
    else if (direction == ExtMD_BTSCO_DL)
    {
        pRingBuf = &mDLRingBuf;
    }

    ASSERT(pRingBuf != NULL);

    uint32_t end = (uint32_t)(pRingBuf->pBufBase + pRingBuf->bufLen);
    uint32_t size1, size2;
    ALOGD("BT_SCO_ExtMDReadDataFromRingBuf end=0x%x,pRingBuf->pRead=0x%x, size=%d,direction=%d", end, pRingBuf->pRead, size, direction);

    if (size <= (end - (uint32_t)pRingBuf->pRead)) //copy once
    {
        memcpy(buf, pRingBuf->pRead, size);
        pRingBuf->pRead += size;

        if ((uint32_t)pRingBuf->pRead >= end)
        {
            pRingBuf->pRead -= pRingBuf->bufLen;
        }
    }
    else
    {
        size1 = end - (uint32_t)pRingBuf->pRead;
        size2 = size - size1;
        memcpy(buf, pRingBuf->pRead, size1);
        memcpy(buf + size1, pRingBuf->pBufBase, size2);
        pRingBuf->pRead = pRingBuf->pBufBase + size2;
    }
}


uint8_t *AudioBTCVSDControl::BT_SCO_ExtMDGetCVSDAccuOutBuf(void)
{
    return mBTSCOCVSDContext->pRX->PcmBuf_8k_accu;
}

uint8_t *AudioBTCVSDControl::BT_SCO_ExtMDGetCVSDULWriteTmpBuf(void)
{
    return mExtMDbtscoULWTmpBuf;
}

uint8_t *AudioBTCVSDControl::BT_SCO_ExtMDGetCVSDULWriteTmpBuf2(void)
{
    return mExtMDbtscoULWTmpBuf2;
}

void AudioBTCVSDControl::AudioExtMDCVSDCreateThread(void)
{
    mExtMDBTSCORunning = true;
    ALOGD("mExtMDBTSCORunning = %d", mExtMDBTSCORunning);

    mExtMDCVSDULThread1 = new AudioExtMDCVSDThread(ExtMD_BTSCO_UL_READTHREAD , NULL, 0);
    if (mExtMDCVSDULThread1.get())
    {
        mExtMDCVSDULThread1->run();
    }

    mExtMDCVSDULThread2 = new AudioExtMDCVSDThread(ExtMD_BTSCO_UL_WRITETHREAD , NULL, 0);
    if (mExtMDCVSDULThread2.get())
    {
        mExtMDCVSDULThread2->run();
    }

    mExtMDCVSDDLThread1 = new AudioExtMDCVSDThread(ExtMD_BTSCO_DL_READTHREAD, NULL, 0);
    if (mExtMDCVSDDLThread1.get())
    {
        mExtMDCVSDDLThread1->run();
    }

    mExtMDCVSDDLThread2 = new AudioExtMDCVSDThread(ExtMD_BTSCO_DL_WRITETHREAD , NULL, 0);
    if (mExtMDCVSDDLThread2.get())
    {
        mExtMDCVSDDLThread2->run();
    }

}

void AudioBTCVSDControl::AudioExtMDCVSDDeleteThread(void)
{
    int ret = 0;

#if 1 // close AFE interconnection first in AudioExtMDCVSDDeleteThread()
    mAudioDigitalControl = AudioDigitalControlFactory::CreateAudioDigitalControl();

    mAudioDigitalControl->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I05, AudioDigitalType::O07);
    mAudioDigitalControl->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I06, AudioDigitalType::O08);

    mAudioDigitalControl->SetinputConnection(AudioDigitalType::DisConnect, AudioDigitalType::I09, AudioDigitalType::O12);
#endif


    if (mExtMDCVSDULThread2.get()) //deleted UL write thread first to prevent DL1 underflow!
    {
        ret = mExtMDCVSDULThread2->requestExitAndWait();
        if (ret == WOULD_BLOCK)
        {
            mExtMDCVSDULThread2->requestExit();
        }
        mExtMDCVSDULThread2.clear();
    }

    ret = 0;
    if (mExtMDCVSDULThread1.get()) //delete UL read thread
    {
        ret = mExtMDCVSDULThread1->requestExitAndWait();
        if (ret == WOULD_BLOCK)
        {
            mExtMDCVSDULThread1->requestExit();
        }
        mExtMDCVSDULThread1.clear();
    }

    ret = 0;
    if (mExtMDCVSDDLThread2.get()) //delete DL write thread
    {
        ret = mExtMDCVSDDLThread2->requestExitAndWait();
        if (ret == WOULD_BLOCK)
        {
            mExtMDCVSDDLThread2->requestExit();
        }
        mExtMDCVSDDLThread2.clear();
    }

    ret = 0;
    if (mExtMDCVSDDLThread1.get()) //delete DL read thread
    {
        ret = mExtMDCVSDDLThread1->requestExitAndWait();
        if (ret == WOULD_BLOCK)
        {
            mExtMDCVSDDLThread1->requestExit();
        }
        mExtMDCVSDDLThread1.clear();
    }

    mExtMDBTSCORunning = false;
    ALOGD("mExtMDBTSCORunning = %d", mExtMDBTSCORunning);
}

AudioBTCVSDControl::AudioExtMDCVSDThread::AudioExtMDCVSDThread(EXTMD_BTSCO_THREAD_TYPE Thread_type, char *RingBuffer, uint32 BufferSize)
{
    ALOGD("AudioExtMDCVSDThread constructor Thread_type = %d", Thread_type);
    mFd = 0;
    mFd2 = 0;
    mThreadType = Thread_type;

    mAudioBTCVSDControl = AudioBTCVSDControl::getInstance();
    if (!mAudioBTCVSDControl)
    {
        ALOGE("AudioBTCVSDControl::getInstance() fail");
    }

    switch (mThreadType)
    {
        case ExtMD_BTSCO_UL_READTHREAD:
            if (mFd2 == 0)
            {
                mFd2 =  ::open(kBTDeviceName, O_RDWR);
                if (mFd2 <= 0)
                {
                    ALOGW("open BTCVSD kernel device fail");
                }
            }
            mName = String8("ExtMDCVSDULReadThread");
            mExtMDULReadPCMDumpFile = NULL;
            mExtMDULReadPCMDumpFile = AudioOpendumpPCMFile("/sdcard/mtklog/audio_dump/extmd_ul_read.pcm", "extmd_ul_read.dump");

            mAudioBTCVSDControl->BT_SCO_RX_Begin(mFd2);
            mAudioBTCVSDControl->BT_SCO_ExtMD_ULBuf_Open();
            mAudioBTCVSDControl->BT_SCO_ExtMDInitBuf(ExtMD_BTSCO_UL); //init ULBuf bufbase/read write pointer
            break;
        case ExtMD_BTSCO_UL_WRITETHREAD:
            if (mFd == 0)
            {
                mFd =  ::open(kAudioDeviceName, O_RDWR);
                if (mFd <= 0)
                {
                    ALOGW("open AFE kernel device fail");
                }
            }
            mName = String8("ExtMDCVSDULWriteThread");
            mExtMDULWritePCMDumpFile = NULL;
            mExtMDULWritePCMDumpFile = AudioOpendumpPCMFile("/sdcard/mtklog/audio_dump/extmd_ul_write.pcm", "extmd_ul_write.dump");

            mAFEULStarting = false;
            mAudioResourceManager = AudioResourceManager::getInstance();
            mAudioDigitalControl = AudioDigitalControlFactory::CreateAudioDigitalControl();

            mAudioDigitalControl->FreeMemBufferSize(AudioDigitalType::MEM_DL1); // default DL1 buf size is 16k, too big... Need to reallocate
            //if (mBTSCOCVSDContext->fIsWideBand == true)
            if (0)
            {
                mAudioDigitalControl->SetMemBufferSize(AudioDigitalType::MEM_DL1, BTSCO_CVSD_RX_INBUF_SIZE * 2 * 2 * 2 * 2); //480*2*2*2*2, 2ch
#ifndef EXTMD_SUPPORT_WB
                pULSRCHandle = new MtkAudioSrc(BTSCO_MSBC_SAMPLERATE_DOMAIN, BTSCO_MSBC_CHANNEL_NUM, BTSCO_EXTMD_SAMPLERATE, BTSCO_MSBC_CHANNEL_NUM, SRC_IN_Q1P15_OUT_Q1P15);
                pULSRCHandle->Open();
#endif
            }
            else
            {
                mAudioDigitalControl->SetMemBufferSize(AudioDigitalType::MEM_DL1, BTSCO_CVSD_RX_INBUF_SIZE * 2 * 2 * 2); //480*2*2*2, 2ch
            }
            mAudioDigitalControl->AllocateMemBufferSize(AudioDigitalType::MEM_DL1);

            ::ioctl(mFd, START_MEMIF_TYPE, AudioDigitalType::MEM_DL1); // fp for write indentify
            break;
        case ExtMD_BTSCO_DL_READTHREAD:
            if (mFd == 0)
            {
                mFd =  ::open(kAudioDeviceName, O_RDWR);
                if (mFd <= 0)
                {
                    ALOGW("open AFE kernel device fail");
                }
            }
            mName = String8("ExtMDCVSDDLReadThread");
            mExtMDDLReadPCMDumpFile = NULL;
            mExtMDDLReadPCMDumpFile = AudioOpendumpPCMFile("/sdcard/mtklog/audio_dump/extmd_dl_read.pcm", "extmd_dl_read.dump");

            mAFEDLStarting = false;
            mAudioBTCVSDControl->BT_SCO_ExtMD_DLBuf_Open();
            mAudioBTCVSDControl->BT_SCO_ExtMDInitBuf(ExtMD_BTSCO_DL); //init ULBuf bufbase/read write pointer

            mAudioResourceManager = AudioResourceManager::getInstance();
            mAudioDigitalControl = AudioDigitalControlFactory::CreateAudioDigitalControl();

            mAudioDigitalControl->FreeMemBufferSize(AudioDigitalType::MEM_MOD_DAI); // default MEM_MOD_DAI buf size is 8k, too big... Need to reallocate

            //if (mBTSCOCVSDContext->fIsWideBand == true)
            if (0)
            {
                mAudioDigitalControl->SetMemBufferSize(AudioDigitalType::MEM_MOD_DAI, BTSCO_CVSD_TX_OUTBUF_SIZE * 2 * 2 * 2); //480*2*2*2
            }
            else
            {
                mAudioDigitalControl->SetMemBufferSize(AudioDigitalType::MEM_MOD_DAI, BTSCO_CVSD_TX_OUTBUF_SIZE * 2 * 2); //480*2*2
            }
            mAudioDigitalControl->AllocateMemBufferSize(AudioDigitalType::MEM_MOD_DAI);

            ::ioctl(mFd, START_MEMIF_TYPE, AudioDigitalType::MEM_MOD_DAI); // fp for write indentify
            break;
        case ExtMD_BTSCO_DL_WRITETHREAD:
            if (mFd2 == 0)
            {
                mFd2 =  ::open(kBTDeviceName, O_RDWR);
                if (mFd2 <= 0)
                {
                    ALOGW("open BTCVSD kernel device fail");
                }
            }
            mName = String8("ExtMDCVSDDLWriteThread");
            mExtMDDLWritePCMDumpFile = NULL;
            mExtMDDLWritePCMDumpFile = AudioOpendumpPCMFile("/sdcard/mtklog/audio_dump/extmd_dl_write.pcm", "extmd_dl_write.dump");
            mAudioBTCVSDControl->BT_SCO_TX_Begin(mFd2, EXTMD_BTSCO_AFE_SAMPLERATE, 1);
            break;
        default:
            ALOGD("unsupport ExtMD_BTSCO_Thread type");
            break;
    }
    // ring buffer to copy data into this ring buffer
    mRingBuffer = RingBuffer;
    mBufferSize = BufferSize;
}

AudioBTCVSDControl::AudioExtMDCVSDThread::~AudioExtMDCVSDThread()
{
#define DL1_BUFFER_SIZE (0x4000)

    ALOGD("+~AudioExtMDCVSDThread()mThreadType=%d", mThreadType);
    ClosePcmDumpFile();

    switch (mThreadType)
    {
        case ExtMD_BTSCO_UL_READTHREAD:
            mAudioBTCVSDControl->BT_SCO_RX_End(mFd2);
            mAudioBTCVSDControl->BT_SCO_ExtMD_ULBuf_Close();
            break;
        case ExtMD_BTSCO_UL_WRITETHREAD:

            mAFEULStarting = false;

            mAudioDigitalControl->SetIrqMcuEnable(AudioDigitalType::IRQ1_MCU_MODE, false);

            mAudioDigitalControl->SetMemIfEnable(AudioDigitalType::MEM_DL1, false);
            //mAudioDigitalControl->SetAfeEnable(false);

            if (mFd)
            {
                ALOGD("threadLoop exit STANDBY_MEMIF_TYPE mThreadType = %d", mThreadType);
                ::ioctl(mFd, STANDBY_MEMIF_TYPE, AudioDigitalType::MEM_DL1); // disable mem interface DL1
            }

#ifdef EXTMD_LOOPBACK_TEST
            //close DAC
            mAudioResourceManager->StopOutputDevice();
#endif
            mAudioDigitalControl->FreeMemBufferSize(AudioDigitalType::MEM_DL1); // revert to default DL1 size
            mAudioDigitalControl->SetMemBufferSize(AudioDigitalType::MEM_DL1, DL1_BUFFER_SIZE);
            mAudioDigitalControl->AllocateMemBufferSize(AudioDigitalType::MEM_DL1);

            //if (mBTSCOCVSDContext->fIsWideBand == true)
            if (0)
            {
#ifndef EXTMD_SUPPORT_WB
                pULSRCHandle->Close();
                delete pULSRCHandle;
                pULSRCHandle = NULL;
#endif
            }

            //ReleasePlaybackclock();
            mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
#ifdef EXTMD_LOOPBACK_TEST
            mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);
#endif

            break;
        case ExtMD_BTSCO_DL_READTHREAD:
            mAFEDLStarting = false;

            mAudioDigitalControl->SetMemIfEnable(AudioDigitalType::MEM_MOD_DAI, false);
            mAudioDigitalControl->SetIrqMcuEnable(AudioDigitalType::IRQ2_MCU_MODE, false);

#ifdef EXTMD_LOOPBACK_TEST
            //close ADC (no need if use sine wave gen to test)
            //mAudioResourceManager->StopInputDevice();
#endif

            if (mFd)
            {
                ALOGD("threadLoop exit STANDBY_MEMIF_TYPE mThreadType = %d", mThreadType);
                ::ioctl(mFd, STANDBY_MEMIF_TYPE, AudioDigitalType::MEM_MOD_DAI);
                ::close(mFd);
                mFd = 0;
            }

#if 0
            // No need to revert to default MEM_MOD_DAI size since it will be reinit each time on start
            mAudioDigitalControl->FreeMemBufferSize(AudioDigitalType::MEM_MOD_DAI);
            mAudioDigitalControl->SetMemBufferSize(AudioDigitalType::MEM_MOD_DAI, DL1_BUFFER_SIZE);
            mAudioDigitalControl->AllocateMemBufferSize(AudioDigitalType::MEM_MOD_DAI);
#endif

            //ReleaseRecordclock();
            mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, false);
#ifdef EXTMD_LOOPBACK_TEST
            //no need if use sine wave gen to test
            //mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, false);
#endif
            mAudioBTCVSDControl->BT_SCO_ExtMD_DLBuf_Close();
            break;
        case ExtMD_BTSCO_DL_WRITETHREAD:
            mAudioBTCVSDControl->BT_SCO_TX_End(mFd2);
            if (mFd2)
            {
                ::close(mFd2);
                mFd2 = 0;
            }
            break;
        default:
            ALOGD("unsupport ExtMD_BTSCO_Thread type");
            break;
    }
    ALOGD("-~AudioExtMDCVSDThread()");
}

void AudioBTCVSDControl::AudioExtMDCVSDThread::onFirstRef()
{
    ALOGD("AudioExtMDCVSDThread onFirstRef");

    run(mName, ANDROID_PRIORITY_URGENT_AUDIO);
}

// Good place to do one-time initializations
status_t  AudioBTCVSDControl::AudioExtMDCVSDThread::readyToRun()
{
    ALOGD("AudioExtMDCVSDThread::readyToRun(),mThreadType=%d", mThreadType);

    return NO_ERROR;
}

void AudioBTCVSDControl::AudioExtMDCVSDThread::WritePcmDumpData(uint8_t *buf, uint32_t size)
{
    int written_data = 0;
    switch (mThreadType)
    {
        case ExtMD_BTSCO_UL_READTHREAD:
            if (mExtMDULReadPCMDumpFile)
            {
                written_data = fwrite((void *)buf, 1, size, mExtMDULReadPCMDumpFile);
            }
            break;
        case ExtMD_BTSCO_UL_WRITETHREAD:
            if (mExtMDULWritePCMDumpFile)
            {
                written_data = fwrite((void *)buf, 1, size, mExtMDULWritePCMDumpFile);
            }
            break;
        case ExtMD_BTSCO_DL_READTHREAD:
            if (mExtMDDLReadPCMDumpFile)
            {
                written_data = fwrite((void *)buf, 1, size, mExtMDDLReadPCMDumpFile);
            }
            break;
        case ExtMD_BTSCO_DL_WRITETHREAD:
            if (mExtMDDLWritePCMDumpFile)
            {
                written_data = fwrite((void *)buf, 1, size, mExtMDDLWritePCMDumpFile);
            }
            break;
        default:
            ALOGW("AudioExtMDCVSDThread::WritePcmDumpData unknown mThreadType!!! ");
            break;
    }
}

void AudioBTCVSDControl::AudioExtMDCVSDThread::ClosePcmDumpFile()
{
    ALOGD("ClosePcmDumpFile");
    switch (mThreadType)
    {
        case ExtMD_BTSCO_UL_READTHREAD:
            if (mExtMDULReadPCMDumpFile)
            {
                AudioCloseDumpPCMFile(mExtMDULReadPCMDumpFile);
                ALOGD("ClosePcmDumpFile mExtMDULReadPCMDumpFile");
            }
            break;
        case ExtMD_BTSCO_UL_WRITETHREAD:
            if (mExtMDULWritePCMDumpFile)
            {
                AudioCloseDumpPCMFile(mExtMDULWritePCMDumpFile);
                ALOGD("ClosePcmDumpFile mExtMDULWritePCMDumpFile");
            }
            break;
        case ExtMD_BTSCO_DL_READTHREAD:
            if (mExtMDDLReadPCMDumpFile)
            {
                AudioCloseDumpPCMFile(mExtMDDLReadPCMDumpFile);
                ALOGD("ClosePcmDumpFile mExtMDDLReadPCMDumpFile");
            }
            break;
        case ExtMD_BTSCO_DL_WRITETHREAD:
            if (mExtMDDLWritePCMDumpFile)
            {
                AudioCloseDumpPCMFile(mExtMDDLWritePCMDumpFile);
                ALOGD("ClosePcmDumpFile mExtMDDLWritePCMDumpFile");
            }
            break;
        default:
            ALOGW("AudioExtMDCVSDThread::ClosePcmDumpFile unknown mThreadType!!! ");
            break;
    }
}

void AudioBTCVSDControl::AudioExtMDCVSDThread::ExtMD_btsco_cvsd_UL_Read_main(void)
{
    uint8_t packetvalid, *outbuf, *workbuf, *tempbuf, *inbuf, *accuoutbuf, trycount;
    uint32_t i, outsize, workbufsize, insize, bytes, offset, accuoutsize;
    int32_t Read_Size;

    ALOGD("ExtMD_btsco_cvsd_UL_Read_main(+)");
    Read_Size = ::read(mFd2, mAudioBTCVSDControl->BT_SCO_RX_GetCVSDTempInBuf(), BTSCO_CVSD_RX_TEMPINPUTBUF_SIZE);
    ALOGD("ExtMD_btsco_cvsd_UL_Read_main ::read() done Read_Size=%d", Read_Size);

    if (Read_Size <= 0)
    {
        ALOGW("ExtMD_btsco_cvsd_UL_Read_main Read_Size=%d!!!",Read_Size);
        usleep(15 * 1000);
        return;
    }

    ASSERT(Read_Size % (SCO_RX_PLC_SIZE + BTSCO_CVSD_PACKET_VALID_SIZE) == 0);

    accuoutbuf = mAudioBTCVSDControl->BT_SCO_ExtMDGetCVSDAccuOutBuf();
    accuoutsize = 0;

    if (mBTSCOCVSDContext->fIsWideBand == true)
    {
        outbuf = mAudioBTCVSDControl->BT_SCO_RX_GetMSBCOutBuf();
        outsize = MSBC_PCM_FRAME_BYTE;
    }
    else
    {
        outbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDOutBuf();
        outsize = SCO_RX_PCM8K_BUF_SIZE;
    }
    workbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDWorkBuf();
    workbufsize = SCO_RX_PCM64K_BUF_SIZE;
    tempbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDTempInBuf();
    inbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDInBuf();
    insize = SCO_RX_PLC_SIZE;
    //bytes = BTSCO_CVSD_RX_INBUF_SIZE;
    bytes = (Read_Size / (SCO_RX_PLC_SIZE + BTSCO_CVSD_PACKET_VALID_SIZE)) * SCO_RX_PLC_SIZE;
    i = 0;
    offset = 0;
    do
    {
        packetvalid = *((char *)tempbuf + SCO_RX_PLC_SIZE + offset + i * BTSCO_CVSD_PACKET_VALID_SIZE); //parser packet valid info for each 30-byte packet
        //packetvalid   = 1; //force packvalid to 1 for test
        memcpy(inbuf + offset, tempbuf + offset + i * BTSCO_CVSD_PACKET_VALID_SIZE, SCO_RX_PLC_SIZE);

        if (mBTSCOCVSDContext->fIsWideBand == true)
        {
            ALOGD("btsco_process_RX_MSBC(+) insize=%d,outsize=%d,packetvalid=%d ", insize, outsize, packetvalid);
            mAudioBTCVSDControl->btsco_process_RX_MSBC(inbuf + offset, &insize, outbuf, &outsize, workbuf, workbufsize, packetvalid);
        }
        else
        {
            ALOGVV("btsco_process_RX_CVSD(+) insize=%d,outsize=%d,packetvalid=%d ", insize, outsize, packetvalid);
            mAudioBTCVSDControl->btsco_process_RX_CVSD(inbuf + offset, &insize, outbuf, &outsize, workbuf, workbufsize, packetvalid);
        }
        offset += SCO_RX_PLC_SIZE;
        bytes -= insize;
        ALOGVV("btsco_process_RX(-) consumed=%d, outsize=%d, bytes=%d", insize, outsize, bytes);

        if (outsize != 0)
        {
            memcpy(accuoutbuf, outbuf, outsize);
            accuoutbuf += outsize;
            accuoutsize += outsize;
        }

        if (mBTSCOCVSDContext->fIsWideBand == true)
        {
            outsize = MSBC_PCM_FRAME_BYTE;
        }
        else
        {
            outsize = SCO_RX_PCM8K_BUF_SIZE;
        }
        insize = SCO_RX_PLC_SIZE;
        i++;
    }
    while (bytes > 0);

    // total outsize is:  BTSCO_CVSD_RX_INBUF_SIZE*2
    trycount = 0;
    uint32_t FreeSpace;

    accuoutbuf = mAudioBTCVSDControl->BT_SCO_ExtMDGetCVSDAccuOutBuf();
    ALOGD("accuoutsize=%d", accuoutsize);

    do
    {
        mAudioBTCVSDControl->BT_SCO_ExtMDULBufLock();
        FreeSpace = mAudioBTCVSDControl->BT_SCO_ExtMDGetBufSpace(ExtMD_BTSCO_UL);
        ALOGVV("ExtMD_btsco_cvsd_UL_Read_main FreeSpace=%d", FreeSpace);
        if (FreeSpace >= BTSCO_CVSD_RX_INBUF_SIZE * 2)
        {
            mAudioBTCVSDControl->BT_SCO_ExtMDWriteDataToRingBuf(accuoutbuf, accuoutsize, ExtMD_BTSCO_UL);
            mAudioBTCVSDControl->BT_SCO_ExtMDULBufUnLock();
            WritePcmDumpData(accuoutbuf, accuoutsize);
            break;
        }
        else
        {
            ALOGD("ExtMD_btsco_cvsd_UL_Read_main FreeSpace=%d < %d,", FreeSpace, BTSCO_CVSD_RX_INBUF_SIZE * 2);
            mAudioBTCVSDControl->BT_SCO_ExtMDULBufUnLock();
            usleep(10 * 1000);
        }
        trycount++;
    }
    while (trycount < 10);

    if (trycount == 10)
    {
        ALOGW("AudioExtMDCVSDThread::ExtMD_btsco_cvsd_UL_Read_main() BT_SCO_RX_ExtMDWriteDataToULBuf() Timeout!!!");
    }

    ALOGVV("ExtMD_btsco_cvsd_UL_Read_main(-)");
}

void AudioBTCVSDControl::AudioExtMDCVSDThread::ExtMD_btsco_cvsd_UL_Write_main(void)
{
    uint32_t DataCount, trycount = 0, i, src_inszie, src_outsize;
    uint8_t *wtmpbuf, *wtmpbuf2, *pbuf1, *pbuf2;
    uint16_t *pSrc, *pDst, threshold, samplecount;

    ALOGVV("ExtMD_btsco_cvsd_UL_Write_main(+)");

#ifndef EXTMD_SUPPORT_WB
    //if (mBTSCOCVSDContext->fIsWideBand == true)
    if (0)
    {
        pbuf1 = mAudioBTCVSDControl->BT_SCO_ExtMDGetCVSDULWriteTmpBuf2();
        pbuf2 = mAudioBTCVSDControl->BT_SCO_ExtMDGetCVSDULWriteTmpBuf();
    }
    else
#endif
    {
        pbuf1 = mAudioBTCVSDControl->BT_SCO_ExtMDGetCVSDULWriteTmpBuf();
    }
    do
    {
        mAudioBTCVSDControl->BT_SCO_ExtMDULBufLock();
        DataCount = mAudioBTCVSDControl->BT_SCO_ExtMDGetBufCount(ExtMD_BTSCO_UL);
        ALOGVV("ExtMD_btsco_cvsd_UL_Write_main DataCount=%d", DataCount);
        //if (mBTSCOCVSDContext->fIsWideBand == true)
        if (0)
        {
            threshold = BTSCO_CVSD_RX_INBUF_SIZE * 2 * 2;
        }
        else
        {
            threshold = BTSCO_CVSD_RX_INBUF_SIZE * 2;
        }
        if (DataCount >= threshold)
        {
            mAudioBTCVSDControl->BT_SCO_ExtMDReadDataFromRingBuf(pbuf1, threshold, ExtMD_BTSCO_UL); //use temp buf to avoid blocking Lock
            mAudioBTCVSDControl->BT_SCO_ExtMDULBufUnLock();
            break;
        }
        else
        {
            ALOGVV("ExtMD_btsco_cvsd_UL_Write_main DataCount=%d < %d,", DataCount, threshold);
            mAudioBTCVSDControl->BT_SCO_ExtMDULBufUnLock();
            usleep(10 * 1000);
        }
        trycount++;
    }
    while (trycount < 10);

    if (trycount == 10)
    {
        ALOGW("AudioExtMDCVSDThread::ExtMD_btsco_cvsd_UL_Write_main() BT_SCO_RX_ExtMDReadDataFromULBuf() Timeout!!!");
        return;
    }

#ifndef EXTMD_SUPPORT_WB
    //16k to 8k SRC for extMD 8k case (if extMD support 16k, only need to remove this SRC)
    //if (pULSRCHandle!=NULL)
    if (0)
    {
        src_inszie = threshold;
        src_outsize = threshold >> 1;

        pULSRCHandle->Process(pbuf1, &src_inszie, pbuf2, &src_outsize);
        ASSERT(src_outsize == (BTSCO_CVSD_RX_INBUF_SIZE * 2));
    }
#endif

    // duplicate (1ch 16 byte) to (2ch 16 byte)
    pSrc = (uint16_t *)mAudioBTCVSDControl->BT_SCO_ExtMDGetCVSDULWriteTmpBuf();
    pDst = (uint16_t *)mAudioBTCVSDControl->BT_SCO_ExtMDGetCVSDULWriteTmpBuf2();
#ifdef EXTMD_SUPPORT_WB
    //if (mBTSCOCVSDContext->fIsWideBand == true)
    if (0)
    {
        samplecount = BTSCO_CVSD_RX_INBUF_SIZE * 2;
    }
    else
#endif
    {
        samplecount = BTSCO_CVSD_RX_INBUF_SIZE;
    }

    ALOGVV("ExtMD_btsco_cvsd_UL_Write_main pSrc=0x%x, pDst=0x%x, samplecount=%d", pSrc, pDst, samplecount);

    for (i = 0; i < samplecount; i++)
    {
        *pDst = *pSrc;
        pDst++;
        *pDst = *pSrc;
        pDst++;
        pSrc++;
    }

    wtmpbuf2 = mAudioBTCVSDControl->BT_SCO_ExtMDGetCVSDULWriteTmpBuf2();

    WritePcmDumpData(wtmpbuf2, samplecount * 2 * 2);

    ALOGD("ExtMD_btsco_cvsd_UL_Write_main ::write to kernel (+),size=%d", samplecount * 2 * 2);
    ::write(mFd, wtmpbuf2, samplecount * 2 * 2); //BTSCO_CVSD_RX_INBUF_SIZE*2
    ALOGD("ExtMD_btsco_cvsd_UL_Write_main ::write to kernel (-)");

    if (mAFEULStarting == false) //mAudioBTCVSDControl->BT_SCO_RX_ExtMDInitAFEDLHW();
    {
        ALOGD("ExtMD_btsco_cvsd_UL_Write_main mAFEULStarting=false");
        ::write(mFd, wtmpbuf2, samplecount * 2 * 2);
        mAFEULStarting = true;
        //RequesetPlaybackclock();
#ifdef EXTMD_LOOPBACK_TEST
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true); // ANA CLK only for test mode
#endif
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true); //no need ANA CLK

        //SetIMcuIRQ(AudioDigitalType::IRQ1_MCU_MODE, mDL1Attribute);
        mAudioDigitalControl->SetIrqMcuSampleRate(AudioDigitalType::IRQ1_MCU_MODE, EXTMD_BTSCO_AFE_SAMPLERATE);
        mAudioDigitalControl->SetIrqMcuCounter(AudioDigitalType::IRQ1_MCU_MODE, BTSCO_CVSD_RX_INBUF_SIZE); // 480 samples, 60ms
        //EnableIMcuIRQ(AudioDigitalType::IRQ1_MCU_MODE, true);
        mAudioDigitalControl->SetIrqMcuEnable(AudioDigitalType::IRQ1_MCU_MODE, true);

#ifdef EXTMD_LOOPBACK_TEST
        //TurnOnAfeDigital(DigitalPart);

        mAudioDigitalControl->SetMemIfFetchFormatPerSample(AudioDigitalType::MEM_DL1, AudioMEMIFAttribute::AFE_WLEN_16_BIT);
        mAudioDigitalControl->SetoutputConnectionFormat(AudioDigitalType::OUTPUT_DATA_FORMAT_16BIT, AudioDigitalType::O03);
        mAudioDigitalControl->SetoutputConnectionFormat(AudioDigitalType::OUTPUT_DATA_FORMAT_16BIT, AudioDigitalType::O04);

        // interconnection to DAC output
        mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I05, AudioDigitalType::O03);
        mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I06, AudioDigitalType::O04);

        // turn on digital part
        mAudioDigitalControl->SetMemIfEnable(AudioDigitalType::I2S_OUT_DAC, true);

        // turn on DAC_I2S out
        {
            AudioDigtalI2S *mDL1Out;
            mDL1Out = new AudioDigtalI2S();

            ALOGD("EXTMD_LOOPBACK_TEST SetI2SOutDACAttribute");
            mDL1Out->mLR_SWAP = AudioDigtalI2S::NO_SWAP;
            mDL1Out->mI2S_SLAVE = AudioDigtalI2S::MASTER_MODE;
            mDL1Out->mINV_LRCK = AudioDigtalI2S::NO_INVERSE;
            mDL1Out->mI2S_FMT = AudioDigtalI2S::I2S;
            mDL1Out->mI2S_WLEN = AudioDigtalI2S::WLEN_16BITS;
            mDL1Out->mI2S_SAMPLERATE = EXTMD_BTSCO_AFE_SAMPLERATE;
            mAudioDigitalControl->SetI2SDacOut(mDL1Out);
        }
        mAudioDigitalControl->SetI2SDacEnable(true);

        //turn on analog part
        //SetAnalogFrequency(DigitalPart);
        mAudioAnalogControl = AudioAnalogControlFactory::CreateAudioAnalogControl();
        if (!mAudioAnalogControl)
        {
            ALOGD("EXTMD_LOOPBACK_TEST CreateAudioAnalogControl fail!!!");
        }

        //mAudioAnalogControl->SetFrequency(AudioAnalogType::DEVICE_OUT_DAC, EXTMD_BTSCO_AFE_SAMPLERATE);

        //SetPlayBackPinmux();
        mAudioAnalogControl->AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETR, AudioAnalogType::MUX_AUDIO);
        mAudioAnalogControl->AnalogSetMux(AudioAnalogType::DEVICE_OUT_HEADSETL, AudioAnalogType::MUX_AUDIO);
        mAudioAnalogControl->AnalogSetMux(AudioAnalogType::DEVICE_OUT_SPEAKERR, AudioAnalogType::MUX_AUDIO);
        mAudioAnalogControl->AnalogSetMux(AudioAnalogType::DEVICE_OUT_SPEAKERL, AudioAnalogType::MUX_AUDIO);

        mAudioResourceManager = AudioResourceManager::getInstance();
        mAudioResourceManager->SetFrequency(AudioResourceManagerInterface::DEVICE_OUT_DAC, EXTMD_BTSCO_AFE_SAMPLERATE);
        mAudioResourceManager->StartOutputDevice();  // open analog device and set master volume
		
#else
        mAudioDigitalControl->SetMemIfFetchFormatPerSample(AudioDigitalType::MEM_DL1, AudioMEMIFAttribute::AFE_WLEN_16_BIT);
        mAudioDigitalControl->SetoutputConnectionFormat(AudioDigitalType::OUTPUT_DATA_FORMAT_16BIT, AudioDigitalType::O07);
        mAudioDigitalControl->SetoutputConnectionFormat(AudioDigitalType::OUTPUT_DATA_FORMAT_16BIT, AudioDigitalType::O08);

        mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I05, AudioDigitalType::O07);
        mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I06, AudioDigitalType::O08);
#endif

        mAudioDigitalControl->SetAfeEnable(true);

        //Note: MOD_PCM interface enable is done in OpenModemSpeechControlFlow() and ChangeDeviceForModemSpeechControlFlow()
        mAudioDigitalControl->SetMemIfSampleRate(AudioDigitalType::MEM_DL1, EXTMD_BTSCO_AFE_SAMPLERATE);
        mAudioDigitalControl->SetMemIfChannelCount(AudioDigitalType::MEM_DL1, 2);
        mAudioDigitalControl->SetMemIfEnable(AudioDigitalType::MEM_DL1, true);
    }
    ALOGVV("ExtMD_btsco_cvsd_UL_Write_main(-)");
}

void AudioBTCVSDControl::AudioExtMDCVSDThread::ExtMD_btsco_cvsd_DL_Read_main(void)
{
    uint8_t rtmpbuf[BTSCO_CVSD_TX_OUTBUF_SIZE * 2];
    uint32_t FreeSpace, readsize;
    uint8_t trycount = 0;
    int32_t Read_Size;

    ALOGVV("ExtMD_btsco_cvsd_DL_Read_main(+)");

    if (mAFEDLStarting == false)
    {
        ALOGD("ExtMD_btsco_cvsd_DL_Read_main mAFEDLStarting = false");
        mAFEDLStarting = true;
        //RequesetRecordclock();
#ifdef EXTMD_LOOPBACK_TEST
        //mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_ANA, true); // ANA CLK only for test mode
#endif
        mAudioResourceManager->EnableAudioClock(AudioResourceManagerInterface::CLOCK_AUD_AFE, true); //no need ANA CLK

        mAudioDigitalControl->SetMemIfSampleRate(AudioDigitalType::MEM_MOD_DAI, EXTMD_BTSCO_AFE_SAMPLERATE);
        mAudioDigitalControl->SetMemIfChannelCount(AudioDigitalType::MEM_MOD_DAI, 1);
        mAudioDigitalControl->SetMemIfEnable(AudioDigitalType::MEM_MOD_DAI, true);

        // set irq enable , need handle with irq2 mcu mode.
        AudioIrqMcuMode IrqStatus;
        mAudioDigitalControl->GetIrqStatus(AudioDigitalType::IRQ2_MCU_MODE, &IrqStatus);
        if (IrqStatus.mStatus == false)
        {
            ALOGD("SetIrqMcuSampleRate mSampleRate = %d", EXTMD_BTSCO_AFE_SAMPLERATE);
            mAudioDigitalControl->SetIrqMcuSampleRate(AudioDigitalType::IRQ2_MCU_MODE, EXTMD_BTSCO_AFE_SAMPLERATE);
            mAudioDigitalControl->SetIrqMcuCounter(AudioDigitalType::IRQ2_MCU_MODE, BTSCO_CVSD_TX_OUTBUF_SIZE); // 480 samples, 60ms
            mAudioDigitalControl->SetIrqMcuEnable(AudioDigitalType::IRQ2_MCU_MODE, true);
        }
        else
        {
            ALOGD("IRQ2_MCU_MODE is enabled , use original irq2 interrupt mode");
        }

        mAudioDigitalControl->SetMemIfFetchFormatPerSample(AudioDigitalType::MEM_MOD_DAI, AudioMEMIFAttribute::AFE_WLEN_16_BIT);
        mAudioDigitalControl->SetoutputConnectionFormat(AudioDigitalType::OUTPUT_DATA_FORMAT_16BIT, AudioDigitalType::O12);

        // set interconnection
#ifdef EXTMD_LOOPBACK_TEST
        mAudioDigitalControl->EnableSideToneHw(AudioDigitalType::O12 , false, true);
#else
        mAudioDigitalControl->SetinputConnection(AudioDigitalType::Connection, AudioDigitalType::I09, AudioDigitalType::O12);
#endif

        mAudioDigitalControl->SetAfeEnable(true);

    }

    //if (mBTSCOCVSDContext->fIsWideBand == true)
    if (0)
    {
        readsize = BTSCO_CVSD_TX_OUTBUF_SIZE * 2 * 2;
    }
    else
    {
        readsize = BTSCO_CVSD_TX_OUTBUF_SIZE * 2;
    }

    Read_Size = ::read(mFd, rtmpbuf, readsize);
    ALOGVV("ExtMD_btsco_cvsd_DL_Read_main ::read() done Read_Size=%d", Read_Size);
    //ASSERT(Read_Size==sizeof(rtmpbuf));

    if (Read_Size <= 0)
    {
        ALOGW("ExtMD_btsco_cvsd_DL_Read_main Read_Size=%d!!!",Read_Size);
        usleep(15 * 1000);
        return;
    }

    do
    {
        mAudioBTCVSDControl->BT_SCO_ExtMDDLBufLock();
        FreeSpace = mAudioBTCVSDControl->BT_SCO_ExtMDGetBufSpace(ExtMD_BTSCO_DL);
        ALOGVV("ExtMD_btsco_cvsd_DL_Read_main FreeSpace=%d", FreeSpace);
        if (FreeSpace >= Read_Size)
        {
            mAudioBTCVSDControl->BT_SCO_ExtMDWriteDataToRingBuf(rtmpbuf, Read_Size, ExtMD_BTSCO_DL);
            mAudioBTCVSDControl->BT_SCO_ExtMDDLBufUnLock();
            WritePcmDumpData(rtmpbuf, Read_Size);
            break;
        }
        else
        {
            ALOGVV("ExtMD_btsco_cvsd_DL_Read_main FreeSpace=%d < %d,", FreeSpace, Read_Size);
            mAudioBTCVSDControl->BT_SCO_ExtMDDLBufUnLock();
            usleep(10 * 1000);
        }
        trycount++;
    }
    while (trycount < 10);

    if (trycount == 10)
    {
        ALOGW("AudioExtMDCVSDThread::ExtMD_btsco_cvsd_DL_Read_main() BT_SCO_ExtMDWriteDataToRingBuf(DL) Timeout!!!");
    }

    ALOGVV("ExtMD_btsco_cvsd_DL_Read_main(-)");
}

void AudioBTCVSDControl::AudioExtMDCVSDThread::ExtMD_btsco_cvsd_DL_Write_main(void)
{

    uint32_t DataCount, trycount = 0;
    ssize_t WrittenBytes = 0, bytes;
    size_t outputSize = 0;
    uint8_t *outbuffer, *inbuf, *workbuf, i;
    uint32_t insize, outsize, workbufsize, total_outsize, src_fs_s, readsize;

    uint8_t wtmpbuf[BTSCO_CVSD_TX_OUTBUF_SIZE * 2];

    ALOGD("ExtMD_btsco_cvsd_DL_Write_main(+)");

    //if (mBTSCOCVSDContext->fIsWideBand == true)
    if (0)
    {
        readsize = BTSCO_CVSD_TX_OUTBUF_SIZE * 2 * 2;
    }
    else
    {
        readsize = BTSCO_CVSD_TX_OUTBUF_SIZE * 2;
    }

    do
    {
        mAudioBTCVSDControl->BT_SCO_ExtMDDLBufLock();
        DataCount = mAudioBTCVSDControl->BT_SCO_ExtMDGetBufCount(ExtMD_BTSCO_DL);
        ALOGVV("ExtMD_btsco_cvsd_DL_Write_main DataCount=%d", DataCount);
        if (DataCount >= readsize)
        {
            mAudioBTCVSDControl->BT_SCO_ExtMDReadDataFromRingBuf(wtmpbuf, readsize, ExtMD_BTSCO_DL); //use temp buf to avoid blocking Lock
            mAudioBTCVSDControl->BT_SCO_ExtMDDLBufUnLock();
            break;
        }
        else
        {
            ALOGVV("ExtMD_btsco_cvsd_DL_Write_main DataCount=%d < %d,", DataCount, readsize);
            mAudioBTCVSDControl->BT_SCO_ExtMDDLBufUnLock();
            usleep(10 * 1000);
        }
        trycount++;
    }
    while (trycount < 10);

    if (trycount == 10)
    {
        ALOGW("AudioExtMDCVSDThread::ExtMD_btsco_cvsd_DL_Write_main() BT_SCO_ExtMDReadDataFromRingBuf(DL) Timeout!!!");
        return;
    }

    inbuf = (uint8_t *)wtmpbuf;
    bytes = readsize;

    WritePcmDumpData(wtmpbuf, bytes);

    do
    {
        outbuffer = mAudioBTCVSDControl->BT_SCO_TX_GetCVSDOutBuf();
        outsize = SCO_TX_ENCODE_SIZE;
        insize = bytes;
        workbuf = mAudioBTCVSDControl->BT_SCO_TX_GetCVSDWorkBuf();
        workbufsize = SCO_TX_PCM64K_BUF_SIZE;
        src_fs_s = EXTMD_BTSCO_AFE_SAMPLERATE;//source sample rate for SRC
        total_outsize = 0;
        i = 0;
        do
        {
            if (mBTSCOCVSDContext->fIsWideBand == true)
            {
                mAudioBTCVSDControl->btsco_process_TX_MSBC(inbuf, &insize, outbuffer, &outsize, workbuf, workbufsize, src_fs_s); //return insize is consumed size
                ALOGVV("btsco_process_TX_MSBC, do mSBC encode outsize=%d, consumed size=%d, bytes=%d", outsize, insize, bytes);
            }
            else
            {
                mAudioBTCVSDControl->btsco_process_TX_CVSD(inbuf, &insize, outbuffer, &outsize, workbuf, workbufsize, src_fs_s); //return insize is consumed size
                ALOGVV("btsco_process_TX_CVSD outsize=%d, consumed size=%d, bytes=%d", outsize, insize, bytes);
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

        ALOGD("ExtMD_btsco_cvsd_DL_Write_main write to kernel(+) total_outsize=%d", total_outsize);
        WrittenBytes =::write(mFd2, mAudioBTCVSDControl->BT_SCO_TX_GetCVSDOutBuf(), total_outsize);  //total_outsize should be BTSCO_CVSD_TX_OUTBUF_SIZE!!!
        ALOGD("ExtMD_btsco_cvsd_DL_Write_main write to kernel(-) remaining bytes=%d", bytes);
    }
    while (bytes > 0);

    ALOGD("ExtMD_btsco_cvsd_DL_Write_main(-)");

}

bool AudioBTCVSDControl::AudioExtMDCVSDThread::threadLoop()
{
    uint32 Read_Size = 0;
    while (!(exitPending() == true))
    {
        ALOGD("threadLoop mThreadType=%d", mThreadType);
        if (mThreadType == ExtMD_BTSCO_UL_READTHREAD)
        {
            //usleep(200*1000); //wait BT CVSD IRQ enable
            ExtMD_btsco_cvsd_UL_Read_main();
            return true;
        }
        else if (mThreadType == ExtMD_BTSCO_UL_WRITETHREAD)
        {
            ExtMD_btsco_cvsd_UL_Write_main();
            return true;
        }
        else if (mThreadType == ExtMD_BTSCO_DL_READTHREAD)
        {
            ExtMD_btsco_cvsd_DL_Read_main();
            return true;
        }
        else if (mThreadType == ExtMD_BTSCO_DL_WRITETHREAD)
        {
            ExtMD_btsco_cvsd_DL_Write_main();
            return true;
        }
    }
    ALOGD("threadLoop exit mThreadType=%d", mThreadType);
    return false;
}
#endif

void AudioBTCVSDControl::BTCVSD_Init(int mFd2, uint32 mSourceSampleRate, uint32 mSourceChannels)
{
    mAudioBTCVSDControl = AudioBTCVSDControl::getInstance();
    if (!mAudioBTCVSDControl)
    {
        ALOGE("BTCVSD_Init getInstance() fail");
    }

    mAudioBTCVSDControl->BT_SCO_TX_Begin(mFd2, mSourceSampleRate, mSourceChannels);

    if (AudioLoopbackController::GetInstance()->IsAPBTLoopbackWithCodec() == true)
    {
        ALOGD("****************BTCVSD loopbacktest create AudioBTCVSDLoopbackRxThread************** \n");
        mBTCVSDRxTestThread = new AudioBTCVSDLoopbackRxThread(this, AudioDigitalType::MEM_DAI , NULL, 0);
        if (mBTCVSDRxTestThread.get())
        {
            mBTCVSDRxTestThread->run();
        }
    }
#if defined(BTCVSD_KERNEL_LOOPBACK) // create MTKRecordthread for test BTCVSD RX
    ALOGD("****************BTCVSD TEST create AudioBTCVSDLoopbackRxThread************** \n");
    mBTCVSDRxTestThread = new AudioBTCVSDLoopbackRxThread(this, AudioDigitalType::MEM_DAI , NULL, 0);
    if (mBTCVSDRxTestThread.get())
    {
        mBTCVSDRxTestThread->run();
    }
#elif defined(BTCVSD_ENC_DEC_LOOPBACK)
    ALOGD("write()===BTCVSD_ENC_DEC_LOOPBACK=== START");
    mAudioBTCVSDControl->BT_SCO_RX_Begin(mFd2);
    mCVSDloopbackPCMDumpFile = NULL;
    mCVSDloopbackPCMDumpFile = AudioOpendumpPCMFile("/sdcard/mtklog/audio_dump/cvsdloopback.pcm", "cvsdloopback.pcm.dump");
#endif
}

void AudioBTCVSDControl::BTCVSD_StandbyProcess(int mFd2)
{
    mAudioBTCVSDControl->BT_SCO_TX_End(mFd2);

    if (AudioLoopbackController::GetInstance()->IsAPBTLoopbackWithCodec() == true)
    {
        int ret = 0;
        if (mBTCVSDRxTestThread.get())
        {
            //ret = mBTCVSDRxTestThread->requestExitAndWait();
            //if (ret == WOULD_BLOCK) {
            mBTCVSDRxTestThread->requestExit();
            //}
            mBTCVSDRxTestThread.clear();
        }
    }
#if defined(BTCVSD_KERNEL_LOOPBACK) // close AudioBTCVSDControl::AudioBTCVSDLoopbackRxThread
    int ret = 0;
    if (mBTCVSDRxTestThread.get())
    {
        ret = mBTCVSDRxTestThread->requestExitAndWait();
        if (ret == WOULD_BLOCK)
        {
            mBTCVSDRxTestThread->requestExit();
        }
        mBTCVSDRxTestThread.clear();
    }
    if (mCVSDloopbackPCMDumpFile)
    {
        AudioCloseDumpPCMFile(mCVSDloopbackPCMDumpFile);
        ALOGD("ClosePcmDumpFile mCVSDloopbackPCMDumpFile");
    }
#elif defined(BTCVSD_ENC_DEC_LOOPBACK)
    ALOGD("standby()===BTCVSD_ENC_DEC_LOOPBACK=== STOP");
    mAudioBTCVSDControl->BT_SCO_RX_End(mFd2);

    if (mCVSDloopbackPCMDumpFile)
    {
        AudioCloseDumpPCMFile(mCVSDloopbackPCMDumpFile);
        ALOGD("ClosePcmDumpFile mCVSDloopbackPCMDumpFile");
    }
#endif
}


#if defined(BTCVSD_ENC_DEC_LOOPBACK)
void AudioBTCVSDControl::BTCVSD_Test_UserSpace_TxToRx(uint32_t total_outsize)
{
    uint32_t offset, loopback_total_outsize, packetvalid, workbufsize, insize, outsize, bytes;
    uint8_t *outbuf;
    uint8_t *inbuf, *workbuf, i;

    ALOGD("WriteDataToBTSCOHW()===BTCVSD_ENC_DEC_LOOPBACK===, total_outsize=%d", total_outsize);
    memcpy(mAudioBTCVSDControl->BT_SCO_RX_GetCVSDInBuf(), mAudioBTCVSDControl->BT_SCO_TX_GetCVSDOutBuf(), total_outsize); //copy data directly from mBTCVSDTXOutBuf to mBTCVSDRXInBuf

    outbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDOutBuf();
    outsize = SCO_RX_PCM8K_BUF_SIZE;
    workbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDWorkBuf();
    workbufsize = SCO_RX_PCM64K_BUF_SIZE;
    inbuf = mAudioBTCVSDControl->BT_SCO_RX_GetCVSDInBuf();
    insize = SCO_RX_PLC_SIZE;

    bytes = total_outsize;//BTSCO_CVSD_RX_INBUF_SIZE;
    i = 0;
    offset = 0;
    loopback_total_outsize = 0;

    do
    {
        packetvalid = 1;

        ALOGD("BTCVSD_ENC_DEC_LOOPBACK btsco_process_RX_CVSD(+) insize=%d,outsize=%d,packetvalid=%d ", insize, outsize, packetvalid);
        mAudioBTCVSDControl->btsco_process_RX_CVSD(inbuf + offset, &insize, outbuf, &outsize, workbuf, workbufsize, packetvalid);
        offset += SCO_RX_PLC_SIZE;
        bytes -= insize;
        ALOGD("BTCVSD_ENC_DEC_LOOPBACK btsco_process_RX_CVSD(-) consumed=%d,outsize=%d, bytes=%d", insize, outsize, bytes);
        loopback_total_outsize += outsize;

        if (mCVSDloopbackPCMDumpFile)
        {
            fwrite((void *)outbuf, 1, outsize, mCVSDloopbackPCMDumpFile);
        }

        outsize = SCO_RX_PCM8K_BUF_SIZE;
        insize = SCO_RX_PLC_SIZE;
        i++;
    }
    while (bytes > 0);

    ALOGD("BTCVSD_ENC_DEC_LOOPBACK input=%d, remaining=%d,loopback_total_outsize=%d", total_outsize, bytes, loopback_total_outsize);

}
#endif

AudioBTCVSDControl::AudioBTCVSDLoopbackRxThread::AudioBTCVSDLoopbackRxThread(void *mAudioManager, uint32 Mem_type, char *RingBuffer, uint32 BufferSize)
{
    ALOGD("BT_SW_CVSD CODEC LOOPBACK record thread: AudioBTCVSDLoopbackRxThread(+) constructor Mem_type = %d", Mem_type);
    mMemType = Mem_type;
    //mManager = AudioManager;
    mFd2 = -1;
    mAudioBTCVSDControl = AudioBTCVSDControl::getInstance();
    if (!mAudioBTCVSDControl)
    {
        ALOGE("BT_SW_CVSD CODEC LOOPBACK record thread: AudioBTCVSDControl::getInstance() fail");
    }
    if (mMemType == AudioDigitalType::MEM_DAI)
    {
        mFd2 =  ::open(kBTDeviceName, O_RDWR);
        if (mFd2 <= 0)
        {
            ALOGW("BT_SW_CVSD CODEC LOOPBACK record thread: open fail");
        }
    }

    switch (mMemType)
    {
        case AudioDigitalType::MEM_DAI:
            mName = String8("AudioBTCVSDLoopbackRxThreadDAI");
            mBTCVSDLoopbackDumpFile = NULL;
            mBTCVSDLoopbackDumpFile = AudioOpendumpPCMFile("/sdcard/mtklog/audio_dump/CVSDloopbackOut.pcm", "CVSDloopbackOut.pcm.dump");
            mAudioBTCVSDControl->BT_SCO_RX_Begin(mFd2);
            break;
        default:
            ALOGD("BT_SW_CVSD CODEC LOOPBACK record thread:  NO support for memory interface");
            break;
    }
    // ring buffer to copy data into this ring buffer
    mRingBuffer = RingBuffer;
    mBufferSize = BufferSize;
    ALOGD("BT_SW_CVSD CODEC LOOPBACK record thread: AudioBTCVSDLoopbackRxThread(-)");
}

AudioBTCVSDControl::AudioBTCVSDLoopbackRxThread::~AudioBTCVSDLoopbackRxThread()
{
    ALOGD("BT_SW_CVSD CODEC LOOPBACK record thread:  ~AudioBTCVSDLoopbackRxThread(+)");
    ClosePcmDumpFile();

    if (mMemType == AudioDigitalType::MEM_DAI)
    {
        mAudioBTCVSDControl->BT_SCO_RX_End(mFd2);

        if (mFd2 > 0)
        {
            ::close(mFd2);
            mFd2 = 0;
        }
    }

    ALOGD("BT_SW_CVSD CODEC LOOPBACK record thread:  ~AudioBTCVSDLoopbackRxThread(-)");
}

void AudioBTCVSDControl::AudioBTCVSDLoopbackRxThread::onFirstRef()
{
    ALOGD("BT_SW_CVSD CODEC LOOPBACK record thread: onFirstRef");
    tempdata = 0;
    mRecordDropms = 0;

    run(mName, ANDROID_PRIORITY_URGENT_AUDIO);
}

// Good place to do one-time initializations
status_t  AudioBTCVSDControl::AudioBTCVSDLoopbackRxThread::readyToRun()
{
    ALOGD("BT_SW_CVSD CODEC LOOPBACK record thread: readyToRun");
    return NO_ERROR;
}

void AudioBTCVSDControl::AudioBTCVSDLoopbackRxThread::WritePcmDumpData(void *outbuf, uint32_t outsize)
{
    int written_data = 0;
    switch (mMemType)
    {
        case AudioDigitalType::MEM_DAI:
            if (mBTCVSDLoopbackDumpFile)
            {
                written_data = fwrite((void *)outbuf, 1, outsize, mBTCVSDLoopbackDumpFile);
            }
            break;
    }
}

void AudioBTCVSDControl::AudioBTCVSDLoopbackRxThread::ClosePcmDumpFile()
{
    ALOGD("BT_SW_CVSD Test ClosePcmDumpFile");

    switch (mMemType)
    {
        case AudioDigitalType::MEM_DAI:
            if (mBTCVSDLoopbackDumpFile)
            {
                AudioCloseDumpPCMFile(mBTCVSDLoopbackDumpFile);
                ALOGD("ClosePcmDumpFile mBTCVSDLoopbackDumpFile");
            }
            break;
    }
}

void AudioBTCVSDControl::AudioBTCVSDLoopbackRxThread::btsco_cvsd_RX_main(void)
{
    uint8_t packetvalid, *outbuf, *workbuf, *tempbuf, *inbuf;
    uint32_t i, Read_Size, outsize, workbufsize, insize, bytes, offset, dump_size;
    ALOGD("BT_SW_CVSD CODEC LOOPBACK record thread: btsco_cvsd_RX_main(+)");

    Read_Size = ::read(mFd2, mAudioBTCVSDControl->BT_SCO_RX_GetCVSDTempInBuf(), BTSCO_CVSD_RX_TEMPINPUTBUF_SIZE);

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
    dump_size = 0;
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
        //#if !defined(BTCVSD_LOOPBACK_WITH_CODEC)

        uint8_t *pWriteBuffer;
        uint32_t uWriteByte;
        uint32_t uTotalWriteByte;
        CVSDLoopbackGetWriteBuffer(&pWriteBuffer, &uWriteByte);
        if (uWriteByte)
        {
            uint32_t uCopyByte = 0;
            if (uWriteByte >= outsize)
            {
                memcpy(pWriteBuffer, outbuf, outsize);
                uCopyByte += outsize;
                CVSDLoopbackWriteDataDone(outsize);
            }
            else
            {
                memcpy(pWriteBuffer, outbuf, uWriteByte);
                uCopyByte += uWriteByte;
                CVSDLoopbackWriteDataDone(uWriteByte);
                CVSDLoopbackGetWriteBuffer(&pWriteBuffer, &uWriteByte);
                if (uWriteByte == 0)
                {
                    ALOGD("BT_SW_CVSD CODEC LOOPBACK record thread: btsco_cvsd_RX_main underflow: uWriteByte: %d, datalen:%d", uWriteByte, outsize - uCopyByte);
                }
                else if (outsize - uCopyByte >= uWriteByte)
                {
                    //overflow
                    memcpy(pWriteBuffer, outbuf + uCopyByte, uWriteByte);
                    uCopyByte += uWriteByte;
                    CVSDLoopbackWriteDataDone(uWriteByte);
                }
                else
                {
                    memcpy(pWriteBuffer, outbuf + uCopyByte, outsize - uCopyByte);
                    uCopyByte += outsize - uCopyByte;
                    CVSDLoopbackWriteDataDone(outsize - uCopyByte);
                }
            }
        }

        outsize = SCO_RX_PCM8K_BUF_SIZE;
        insize = SCO_RX_PLC_SIZE;
        i++;
    }
    while (bytes > 0);

    ALOGD("BT_SW_CVSD CODEC LOOPBACK record thread: btsco_cvsd_RX_main(-)");
}

bool AudioBTCVSDControl::AudioBTCVSDLoopbackRxThread::threadLoop()
{
    uint32 Read_Size = 0;
    ALOGD("BT_SW_CVSD CODEC LOOPBACK RX thread: threadLoop(+)");
    while (!(exitPending() == true))
    {
        if (mMemType == AudioDigitalType::MEM_DAI)
        {
            btsco_cvsd_RX_main();
            return true;
        }
    }
    ALOGD("BT_SW_CVSD CODEC LOOPBACK RX thread: threadLoop(-), threadLoop exit");
    return false;
}
#endif
}
