#ifndef AUDIO_MTK_STREAM_OUT_H
#define AUDIO_MTK_STREAM_OUT_H

#include <hardware_legacy/AudioHardwareInterface.h>
#include "AudioCompensationFilter.h"
#include "AudioStreamAttribute.h"
#include "AudioResourceManagerInterface.h"
#include "AudioDigitalControlFactory.h"
#include "AudioAnalogControlFactory.h"
#include "AudioDigitalType.h"
#include "AudioSpeechEnhanceInfo.h"
#include "AudioSpeechEnhLayer.h"

#include "AudioBTCVSDControl.h"
//#if defined(MTK_VIBSPK_SUPPORT)
#include "AudioCompFltCustParam.h"
#include "AudioVIBSPKControl.h"
//#endif

extern "C" {
#include  "bli_exp.h"
}

namespace android
{
class   AudioSpeechEnhanceInfo;
class   AudioMTKFilterManager;
class   AudioBTCVSDControl;

class AudioMTKStreamOut : public android_audio_legacy::AudioStreamOut
{
    public:
        AudioMTKStreamOut();
        AudioMTKStreamOut(uint32_t devices, int *format,
                          uint32_t *channels, uint32_t *sampleRate, status_t *status);

        virtual ~AudioMTKStreamOut();

        /** return audio sampling rate in hz - eg. 44100 */
        virtual uint32_t    sampleRate() const;
        /** returns size of output buffer - eg. 4800 */
        virtual size_t      bufferSize() const;

        /** returns the output channel mask */
        virtual uint32_t    channels() const;

        /**
         * return audio format in 8bit or 16bit PCM format -
         * eg. AudioSystem:PCM_16_BIT
         */
        virtual int         format() const;

        /**
         * return the frame size (number of bytes per sample).
         */
        uint32_t    frameSize() const
        {
            return popcount(channels()) * ((format() == AUDIO_FORMAT_PCM_16_BIT) ? sizeof(int16_t) : sizeof(int8_t));
        }

        /**
         * return the audio hardware driver latency in milli seconds.
         */
        virtual uint32_t    latency() const;

        /**
         * Use this method in situations where audio mixing is done in the
         * hardware. This method serves as a direct interface with hardware,
         * allowing you to directly set the volume as apposed to via the framework.
         * This method might produce multiple PCM outputs or hardware accelerated
         * codecs, such as MP3 or AAC.
         */
        virtual status_t    setVolume(float left, float right);

        /** write audio buffer to driver. Returns number of bytes written */
        virtual ssize_t     write(const void *buffer, size_t bytes);

        /**
         * Put the audio hardware output into standby mode. Returns
         * status based on include/utils/Errors.h
         */
        virtual status_t    standby();

        /** dump the state of the audio output device */
        virtual status_t dump(int fd, const Vector<String16> &args);

        // set/get audio output parameters. The function accepts a list of parameters
        // key value pairs in the form: key1=value1;key2=value2;...
        // Some keys are reserved for standard parameters (See AudioParameter class).
        // If the implementation does not accept a parameter change while the output is
        // active but the parameter is acceptable otherwise, it must return INVALID_OPERATION.
        // The audio flinger will put the output in standby and then change the parameter value.
        virtual status_t    setParameters(const String8 &keyValuePairs);
        virtual String8     getParameters(const String8 &keys);

        // return the number of audio frames written by the audio dsp to DAC since
        // the output has exited standby
        virtual status_t    getRenderPosition(uint32_t *dspFrames);

        /**
         * get the local time at which the next write to the audio driver will be
         * presented
         */
        virtual status_t    getNextWriteTimestamp(int64_t *timestamp);

        virtual status_t    setCallBack(stream_callback_t callback, void *cookie);//Add for KK

        virtual status_t    getPresentationPosition(uint64_t *frames, struct timespec *timestamp);//Add for KK

        // here to implement  mtk proprietary functions
    public:
        status_t SetSuspend(bool suspend);
        bool     GetSuspend(void);

        bool     GetStreamRunning(void);

        //can be removed to use sampleRate() & channels
        int      GetSampleRate(void);
        int      GetChannel(void);

        status_t StreamOutCompFltPreviewParameter(AudioCompFltType_t eCompFltType, void *ptr , int len);

        // these two functions can be removed
        status_t SetStreamOutCompFltStatus(AudioCompFltType_t eCompFltType, bool bEnable);
        bool     GetStreamOutCompFltStatus(AudioCompFltType_t eCompFltType);

        void     add_echo_reference(struct echo_reference_itfe *reference);
        void     remove_echo_reference(struct echo_reference_itfe *reference);

        void     SetMusicPlusStatus(bool bEnable);
        bool     GetMusicPlusStatus(void);

        status_t setForceStandby(bool bEnable);

    private:
        uint32_t calInterrupttime(void);

        status_t SetMEMIFAttribute(AudioDigitalType::Digital_Block Mem_IF, AudioStreamAttribute *Attribute);
        status_t SetMEMIFEnable(AudioDigitalType::Digital_Block Mem_IF, bool bEnable);
        status_t SetI2SOutDACAttribute();
        status_t Set2ndI2SOutAttribute();
        status_t Set2ndI2SOutAttribute(
            AudioDigtalI2S::LR_SWAP LRswap ,
            AudioDigtalI2S::I2S_SRC mode ,
            AudioDigtalI2S::INV_LRCK inverse,
            AudioDigtalI2S::I2S_FORMAT format,
            AudioDigtalI2S::I2S_WLEN Wlength,
            int samplerate);

