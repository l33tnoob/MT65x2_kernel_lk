#ifndef _AUDIO_BTCVSD_CONTROL_H_
#define _AUDIO_BTCVSD_CONTROL_H_

#ifdef SW_BTCVSD_ENABLE
#define BT_SW_CVSD  //Enable SW BT CVSD
#define EXT_MODEM_BT_CVSD
#define EXTMD_SUPPORT_WB
//#define EXTMD_LOOPBACK_TEST

//#define BTCVSD_LOOPBACK_WITH_CODEC

//#define BTCVSD_ENC_DEC_LOOPBACK //copy TX output buf to RX inbuf directly, without writing to kernel
//#define BTCVSD_KERNEL_LOOPBACK //enable this option for TXRX loopback in kernel

//#define TXOUT_RXIN_TEST
//#define BTCVSD_TEST_HW_ONLY
#endif

#ifdef BT_SW_CVSD

#include "AudioType.h"
#include "AudioBTCVSDDef.h"
#include "AudioIoctl.h"
#include "AudioUtility.h"

#ifdef EXT_MODEM_BT_CVSD
#include "AudioDigitalControlFactory.h"
#include "AudioResourceManager.h"
#ifdef EXTMD_LOOPBACK_TEST
#include "AudioAnalogControlFactory.h"
#include "AudioAfeReg.h"
#include "AudioDigitalType.h"
#endif
#endif

extern "C" {
#include "MtkAudioSrc.h"
#include "cvsd_codec_exp.h"
#include "msbc_codec_exp.h"
}

namespace android
{

enum BT_SCO_STATE {
    BT_SCO_TXSTATE_IDLE = 0x0,
    BT_SCO_TXSTATE_INIT,
    BT_SCO_TXSTATE_READY,
    BT_SCO_TXSTATE_RUNNING,
    BT_SCO_TXSTATE_ENDING,
    BT_SCO_RXSTATE_IDLE = 0x10,
    BT_SCO_RXSTATE_INIT,
    BT_SCO_RXSTATE_READY,
    BT_SCO_RXSTATE_RUNNING,
    BT_SCO_RXSTATE_ENDING,
    BT_SCO_TXSTATE_DIRECT_LOOPBACK
} ;

#ifdef EXT_MODEM_BT_CVSD
#define EXTMD_BTSCO_AFE_SAMPLERATE (8000)

enum EXTMD_BTSCO_THREAD_TYPE
{
    ExtMD_BTSCO_UL_READTHREAD = 0x0,
    ExtMD_BTSCO_UL_WRITETHREAD,
    ExtMD_BTSCO_DL_READTHREAD,
    ExtMD_BTSCO_DL_WRITETHREAD,
} ;

enum EXTMD_BTSCO_DIRECTION
{
    ExtMD_BTSCO_UL = 0x0,
    ExtMD_BTSCO_DL,
} ;
#endif


class BT_SCO_TX
{
    public:
        MtkAudioSrc          *pSRCHandle;
        void          *pEncHandle;
        void          *pHPFHandle;
        void (*pCallback)(void *pData);
        void          *pUserData;
        uint8_t     PcmBuf_64k[SCO_TX_PCM64K_BUF_SIZE];
        uint32_t    uPcmBuf_w;
        uint32_t  iPacket_w;
        uint16_t    uSampleRate;
        uint8_t     uChannelNumber;
        bool      fEnableFilter;
        bool      fEnablePLC;

} ;

class BT_SCO_RX
{
    public:
        //handle
        void          *pDecHandle;
        void          *pHPFHandle;
        void          *pPLCHandle;
        MtkAudioSrc          *pSRCHandle_1;
        MtkAudioSrc          *pSRCHandle_2;
        //callback
        void (*pCallback)(void *pData);
        void          *pUserData;
        //temp buffer
        uint8_t     PcmBuf_64k[SCO_RX_PCM64K_BUF_SIZE];
        uint8_t     PcmBuf_8k[SCO_RX_PCM8K_BUF_SIZE];
#ifdef EXT_MODEM_BT_CVSD
        uint8_t     PcmBuf_8k_accu[BTSCO_CVSD_RX_INBUF_SIZE * 2 * 2];
#endif
        uint32_t    uPcmBuf_r; //for PcmBuf_8k
        uint16_t    uSampleRate;
        uint8_t     uChannelNumber;
        bool      fEnableSRC2;
        bool      fEnableFilter;
        bool      fEnablePLC;
        //mSBC
        uint8_t     PacketBuf[SCO_RX_PACKER_BUF_NUM][SCO_RX_PLC_SIZE];
        uint8_t     EntirePacket[MSBC_PACKET_SIZE_BYTE];
        uint8_t     PcmBuf_mSBC[MSBC_PCM_FRAME_BYTE];
        bool        PacketValid[SCO_RX_PACKER_BUF_NUM];
        uint32_t    iPacket_w;
        uint32_t    iPacket_r;
} ;

class BTSCO_CVSD_Context
{
    public:

