#ifndef AUDIO_MTK_STREAM_IN_H
#define AUDIO_MTK_STREAM_IN_H

#include <hardware_legacy/AudioHardwareInterface.h>
#include "AudioStreamAttribute.h"
#include "AudioMTKStreamInClient.h"
#include "AudioMTKStreamInManager.h"
#include "AudioMTKStreamInManagerInterface.h"
#include "AudioResourceFactory.h"
#include "AudioDigitalType.h"
#include "AudioType.h"

#include "AudioSpeechEnhLayer.h"
#include "AudioSpeechEnhanceInfo.h"

#include "AudioPreProcess.h"

#define STREAM_IN_DEFAULT_BUFFER_SIZE (0x2000)

namespace android
{

class AudioMTKStreamIn : public android_audio_legacy::AudioStreamIn
{
    public:
        AudioMTKStreamIn();

        /**
         * for setting streram attribute, return NO_ERROR
         * eg. AudioSystem:PCM_16_BIT
         */
        void Set(uint32_t devices, int *format, uint32_t *channels, uint32_t *sampleRate,
                 status_t *status, android_audio_legacy::AudioSystem::audio_in_acoustics acoustics);

        virtual ~AudioMTKStreamIn();

        /** return audio sampling rate in hz - eg. 44100 */
        virtual uint32_t    sampleRate() const;

        /** return the input buffer size allowed by audio driver */
        virtual size_t      bufferSize() const;

        /** return input channel mask */
        virtual uint32_t    channels() const;

        /**
         * return audio format in 8bit or 16bit PCM format -
         * eg. AudioSystem:PCM_16_BIT
         */
        virtual int         format() const;

        /**
         * return the frame size (number of bytes per sample).
         */
        uint32_t    frameSize() const { return android_audio_legacy::AudioSystem::popCount(channels()) * ((format() == android_audio_legacy::AudioSystem::PCM_16_BIT) ? sizeof(int16_t) : sizeof(int8_t)); }

        /** set the input gain for the audio driver. This method is for
         *  for future use */
        virtual status_t    setGain(float gain);

        /** read audio buffer in from audio driver */
        virtual ssize_t     read(void *buffer, ssize_t bytes);

        /** dump the state of the audio input device */
        virtual status_t dump(int fd, const Vector<String16> &args);

        /**
         * Put the audio hardware input into standby mode. Returns
         * status based on include/utils/Errors.h
         */
        virtual status_t    standby();

        // set/get audio input parameters. The function accepts a list of parameters
        // key value pairs in the form: key1=value1;key2=value2;...
        // Some keys are reserved for standard parameters (See AudioParameter class).
        // If the implementation does not accept a parameter change while the output is
        // active but the parameter is acceptable otherwise, it must return INVALID_OPERATION.
        // The audio flinger will put the input in standby and then change the parameter value.
        virtual status_t    setParameters(const String8 &keyValuePairs);
        virtual String8     getParameters(const String8 &keys);


        // Return the amount of input frames lost in the audio driver since the last call of this function.
        // Audio driver is expected to reset the value to 0 and restart counting upon returning the current value by this function call.
        // Such loss typically occurs when the user space process is blocked longer than the capacity of audio driver buffers.
        // Unit: the number of input audio frames
        virtual unsigned int  getInputFramesLost() const;

        /**
         * get input buffer if streamin is not create, just for estimation
         */
        static size_t GetfixBufferSize(uint32_t sampleRate, int format, int channelCount);

        virtual status_t addAudioEffect(effect_handle_t effect);
        virtual status_t removeAudioEffect(effect_handle_t effect);

        // here to implemenmt dedicate function
        status_t SetSuspend(bool suspend);


        bool SetIdentity(uint32_t id);
        uint32_t GetIdentity(void);



        void NeedUpdateVoIPParams(void);
        bool GetVoIPRunningState(void);

#ifdef MTK_AUDIO_HD_REC_SUPPORT
        uint32_t BesRecordPreprocess(void *buffer , uint32_t bytes, AdditionalInfo_STRUCT AddInfo);
#endif
        void UpdateDynamicFunction();

    private:
        status_t standbyWithMode();
        // for streamin contructor check
        bool CheckFormat(int *pFormat);
        bool CheckSampleRate(uint32_t devices, uint32_t *pChannels);
        bool CheckChannel(uint32_t devices, uint32_t *pRate);
        uint32 GetBufferSizeBydevice(uint32_t devices);
        status_t SetClientSourceandMemType(AudioMTKStreamInClient *mStreamInClient);
        bool  CheckMemTypeChange(uint32_t oldDevice, uint32_t newDevice);
        uint32_t  GetMemTypeByDevice(uint32_t Device);
        uint32_t StreamInPreprocess(void *buffer , uint32_t bytes);
        status_t SetStreamInPreprocessStatus(bool Enable);
        bool CheckVoIPNeeded(void);

        void SetDMNREnable(DMNR_TYPE type, bool enable);

#ifdef MTK_VOICEUNLOCK_DEBUG_ENABLE
        bool CheckRecordNoData(short *buffer , uint32_t bytes);
        uint32_t mNoDataCount;
        int              mFd;
#endif

