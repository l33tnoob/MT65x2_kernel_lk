#ifndef _AUDIO_STREAM_ATTRIBUTE_H_
#define _AUDIO_STREAM_ATTRIBUTE_H_

class AudioStreamAttribute
{
    public:

        AudioStreamAttribute();
        ~AudioStreamAttribute();
        int mFormat;
        int mDirection;
        unsigned int mSampleRate;
        unsigned int mChannels;
        unsigned int mBufferSize;
        unsigned int mInterruptSample;
        unsigned int mSource;
        unsigned int mdevices;
        unsigned int mAcoustic;
        unsigned int mPredevices;
        bool mIsDigitalMIC;
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
        bool mEchoRefUse;
#endif        
};

class AudioMEMIFAttribute
{
    public:
        enum STREAMSTATUS
        {
            STATE_FREE = -1,     // memory is not allocate
            STATE_STANDBY,  // memory allocate and ready
            STATE_EXECUTING, // stream is running
        };

        // use in modem pcm and DAI
        enum MEMIFDUPWRITE
        {
            DUP_WR_DISABLE = 0x0,
            DUP_WR_ENABLE  = 0x1
        };

        //Used when AWB and VUL and data is mono
        enum MEMIFMONOSEL
        {
            AFE_MONO_USE_L = 0x0,
            AFE_MONO_USE_R = 0x1
        };

        enum SAMPLINGRATE
        {
            AFE_8000HZ   = 0x0,
            AFE_11025HZ = 0x1,
            AFE_12000HZ = 0x2,
            AFE_16000HZ = 0x4,
            AFE_22050HZ = 0x5,
            AFE_24000HZ = 0x6,
            AFE_32000HZ = 0x8,
            AFE_44100HZ = 0x9,
            AFE_48000HZ = 0xa
        };

        enum FETCHFORMATPERSAMPLE
        {
            AFE_WLEN_16_BIT = 0,
            AFE_WLEN_32_BIT_ALIGN_8BIT_0_24BIT_DATA = 1,
            AFE_WLEN_32_BIT_ALIGN_24BIT_DATA_8BIT_0 = 3,
        };

        AudioMEMIFAttribute();
        ~AudioMEMIFAttribute();

        int mFormat;
        int mDirection;
        unsigned int mSampleRate;
        unsigned int mChannels;
        unsigned int mBufferSize;
        unsigned int mInterruptSample;
        unsigned int mMemoryInterFaceType;
        unsigned int mClockInverse;
        unsigned int m;
        unsigned int mMonoSel;
        unsigned int mdupwrite;
        unsigned int mState;
        FETCHFORMATPERSAMPLE mFetchFormatPerSample;
};

#endif