        BT_SCO_TX *pTX;   //btsco.pTx
        BT_SCO_RX *pRX;
        uint8_t *pStructMemory;
        uint8_t *pTXWorkingMemory;
        uint8_t *pRXWorkingMemory;
        uint16_t uAudId;
        BT_SCO_STATE uTXState;
        BT_SCO_STATE uRXState;
        bool  fIsStructMemoryOnMED;
        bool  fIsWideBand;
};


class AudioBTCVSDControl
{
    public:

        enum BT_SCO_MODE
        {
            BT_SCO_MODE_CVSD,
            BT_SCO_MODE_MSBC
        } ;

        enum BT_SCO_LINK
        {
            BT_SCO_LINK_TX_ONLY,
            BT_SCO_LINK_RX_ONLY,
            BT_SCO_LINK_BOTH,
        } ;

        enum BT_SCO_MODULE
        {
            BT_SCO_MOD_CVSD_ENCODE,
            BT_SCO_MOD_CVSD_DECODE,
            BT_SCO_MOD_FILTER_TX,
            BT_SCO_MOD_FILTER_RX,
            BT_SCO_MOD_PLC_NB,
            BT_SCO_MOD_CVSD_TX_SRC,
            BT_SCO_MOD_MSBC_TX_SRC,
            BT_SCO_MOD_MSBC_RX_SRC,
            BT_SCO_MOD_CVSD_RX_SRC1,
            BT_SCO_MOD_CVSD_RX_SRC2,
            BT_SCO_MOD_PCM_RINGBUF_TX,
            BT_SCO_MOD_PCM_RINGBUF_RX,
            BT_SCO_MOD_MSBC_DECODE,
            BT_SCO_MOD_MSBC_ENCODE,
            BT_SCO_MOD_PLC_WB,
        } ;

        enum BT_SCO_DIRECT
        {
            BT_SCO_DIRECT_BT2ARM,
            BT_SCO_DIRECT_ARM2BT
        } ;

        enum BT_SCO_PACKET_LEN
        {
            BT_SCO_CVSD_30 = 0,
            BT_SCO_CVSD_60 = 1,
            BT_SCO_CVSD_90 = 2,
            BT_SCO_CVSD_120 = 3,
            BT_SCO_CVSD_10 = 4,
            BT_SCO_CVSD_20 = 5,
            BT_SCO_CVSD_MAX = 6
        } ;

