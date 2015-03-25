#include "AudioStreamAttribute.h"
#include <utils/Log.h>

#define LOG_TAG  "AudioStreamAttribute"

AudioStreamAttribute::AudioStreamAttribute()
{
    ALOGD("AudioStreamAttribute constructor \n");
    mFormat = 0;
    mDirection = 0 ;
    mSampleRate = 0 ;
    mChannels = 0 ;
    mSource = 0 ;
    mdevices = 0 ;
    mAcoustic = 0;
    mPredevices = 0;
    mIsDigitalMIC = false;
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
    mEchoRefUse = false;
#endif

}

AudioStreamAttribute::~AudioStreamAttribute()
{
    ALOGD("AudioStreamAttribute destructor \n");
}


AudioMEMIFAttribute::AudioMEMIFAttribute()
{
    mMemoryInterFaceType = 0;
    mSampleRate = 0;
    mBufferSize = 0;
    mChannels = 0 ;
    mInterruptSample = 0;
    mState = AudioMEMIFAttribute::STATE_FREE;
    ALOGD("AudioMEMIFAttribute constructor \n");
}

AudioMEMIFAttribute::~AudioMEMIFAttribute()
{
    ALOGD("AudioMEMIFAttribute destructor \n");
}