        status_t SetI2SDACOut(bool bEnable);
        status_t Set2ndI2SOut(bool bEnable);
        status_t SetDAIBTAttribute();
        status_t SetDAIBTOut(bool bEnable);
        status_t SetAnalogFrequency(uint32 AfeDigital);
        status_t SetPlayBackPinmux();

        status_t SetStreamRunning(bool bEnable);

        status_t SetStreamOutCompFltApplyStauts(AudioCompFltType_t eCompFltType, bool bEnable);
        bool DoStereoMonoConvert(void *buffer, size_t bytes);
        status_t SetStereoToMonoFlag(int new_device);

        // MCU interrupt
        status_t SetIMcuIRQ(AudioDigitalType::IRQ_MCU_MODE IRQ_mode, AudioStreamAttribute *Attribute);
        status_t EnableIMcuIRQ(AudioDigitalType::IRQ_MCU_MODE IRQ_mode , bool bEnable);

        // power request
        status_t RequesetPlaybackclock();
        status_t ReleasePlaybackclock();

        // here decide device for turn on which didigital part
        status_t TurnOnAfeDigital(uint32 AfeDigital);
        status_t TurnOffAfeDigital(uint32 AfeDigital, bool keepDacOpen);
        bool     NeedAFEDigitalAnalogControl(uint32 DigitalPart);
        ssize_t  WriteDataToBTSCOHW(const void *buffer, size_t bytes);
        ssize_t  DoBTSCOSRC(const void *buffer, size_t bytes, void **outbuffer);
        ssize_t  DoVoIPSRC(const void *buffer, size_t bytes, void **outbuffer);

        bool     EffectMutexLock(void);
        bool     EffectMutexUnlock(void);
        int      get_playback_delay(size_t frames, struct echo_reference_buffer *buffer);
        size_t   writeDataToEchoReference(const void *buffer, size_t bytes, int remain_ms, struct timespec TimeInfo);
        void     StopWriteDataToEchoReference();
        int CalRemaintime(int buffersize);
        void     OpenPcmDumpFile(void);
        void     ClosePcmDumpFile(void);
        bool     IsStereoSpeaker();
        int      dumpPcm(FILE *fp, const void *buffer, size_t bytes);
        bool     NeedToDoBTSCOProcess(uint32 DigitalPart);
#ifdef EXTMD_LOOPBACK_TEST
        void     BTCVSD_ExtMDLoopBackTest_Init(void);
        void     BTCVSD_ExtMDLoopBackTest_Stadby(void);
#endif

        timespec GetSystemTime(bool print = 0);

    private:
        size_t WriteDataToAudioHW(const void *buffer, size_t bytes);
        void dokeyRouting(uint32_t devices);
        void UpdateLine(int line);
        class BliSrc
        {
            public:
                BliSrc();
                ~BliSrc();
                status_t initStatus();
                status_t init(uint32 inSamplerate, uint32 inChannel, uint32 OutSamplerate, uint32 OutChannel);
                size_t process(const void *inbuffer, size_t bytes, void *outbuffer);
                status_t close();
            private:
                BLI_HANDLE *mHandle;
                uint8_t *mBuffer;
                status_t mInitCheck;
                BliSrc(const BliSrc &);
                BliSrc &operator=(const BliSrc &);
        };

        AudioResourceManagerInterface *mAudioResourceManager;
        AudioDigitalControlInterface *mAudioDigitalControl;
        AudioAnalogControlInterface *mAudioAnalogControl;
        AudioStreamAttribute *mDL1Attribute;
        AudioDigtalI2S *mDL1Out;
        AudioDigtalI2S *m2ndI2SOut;
        AudioDigitalDAIBT *mDaiBt;
        AudioSpeechEnhanceInfo *mAudioSpeechEnhanceInfoInstance;

        int mFd;
        int mFd2;
        bool mStarting;
        int mSuspend;
        uint32_t mLatency;
        uint32_t mInterruptCounter;
        uint32_t mSourceSampleRate;
        uint32_t mSourceChannels;
        bool mSteroToMono;

        int DumpFileNum;
        String8 DumpFileName;
        FILE *mPDacPCMDumpFile;
        FILE *mPFinalPCMDumpFile;

        BliSrc *mBliSrc;
        uint8_t *mSwapBufferTwo;
        Mutex   mEffectLock;
        struct echo_reference_itfe *mEcho_reference;
        int mHwBufferSize;
        bool mForceStandby;

        AudioBTCVSDControl *mAudioBTCVSDControl;
        FILE *mCVSDTXoutDumpFile;

        Mutex   mSPEVoIPLock;
        BliSrc *mBliSrcVoIP;
        uint8_t *mSwapBufferVoIP;

        AudioMTKFilterManager    *mFilters;
//#if defined(MTK_VIBSPK_SUPPORT)
        AudioVIBSPKControl *mVibSpk;
        uint32_t mVibSpkFreq;
        status_t SetVibSpkDefaultParam();
        bool  mVibSpkEnable;
        size_t  DoVibSignal2DLProcess(void *outbuffer,void *vibtonebuffer, void *src2DLbuffer, size_t bytes);
        FILE *mVIBsignalDumpFile;
        FILE *mLoudNotchDumpFile;
//#endif
        uint64_t mPresentedBytes;
        timespec mPresentedTime;


};

}

#endif