        static AudioBTCVSDControl *getInstance();
        static void freeInstance();
        void BT_SCO_CVSD_Init(void);
        void BT_SCO_CVSD_DeInit(void);
        void BT_SCO_SET_TXState(BT_SCO_STATE state);
        void BT_SCO_SET_RXState(BT_SCO_STATE state);
        void BT_SCO_RX_Open(void);
        int BT_SCO_RX_SetHandle(void(*pCallback)(void *pData), void *pData, uint32_t uSampleRate, uint32_t uChannelNumber, uint32_t uEnableFilter);
        void BT_SCO_RX_Start(void);
        void BT_SCO_RX_Stop(void);
        void BT_SCO_RX_Close(void);
        void BT_SCO_RX_Begin(int mFd2);
        void BT_SCO_RX_End(int mFd2);
        void BT_SCO_RX_DestroyModule(void);
        uint8_t *BT_SCO_RX_GetCVSDTempInBuf(void);
        void BT_SCO_RX_SetCVSDTempInBuf(uint8_t *addr);
        uint8_t *BT_SCO_RX_GetCVSDInBuf(void);
        void BT_SCO_RX_SetCVSDInBuf(uint8_t *addr);
        uint8_t *BT_SCO_RX_GetCVSDOutBuf(void);
        uint8_t *BT_SCO_RX_GetCVSDWorkBuf(void);
        uint8_t *BT_SCO_RX_GetMSBCOutBuf(void);
        void btsco_cvsd_RX_main(void);
        void btsco_process_RX_CVSD(void *inbuf, uint32_t *insize, void *outbuf, uint32_t *outsize, void *workbuf, const uint32_t workbufsize, uint8_t packetvalid);
        void btsco_process_TX_CVSD(void *inbuf, uint32_t *insize, void *outbuf, uint32_t *outsize, void *workbuf, const uint32_t workbufsize, uint32_t src_fs_s);
        void BT_SCO_TX_Open(void);
        int BT_SCO_TX_SetHandle(void(*pCallback)(void *pData), void *pData, uint32_t uSampleRate, uint32_t uChannelNumber, uint32_t uEnableFilter);
        void BT_SCO_TX_Start(void);
        void BT_SCO_TX_Stop(void);
        void BT_SCO_TX_Close(void);
        void BT_SCO_TX_Begin(int mFd2, uint32_t uSampleRate, uint32_t uChannelNumber);
        void BT_SCO_TX_End(int mFd2);
        void BT_SCO_TX_DestroyModule(void);
        void BT_SCO_TX_SetCVSDOutBuf(uint8_t *addr);
        uint8_t *BT_SCO_TX_GetCVSDOutBuf(void);
        uint8_t *BT_SCO_TX_GetCVSDWorkBuf(void);
        void BTCVSD_Init(int mFd2, uint32 mSourceSampleRate, uint32 mSourceChannels);
        void BTCVSD_StandbyProcess(int mFd2);
#if defined(BTCVSD_ENC_DEC_LOOPBACK)
        void BTCVSD_Test_UserSpace_TxToRx(uint32_t total_outsize);
#endif
#ifdef EXT_MODEM_BT_CVSD
        void AudioExtMDCVSDCreateThread(void);
        void AudioExtMDCVSDDeleteThread(void);
        bool BT_SCO_ExtMDULBufLock(void);
        bool BT_SCO_ExtMDULBufUnLock(void);
        bool BT_SCO_ExtMDDLBufLock(void);
        bool BT_SCO_ExtMDDLBufUnLock(void);
        void BT_SCO_ExtMDInitBuf(EXTMD_BTSCO_DIRECTION direction);
        uint32_t BT_SCO_ExtMDGetBufSpace(EXTMD_BTSCO_DIRECTION direction);
        uint32_t BT_SCO_ExtMDGetBufCount(EXTMD_BTSCO_DIRECTION direction);
        void BT_SCO_ExtMDWriteDataToRingBuf(uint8_t *buf, uint32_t size, EXTMD_BTSCO_DIRECTION direction);
        void BT_SCO_ExtMDReadDataFromRingBuf(uint8_t *buf, uint32_t size, EXTMD_BTSCO_DIRECTION direction);
        uint8_t *BT_SCO_ExtMDGetCVSDAccuOutBuf(void);
        uint8_t *BT_SCO_ExtMDGetCVSDULWriteTmpBuf(void);
        uint8_t *BT_SCO_ExtMDGetCVSDULWriteTmpBuf2(void);
        void BT_SCO_ExtMD_ULBuf_Open(void);
        void BT_SCO_ExtMD_ULBuf_Close(void);
        void BT_SCO_ExtMD_DLBuf_Open(void);
        void BT_SCO_ExtMD_DLBuf_Close(void);
#endif
        static BTSCO_CVSD_Context *mBTSCOCVSDContext;  //btsco

        void btsco_process_RX_MSBC(void *inbuf, uint32_t *insize, void *outbuf, uint32_t *outsize, void *workbuf, const uint32_t workbufsize, uint8_t packetvalid);
        void btsco_process_TX_MSBC(void *inbuf, uint32_t *insize, void *outbuf, uint32_t *outsize, void *workbuf, const uint32_t workbufsize, uint32_t src_fs_s);
        void BT_SCO_SetMode(uint32_t mode);
        bool BT_SCO_isWideBand(void);

    private:

        AudioBTCVSDControl();
        ~AudioBTCVSDControl();
        uint32_t BT_SCO_GetMemorySize_4ByteAlign(BT_SCO_MODULE uModule);
        void BT_SCO_InitialModule(BT_SCO_MODULE uModule, uint8_t *pBuf);
        static AudioBTCVSDControl *UniqueAudioBTCVSDControl;
        uint8_t *mBTCVSDRXTempInBuf;
        uint8_t *mBTCVSDRXInBuf;
        uint8_t *mBTCVSDTXOutBuf;
#ifdef EXT_MODEM_BT_CVSD
        uint8_t *mExtMDbtscoULBuf;
        uint8_t *mExtMDbtscoULWTmpBuf;
        uint8_t *mExtMDbtscoULWTmpBuf2;
        uint8_t *mExtMDbtscoDLBuf;
        Mutex mLockUL;
        Mutex mLockDL;
        RingBuf mULRingBuf;
        RingBuf mDLRingBuf;
        AudioDigitalControlInterface *mAudioDigitalControl;
#endif
        FILE *mTXSRCPCMDumpFile;
        FILE *mBTCVSDRXDumpFile;

