#ifndef ANDROID_SPEECH_ENHANCEMENT_CONTROLLER_H
#define ANDROID_SPEECH_ENHANCEMENT_CONTROLLER_H

#include "AudioType.h"
#include "SpeechType.h"

#include "CFG_AUDIO_File.h"

namespace android
{
class SpeechEnhancementController
{
    public:
        static SpeechEnhancementController *GetInstance();

        virtual ~SpeechEnhancementController();

        /**
         * speech enhancement parameters setting
         */
        status_t SetNBSpeechParametersToAllModem(const AUDIO_CUSTOM_PARAM_STRUCT *pSphParamNB);
#if defined(MTK_DUAL_MIC_SUPPORT)
        status_t SetDualMicSpeechParametersToAllModem(const AUDIO_CUSTOM_EXTRA_PARAM_STRUCT *pSphParamDualMic);
#endif
#if defined(MTK_WB_SPEECH_SUPPORT)
        status_t SetWBSpeechParametersToAllModem(const AUDIO_CUSTOM_WB_PARAM_STRUCT *pSphParamWB);
#endif

        /**
         * speech enhancement functions on/off
         */
        sph_enh_mask_struct_t GetSpeechEnhancementMask() const { return mSpeechEnhancementMask; }
        status_t SetSpeechEnhancementMaskToAllModem(const sph_enh_mask_struct_t &mask);

        bool GetDynamicMask(const sph_enh_dynamic_mask_t dynamic_mask_type) const { return ((mSpeechEnhancementMask.dynamic_func & dynamic_mask_type) > 0); }
        status_t SetDynamicMaskOnToAllModem(const sph_enh_dynamic_mask_t dynamic_mask_type, const bool new_flag_on);


        bool GetMagicConferenceCallOn() const { return mMagicConferenceCallOn; }
        void SetMagicConferenceCallOn(const bool magic_conference_call_on);


        bool GetBtHeadsetNrecOn() const { return mBtHeadsetNrecOn; }
        void SetBtHeadsetNrecOnToAllModem(const bool bt_headset_nrec_on);

    private:
        SpeechEnhancementController();
        static SpeechEnhancementController *mSpeechEnhancementController; // singleton

        sph_enh_mask_struct_t mSpeechEnhancementMask;

        bool mMagicConferenceCallOn;
        
        bool mBtHeadsetNrecOn;
};


} // end namespace android

#endif // end of ANDROID_SPEECH_ENHANCEMENT_CONTROLLER_H
