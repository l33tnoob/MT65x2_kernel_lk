#ifndef _AUDIO_VIBSPK_CONTROL_H_
#define _AUDIO_VIBSPK_CONTROL_H_


#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>
#include <utils/threads.h>
#include <utils/String8.h>

#include <hardware_legacy/AudioHardwareBase.h>
#include <hardware_legacy/AudioSystemLegacy.h>
#include <media/AudioSystem.h>
#include <utils/threads.h>
#include <utils/KeyedVector.h>
#include <utils/Vector.h>

#include "AudioVIBSPKCoeff.h"
#include "audio_custom_exp.h"

namespace android
{
#define DELTA_FREQ  5
#define MOD_FREQ    15
#define VIB_DIGITAL_GAIN 16384
#define VIB_RAMPSTEP 16
#define VIBSPK_SPH_PARAM_SIZE 15
#define VIBSPK_AUD_PARAM_SIZE 36
#define VIBSPK_FILTER_NUM     64
#define VIBSPK_FREQ_LOWBOUND  141
#define VIBSPK_FILTER_FREQSTEP 3

#define VIBSPK_CALIBRATION_DONE    0x7777
#define VIBSPK_SETDEFAULT_VALUE    0x8888


class AudioVIBSPKVsgGen
{
public:
   static AudioVIBSPKVsgGen *getInstance();
   void freeInstance();
   uint32_t Process(uint32_t size, void *buffer, uint16_t channels, uint8_t rampcontrol, int32_t gain);
   void vsgDeInit();
   void vsgInit(int32_t samplerate, int32_t center_freq, int32_t mod_freq, int32_t delta_freq);
   uint8_t  mRampControl; //0--none, 1--rampdown, 2--rampup

private:
   AudioVIBSPKVsgGen();
   ~AudioVIBSPKVsgGen();
   int16_t  SineGen(int16_t cur_ph, int16_t ph_st);	
   int16_t  mCenter_Freq;
   int16_t  mDelta_Freq;
   int16_t  mMod_Freq;
   int32_t  mCenter_Phase;
   int16_t  mCenter_PhaseInc;
   int16_t  mCenter_PhaseStat;
   int32_t  mMod_Phase;
   int16_t  mMod_PhaseInc;
   int16_t  mMod_PhaseStat;
   uint16_t mMod_Idx;
   int16_t  mGain;
   static AudioVIBSPKVsgGen *UniqueAudioVIBSPKVsgGen;
   
};   //AudioVIBSPKVsgGen

	
class AudioVIBSPKControl
{
public:
   static AudioVIBSPKControl *getInstance();
   void freeInstance();
   void setVibSpkEnable(bool enable);
   bool getVibSpkEnable(void);
   void setParameters(int32_t rate, int32_t center_freq, int32_t mod_freq, int32_t delta_freq);
   void VibSpkProcess(uint32_t size, void *buffer, uint32_t channels);
   void VibSpkRampControl(uint8_t rampcontrol);
   void setVibSpkGain(int32_t MaxVolume, int32_t MinVolume, int32_t VolumeRange);
   int16_t getVibSpkGain(void);
private:
   AudioVIBSPKControl();
   ~AudioVIBSPKControl();
   Mutex   mMutex;
   int32_t mSampleRate;
   int32_t mCenterFreq;
   int32_t mModFreq;
   int32_t mDeltaFreq;
   int32_t mDigitalGain;
   AudioVIBSPKVsgGen *mVsg;
   bool mEnable;
   static AudioVIBSPKControl *UniqueAudioVIBSPKControl;
   uint8_t  mRampControl; //0--none, 1--rampdown, 2--rampup
};   //AudioVIBSPKControl
	
}   //namespace android

#endif   //_AUDIO_VIBSPK_CONTROL_H_