        void CheckDmicNeedLRSwitch(short *buffer, ssize_t bytes);

        status_t RequesetRecordclock();
        status_t ReleaseRecordclock();


        // defualt setting for Streamin
        static const int mStream_Default_Format = AUDIO_FORMAT_PCM_16_BIT;
        static const int mStream_Default_Channels = AUDIO_CHANNEL_IN_STEREO;
        static const uint32_t mStream_Default_SampleRate = 48000;   //DMIC upper bound is 32K

        // defualt setting for Streamin
        static const uint32_t Default_Mic_Buffer = 0x1000;
        static const uint32_t Default_BT_Buffer = 0x1000;

        Mutex     mLock;
        uint32_t mIdentity;
        uint32_t mLatency;
        bool mStarting;
        int mSuspend;
        int mReadCount;
        AudioStreamAttribute mAttribute;
        AudioMTKStreamInClient *mStreamInClient;
        AudioMTKStreamInManager *mStreamInManager;
        AudioResourceManagerInterface *mAudioResourceManager;
        AudioSpeechEnhanceInfo *mAudioSpeechEnhanceInfoInstance;

        static int DumpFileNum;
        String8 DumpFileName;
        FILE *mPAdcPCMDumpFile;
        FILE *mPAdcPCMInDumpFile;

        bool mEnablePreprocess;
        bool mForceMagiASREnable;
        bool mForceAECRecEnable;
        bool mUsingMagiASR;
        bool mUsingAECRec;

        bool MutexLock(void);
        bool MutexUnlock(void);
        void CheckNeedDataConvert(short *buffer, ssize_t bytes);
        void StereoToMono(short *buffer , int length);
        timespec GetSystemTime(bool print = 0);

#ifdef MTK_AUDIO_HD_REC_SUPPORT
        SPELayer    *mpSPELayer;
        int mHDRecordModeIndex;
        int mHDRecordSceneIndex;
        AUDIO_HD_RECORD_SCENE_TABLE_STRUCT mhdRecordSceneTable;
        AUDIO_HD_RECORD_PARAM_STRUCT mhdRecordParam;

        AUDIO_VOIP_PARAM_STRUCT mVOIPParam;

        AUDIO_CUSTOM_EXTRA_PARAM_STRUCT mDMNRParam;

        bool mStereoMode;
        int CheckHDRecordMode(void);
        bool GetHdRecordModeInfo(uint8_t *modeIndex);
        void LoadHDRecordParams(void);
        void ConfigHDRecordParams(SPE_MODE mode);
        void StartHDRecord(SPE_MODE mode);
        void StopHDRecord(void);
        uint32_t HDRecordPreprocess(void *buffer , uint32_t bytes, AdditionalInfo_STRUCT AddInfo);
        bool IsHDRecordRunning(void);

        void CheckHDRecordVoIPSwitch(void);
        bool IsVoIPRouteChange(void);
        int GetRoutePath(void);
        bool mVoIPRunning;
        bool mHDRecordParamUpdate;
#endif

        AudioPreProcess *mAPPS;
        struct echo_reference_itfe *mEcho_Reference;
        ssize_t NativeRecordPreprocess(void *buffer, ssize_t bytes);

#ifdef MTK_DUAL_MIC_SUPPORT
        bool    mLRChannelSwitch;
        int mSpecificMicChoose;
#endif
        bool StreamIn_NeedToSRC(void);
        bool IsNeedDMICSRC(void);
        void StreamInSRC_Init(void);
        void StreamInSRC_Process(void *buffer, size_t bytes);
        uint32 GetSrcbufvalidSize(RingBuf *SrcInputBuf);
        uint32 GetSrcbufFreeSize(RingBuf *SrcInputBuf);
        uint32 CopySrcBuf(char *buffer, uint32 *bytes, RingBuf *SrcInputBuf, uint32 *length);

        // BLI_SRC
        BLI_HANDLE *mBliHandler1;
        char       *mBliHandler1Buffer;
        BLI_HANDLE *mBliHandler2;
        char       *mBliHandler2Buffer;

        class BliSrc
        {
            public:
                BliSrc();
                ~BliSrc();
                status_t initStatus();
                status_t init(uint32 inSamplerate, uint32 inChannel, uint32 OutSamplerate, uint32 OutChannel);
                size_t process(const void *inbuffer, size_t *inBytes, void *outbuffer, size_t *outBytes);

                status_t close();
            private:
                BLI_HANDLE *mHandle;
                uint8_t *mBuffer;
                status_t mInitCheck;
                BliSrc(const BliSrc &);
                BliSrc &operator=(const BliSrc &);
        };

        BliSrc *mBliSrc;
        uint8_t *mSwapBufferTwo;

        //HDRec tunning tool
        bool mIsHDRecTunningEnable;
        bool mHDRecTunning16K;
        bool mIsAPDMNRTuningEnable;
        char m_strTunningFileName[128];
};

}

#endif