        uint32_t Audio_IIRHPF_GetBufferSize(int a);
        void *Audio_IIRHPF_Init(int8_t *pBuf, const int32_t *btsco_FilterCoeff_8K, int a);
        void Audio_IIRHPF_Process(void *handle, uint16_t *inbuf, uint16_t *InSample, uint16_t *outbuf, uint16_t *OutSample);
        bool BTmode;
        void btsco_AllocMemory_TX_CVSD(void);
        void btsco_AllocMemory_TX_MSBC(void);
        void btsco_AllocMemory_RX_CVSD(void);
        void btsco_AllocMemory_RX_MSBC(void);

#ifdef EXT_MODEM_BT_CVSD
        class AudioExtMDCVSDThread : public Thread
        {
            public:
                AudioExtMDCVSDThread(EXTMD_BTSCO_THREAD_TYPE Thread_type, char *RingBuffer, uint32 BufferSize);
                virtual ~AudioExtMDCVSDThread();
                virtual status_t    readyToRun();
                virtual void        onFirstRef();
                void WritePcmDumpData(uint8_t *buf, uint32_t size);
                void ClosePcmDumpFile();
                void ExtMD_btsco_cvsd_UL_Read_main();
                void ExtMD_btsco_cvsd_UL_Write_main();
                void ExtMD_btsco_cvsd_DL_Read_main();
                void   ExtMD_btsco_cvsd_DL_Write_main();

            private:
                int mFd;
                int mFd2;
                int mThreadType;
                String8 mName;
                virtual bool threadLoop();
                char *mRingBuffer;
                uint32 mBufferSize;
                //AudioMTKStreamInManager *mManager;
                unsigned char tempdata;
                uint32 mRecordDropms;

                bool mAFEDLStarting;
                bool mAFEULStarting;

                FILE *mPAdcPCMDumpFile;
                FILE *mPI2SPCMDumpFile;
                FILE *mExtMDULReadPCMDumpFile;
                FILE *mExtMDULWritePCMDumpFile;
                FILE *mExtMDDLReadPCMDumpFile;
                FILE *mExtMDDLWritePCMDumpFile;

                AudioDigitalControlInterface *mAudioDigitalControl;
                AudioBTCVSDControl *mAudioBTCVSDControl;
                AudioResourceManagerInterface *mAudioResourceManager;
#ifndef EXTMD_SUPPORT_WB
                MtkAudioSrc *pULSRCHandle;
#endif

#ifdef EXTMD_LOOPBACK_TEST
                AudioAnalogControlInterface *mAudioAnalogControl;
                AudioAfeReg *mAfeReg;
#endif
        };

        bool mExtMDBTSCORunning;
        EXTMD_BTSCO_THREAD_TYPE mThreadType;

        sp<AudioExtMDCVSDThread>  mExtMDCVSDULThread1;
        sp<AudioExtMDCVSDThread>  mExtMDCVSDULThread2;
        sp<AudioExtMDCVSDThread>  mExtMDCVSDDLThread1;
        sp<AudioExtMDCVSDThread>  mExtMDCVSDDLThread2;
#endif

        class AudioBTCVSDLoopbackRxThread : public Thread
        {
            public:
                AudioBTCVSDLoopbackRxThread(void *mAudioManager, uint32 Mem_type, char *mRingBuffer, uint32 mBufferSize);
                virtual ~AudioBTCVSDLoopbackRxThread();
                // Good place to do one-time initializations
                virtual status_t   readyToRun();
                virtual void           onFirstRef();
                void WritePcmDumpData(void *outbuf, uint32_t outsize);
                void ClosePcmDumpFile();
                void DropRecordData();
                void btsco_cvsd_RX_main();

            private:
                int mFd;
                int mFd2;
                int mMemType;
                String8 mName;
                virtual bool threadLoop();
                char *mRingBuffer;
                uint32 mBufferSize;
                unsigned char tempdata;
                uint32 mRecordDropms;
                FILE *mBTCVSDLoopbackDumpFile;
                AudioBTCVSDControl *mAudioBTCVSDControl;
        };

        AudioBTCVSDControl *mAudioBTCVSDControl;
#if defined(BTCVSD_ENC_DEC_LOOPBACK) || defined(BTCVSD_KERNEL_LOOPBACK)
        FILE *mCVSDloopbackPCMDumpFile;
#endif
        sp<AudioBTCVSDLoopbackRxThread>  mBTCVSDRxTestThread;
};
}
#endif

#endif
