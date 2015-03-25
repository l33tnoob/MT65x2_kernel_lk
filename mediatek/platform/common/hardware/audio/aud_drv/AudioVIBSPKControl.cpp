#include "AudioVIBSPKControl.h"
#include "audio_custom_exp.h"

#include "AudioAssert.h"

#define LOG_TAG "AudioVIBSPKControl"

namespace android
{
//#if defined(MTK_VIBSPK_SUPPORT)

AudioVIBSPKControl *AudioVIBSPKControl::UniqueAudioVIBSPKControl = NULL;
AudioVIBSPKVsgGen *AudioVIBSPKVsgGen::UniqueAudioVIBSPKVsgGen = NULL;

// VSG_SIN_TAB_SIZE must be 2^n - 1
#define VSG_PHASE_BITS      14
#define VSG_PHASE_ADJUST    (15-VSG_PHASE_BITS)
#define VSG_SIN_TAB_SIZE    65
#define VSG_ODD_STAT        (VSG_SIN_TAB_SIZE-2)
#define VSG_SIN_TAB_ORDER   6
#define VSG_SIN_L_SHIFT     (VSG_PHASE_BITS-VSG_SIN_TAB_ORDER)
#define VSG_SIN_R_SHIFT     (VSG_PHASE_BITS-VSG_SIN_TAB_ORDER)
#define DEC_LEN_MAX         1152


/// define Vibration SPK Default Center Freq and RMS
#ifndef VIBSPK_MV_RMS
#define VIBSPK_MV_RMS           (350) //280~560, 70 per step
#endif

#ifndef VIBSPK_DEFAULT_FREQ
#define VIBSPK_DEFAULT_FREQ     (156) //141~330 Hz
#endif


const short vsg_sin_tab[VSG_SIN_TAB_SIZE] = 
{    0,   804,  1608,  2410,  3212,  4011,  4808,  5602,  6393,  7179,
  7962,  8739,  9512, 10278, 11039, 11793, 12539, 13279, 14010, 14732,
 15446, 16151, 16846, 17530, 18204, 18868, 19519, 20159, 20787, 21403,
 22005, 22594, 23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790, 
 27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956, 30273, 30571, 
 30852, 31113, 31356, 31580, 31785, 31971, 32137, 32285, 32412, 32521, 
 32609, 32678, 32728, 32757, 32767};


const short vsg_gain_tab[5][16] = 
{
0x08a7, 0x09b5, 0x0ae4, 0x0c39, 0x0db7, 0x0f63, 0x1144, 0x135f, 0x15bc, 0x1863, 0x1b5d, 0x1eb4, 0x2273, 0x26a7, 0x2b5e, 0x30a9, //280MVRMS
0x0ace, 0x0c20, 0x0d9a, 0x0f43, 0x1120, 0x1337, 0x158f, 0x1831, 0x1b25, 0x1e75, 0x222c, 0x2658, 0x2b05, 0x3045, 0x3629, 0x3cc5, //350MVRMS
0x0cf6, 0x0e8b, 0x1051, 0x124f, 0x148b, 0x170d, 0x19dd, 0x1d05, 0x208f, 0x2488, 0x28fd, 0x2dfe, 0x339b, 0x39e7, 0x40f7, 0x48e5, //420MVRMS
0x0f20, 0x10f8, 0x130a, 0x155d, 0x17f8, 0x1ae5, 0x1e2d, 0x21dc, 0x25fe, 0x2aa0, 0x2fd4, 0x35aa, 0x3c36, 0x438f, 0x4bce, 0x550d, //490MVRMS
0x1149, 0x1365, 0x15c3, 0x186a, 0x1b65, 0x1ebd, 0x227d, 0x26b2, 0x2b6b, 0x30b8, 0x36a9, 0x3d55, 0x44d1, 0x4d36, 0x56a2, 0x6134, //560MVRMS
};

//=============================================================================================
//                 AudioVIBSPKVsgGen Imeplementation
//=============================================================================================
AudioVIBSPKVsgGen::AudioVIBSPKVsgGen()
{
   mCenter_Freq = 0;
   ALOGD("VsgGen constructor");    
}

AudioVIBSPKVsgGen::~AudioVIBSPKVsgGen()
{
   ALOGD("VsgGen destructor");  
}

AudioVIBSPKVsgGen *AudioVIBSPKVsgGen::getInstance()
{
   if (UniqueAudioVIBSPKVsgGen == NULL) 
   {
      ALOGD("+UniqueAudioVIBSPKVsgGen");
      UniqueAudioVIBSPKVsgGen = new AudioVIBSPKVsgGen();
      ALOGD("-UniqueAudioVIBSPKVsgGen");
   }
   ALOGD("VsgGen getInstance()");
   return UniqueAudioVIBSPKVsgGen;
}

void AudioVIBSPKVsgGen::freeInstance()
{
   if (UniqueAudioVIBSPKVsgGen != NULL) 
   {
      delete UniqueAudioVIBSPKVsgGen;
   }
   ALOGD("VsgGen freeInstance()");
}

void AudioVIBSPKVsgGen::vsgInit(int32_t samplerate, int32_t center_freq, int32_t mod_freq, int32_t delta_freq)
{
   uint16_t my0;
   ALOGD("VsgGenInit");
   switch (samplerate) // my0: Q5.11
   {
        case 8000:  my0 = 0x4189; break;
        case 11025: my0 = 0x2F8E; break;
        case 12000: my0 = 0x2BB1; break;
        case 16000: my0 = 0x20C5; break;
        case 22050: my0 = 0x17C7; break;
        case 24000: my0 = 0x15D8; break;
        case 32000: my0 = 0x1062; break;
        case 44100: my0 = 0x0BE3; break;
        case 48000: my0 = 0x0AEC; break;
   }
   
   mCenter_Freq = center_freq;
   mDelta_Freq  = delta_freq;
   mMod_Freq    = mod_freq;
   
   // limitation: the generated tone is Fs/4 at most
   mCenter_Phase     = 0;
   mCenter_PhaseInc  = (int16_t)(((uint32_t)mCenter_Freq * my0 << 1) >> 12);
   mCenter_PhaseStat = 0;
   
   mMod_Phase        = 0;
   mMod_PhaseInc     = (int16_t)(((uint32_t)mMod_Freq * my0 << 1) >> 12);
   mMod_PhaseStat    = 0;
 
   my0 = 0x28BE; // 2^16/pi, Q2.14
   if(mMod_Freq == 0)
      mMod_Idx = 0;
   else
      mMod_Idx = (uint16_t)((uint32_t)mDelta_Freq * my0 / mMod_Freq);

   mRampControl = 0;
   mGain        = 0;
    
}

void AudioVIBSPKVsgGen::vsgDeInit()
{
  // mCenter_Freq = 0;
}

int16_t AudioVIBSPKVsgGen::SineGen(int16_t cur_ph, int16_t ph_st)
{
   uint16_t ft;
   int16_t lo_idx, lo_value, hi_value, lo_ph, v_diff, p_diff, temp_out;
   uint32_t temp_mr, temp_sr;
   
   ft = cur_ph << VSG_PHASE_ADJUST;
   lo_idx = cur_ph >> VSG_SIN_R_SHIFT;

   if ((ph_st & 0x1) != 0)
   {
      lo_idx = VSG_ODD_STAT - lo_idx;
      ft = 0x8000 - (cur_ph << VSG_PHASE_ADJUST);
   }

   lo_ph = lo_idx << (VSG_SIN_L_SHIFT + VSG_PHASE_ADJUST);

   lo_value = vsg_sin_tab[lo_idx];
   hi_value = vsg_sin_tab[lo_idx+1];
   
   v_diff = hi_value - lo_value;
   p_diff = ft - lo_ph;
   temp_mr = (uint32_t)v_diff * (uint32_t)p_diff * 2;
   temp_sr = temp_mr >> 10;
   temp_out = lo_value + (int16_t)temp_sr;
       
   if ((ph_st & 0x2) != 0)
   {
      temp_out = ~temp_out;
   }
   
   return temp_out;
}

uint32_t AudioVIBSPKVsgGen::Process(uint32_t size, void *buffer, uint16_t channels, uint8_t rampcontrol, int32_t gain)
{
   uint32_t I;
   int16_t mr1, ar, vsg_temp_stat, temp_out;
   int32_t outputsample, vsg_temp_phase;
   int16_t *ptr16 = (int16_t*)buffer;
   
   if(mRampControl != rampcontrol)
   {
      if(rampcontrol == 0 || rampcontrol == 1)
         mGain = gain;
      else if(rampcontrol == 2)
         mGain = 0;
      mRampControl = rampcontrol;
   }

   for( I = 0; I < size; I++ )
   {
      mMod_Phase += mMod_PhaseInc;
      
      if( mMod_Phase  > ((1 << VSG_PHASE_BITS) - 1) ) 
      {
         mMod_PhaseStat += mMod_Phase >> VSG_PHASE_BITS;
         mMod_PhaseStat &= 0x3;
         mMod_Phase &= ((1 << VSG_PHASE_BITS) - 1);
      }
      
      ar = SineGen(mMod_Phase, mMod_PhaseStat);     
      mr1 = (int16_t)((int32_t)ar * mMod_Idx >> 15);
      
      mCenter_Phase += mCenter_PhaseInc;
      if( mCenter_Phase > ((1 << VSG_PHASE_BITS) - 1)) 
      {
         mCenter_PhaseStat += mCenter_Phase >> VSG_PHASE_BITS;
         mCenter_PhaseStat &= 0x3;
         mCenter_Phase &= ((1 << VSG_PHASE_BITS) - 1);
      }
      
      vsg_temp_phase = mCenter_Phase + mr1;
      vsg_temp_stat = mCenter_PhaseStat;
      
      if (vsg_temp_phase > ((1 << VSG_PHASE_BITS) - 1) || vsg_temp_phase < 0)
      {
         vsg_temp_stat  += mCenter_PhaseStat >> VSG_PHASE_BITS;
         vsg_temp_stat  &= 0x3;
         vsg_temp_phase &= ((1 << VSG_PHASE_BITS) - 1);
      }

      temp_out = SineGen(vsg_temp_phase, vsg_temp_stat);
      outputsample = (temp_out*mGain)>>15;
	  if(mRampControl == 1 && mGain > 0)
      {
         mGain-= VIB_RAMPSTEP;
         if(mGain < 0)
            mGain = 0;
      }
      else if(mRampControl == 2 && mGain < gain)
      {
         mGain+= VIB_RAMPSTEP;
         if(mGain > gain)
            mGain = gain;
      }

      *ptr16++ = (int16_t)outputsample;
      if(channels == 2)
      {
         *ptr16++ = (int16_t)outputsample;
         I++;
      }
   }
   return I;
}

//=============================================================================================
//                 AudioVIBSPKControl Imeplementation
//=============================================================================================

AudioVIBSPKControl::AudioVIBSPKControl()
{
   mEnable      = false;
   mSampleRate  = 44100;
   mCenterFreq  = 0;
   mModFreq     = 0;
   mDeltaFreq   = 0;
   mRampControl = 0;
   mDigitalGain = 0;
   mVsg         = AudioVIBSPKVsgGen::getInstance();
   ALOGD("constructor");    
}

AudioVIBSPKControl::~AudioVIBSPKControl()
{
   ALOGD("destructor");
}

AudioVIBSPKControl *AudioVIBSPKControl::getInstance()
{
   if (UniqueAudioVIBSPKControl == NULL) {
      ALOGD("+UniqueAudioVIBSPKControl");
      UniqueAudioVIBSPKControl = new AudioVIBSPKControl();
      ASSERT(NULL!=UniqueAudioVIBSPKControl);  
      ALOGD("-UniqueAudioVIBSPKControl");
   }
   ALOGD("getInstance()");
   return UniqueAudioVIBSPKControl;
}

void AudioVIBSPKControl::freeInstance()
{
   if (UniqueAudioVIBSPKControl != NULL) 
   {
      delete UniqueAudioVIBSPKControl;
   }
    ALOGD("freeInstance()");
}

void AudioVIBSPKControl::setVibSpkEnable(bool enable)
{
   if(enable && !mEnable)
   {
      Mutex::Autolock _l(mMutex);
      mEnable = true;
   }
   else if(!enable && mEnable)
   {
      Mutex::Autolock _l(mMutex);
   	  mVsg->vsgDeInit();
      mEnable     = false;
     // mCenterFreq = 0;
   }
   ALOGD("Enable:%x", enable);
}

bool AudioVIBSPKControl::getVibSpkEnable(void)
{
   return mEnable;
}

void AudioVIBSPKControl::VibSpkRampControl(uint8_t rampcontrol)
{
   Mutex::Autolock _l(mMutex);
   mRampControl = rampcontrol;
   ALOGD("Ramp : [%d] 0--none, 1--rampdown, 2--rampup",mRampControl);
}

void AudioVIBSPKControl::setParameters(int32_t rate, int32_t center_freq, int32_t mod_freq, int32_t delta_freq)
{
   ALOGD("setParameters:%x %x %x %x", rate, center_freq, mod_freq, delta_freq);
   if( mSampleRate != rate || mCenterFreq != center_freq || mModFreq != mod_freq || mDeltaFreq != delta_freq )
   {
      Mutex::Autolock _l(mMutex);
      mSampleRate = rate;
      mCenterFreq = center_freq;
      mModFreq    = mod_freq;
      mDeltaFreq  = delta_freq;
      mVsg->vsgInit(mSampleRate, mCenterFreq, mModFreq, mDeltaFreq);
   }
}

void AudioVIBSPKControl::setVibSpkGain(int32_t MaxVolume, int32_t MinVolume, int32_t VolumeRange)
{
    int32_t mvrms, index;
    ALOGD("setVibSpkGain:%x %x %x", MaxVolume, MinVolume, VolumeRange);
    
    mvrms = VIBSPK_MV_RMS;
    if((mvrms < 280) )
        mvrms = 280;
    else if(mvrms > 560)
        mvrms = 560;
        
    index = (mvrms - 280)/70;
    mDigitalGain = vsg_gain_tab[index][VolumeRange];
}

int16_t AudioVIBSPKControl::getVibSpkGain(void)
{
    return (int16_t)mDigitalGain;
}

void AudioVIBSPKControl::VibSpkProcess(uint32_t size, void *buffer, uint32_t channels)
{
   Mutex::Autolock _l(mMutex);
   mVsg->Process(size>>1,buffer, channels, mRampControl, mDigitalGain);
}



//#endif//#if defined(MTK_VIBSPK_SUPPORT)

}   //namespace android
