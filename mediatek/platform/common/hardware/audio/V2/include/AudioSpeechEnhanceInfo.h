#ifndef AUDIO_SPEECH_ENHANCE_INFO_H
#define AUDIO_SPEECH_ENHANCE_INFO_H

#include "AudioIoctl.h"
#include "AudioUtility.h"
#include <utils/threads.h>
#include <utils/SortedVector.h>
#include <utils/KeyedVector.h>
#include <utils/TypeHelpers.h>
#include <utils/Vector.h>
#include <utils/String16.h>

#include <audio_utils/echo_reference.h>
#include "AudioMTKStreamOut.h"
#include "AudioSpeechEnhLayer.h"
#include "AudioMTKStreamInClient.h"
#include "AudioMTKStreamInManager.h"
#include "AudioMTKStreamInManagerInterface.h"

extern "C" {
#include "MtkAudioSrc.h"
}

namespace android
{
class AudioMTKStreamOut;
class AudioMTKStreamIn;
class AudioMTKStreamInManager;

enum ENUM_SPH_FIR_COEFF
{
    SPH_FIR_COEFF_NORMAL            = 0,
    SPH_FIR_COEFF_HEADSET           = 1,
    //SPH_FIR_COEFF_HANDFREE        = 2,
    SPH_FIR_COEFF_BT                = 3,
    SPH_FIR_COEFF_VOICE_REC_HEADSET  = 4,
    //SPH_FIR_COEFF_VOIP_HANDFREE   = 5,
    SPH_FIR_COEFF_HANDSET_MIC2      = 6,
    SPH_FIR_COEFF_VOICE_REC         = 7,
    SPH_FIR_COEFF_VOICE_UNLOCK      = 8,
    SPH_FIR_COEFF_VOICE_UNLOCK_MIC2   = 9,
    SPH_FIR_COEFF_CUSTOMIZATION1      = 10,
    SPH_FIR_COEFF_CUSTOMIZATION1_MIC2 = 11,
    SPH_FIR_COEFF_CUSTOMIZATION2      = 12,
    SPH_FIR_COEFF_CUSTOMIZATION2_MIC2 = 13,
    SPH_FIR_COEFF_CUSTOMIZATION3      = 14,
    SPH_FIR_COEFF_CUSTOMIZATION3_MIC2 = 15,
    SPH_FIR_COEFF_NONE              = 0xFF
};

// speech enhancement function dynamic mask
// This is the dynamic switch to decided the enhancment output.
enum voip_sph_enh_dynamic_mask_t
{
    VOIP_SPH_ENH_DYNAMIC_MASK_DMNR      = (1 << 0), // for receiver
    VOIP_SPH_ENH_DYNAMIC_MASK_VCE       = (1 << 1),
    VOIP_SPH_ENH_DYNAMIC_MASK_BWE       = (1 << 2),
    VOIP_SPH_ENH_DYNAMIC_MASK_LSPK_DMNR = (1 << 5), // for loud speaker
    VOIP_SPH_ENH_DYNAMIC_MASK_ALL       = 0xFFFFFFFF
};

typedef struct
{
    uint32_t dynamic_func; // DMNR,VCE,BWE,
} voip_sph_enh_mask_struct_t;

enum TOOL_TUNING_MODE
{
    TUNING_MODE_NONE            = 0,
    NORMAL_MODE_DMNR            = 1,
    HANDSFREE_MODE_DMNR           = 2
};


class AudioSpeechEnhanceInfo
{
    public:

        static AudioSpeechEnhanceInfo *getInstance();
        static void freeInstance();

        void SetHDRecScene(int32 HDRecScene);
        int32 GetHDRecScene(void);
        void ResetHDRecScene(void);

        void SetStreamOutPointer(void *pStreamOut);

        int GetOutputSampleRateInfo(void);
        int GetOutputChannelInfo(void);
        bool IsOutputRunning(void);

        void add_echo_reference(struct echo_reference_itfe *reference);
        void remove_echo_reference(struct echo_reference_itfe *reference);

        void SetRecordLRChannelSwitch(bool bIsLRSwitch);
        bool GetRecordLRChannelSwitch(void);

        void SetUseSpecificMIC(int32 UseSpecificMic);
        int GetUseSpecificMIC(void);

        //for HDRec tunning
        void SetHDRecTunningEnable(bool bEnable);
        bool IsHDRecTunningEnable(void);

        status_t SetHDRecVMFileName(const char *fileName);
        void GetHDRecVMFileName(char *VMFileName);

        void SetSPEPointer(AudioMTKStreamIn *pMTKStreamIn, SPELayer *pSPE);
        void ClearSPEPointer(AudioMTKStreamIn *pMTKStreamIn);
        bool IsVoIPActive(AudioMTKStreamIn *pMTKStreamIn = 0);
        bool IsInputStreamAlive(void);

        void WriteReferenceBuffer(struct InBufferInfo *Binfo);
#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT  
        void StartEchoReferenceThread(uint32_t DLdevice);
        void StopEchoReferenceThread(void);
        void CloseEchoReferenceThread(void);
        void SetEchoRefStartTime(struct timespec EchoRefStartTime);

        int get_playback_delay(size_t frames, struct echo_reference_buffer *buffer, bool bIsAccuracyTime, struct timespec TimeInfo);
#endif

        void GetDownlinkIntrStartTime(void);

        void NeedUpdateVoIPParams(void);

        void SetEnableNormalModeVoIP(bool bEnable);
        bool GetEnableNormalModeVoIP(void);

        void SetOutputStreamRunning(bool bRun);
        int GetOutputBufferSize(void);


#ifndef DMNR_TUNNING_AT_MODEMSIDE
        //for DMNR tuning in AP side
        void SetAPDMNRTuningEnable(bool bEnable);
        bool IsAPDMNRTuningEnable(void);
        bool SetAPTuningMode(const TOOL_TUNING_MODE mode);
        int GetAPTuningMode(void);
#endif

        status_t GetForceMagiASRState();
        bool SetForceMagiASR(bool enable);
        bool GetForceAECRecState();
        bool SetForceAECRec(bool enable);

        //get the MMI switch info
        bool GetDynamicSpeechEnhancementMaskOnOff(const voip_sph_enh_dynamic_mask_t dynamic_mask_type);
        void UpdateDynamicSpeechEnhancementMask(const voip_sph_enh_mask_struct_t &mask);
        status_t SetDynamicVoIPSpeechEnhancementMask(const voip_sph_enh_dynamic_mask_t dynamic_mask_type, const bool new_flag_on);
        voip_sph_enh_mask_struct_t GetDynamicVoIPSpeechEnhancementMask() const { return mVoIPSpeechEnhancementMask; }

    private:
        AudioSpeechEnhanceInfo();
        ~AudioSpeechEnhanceInfo();

        static AudioSpeechEnhanceInfo *UniqueAudioSpeechEnhanceInfoInstance;

        int32 mHdRecScene; // for HD Record
        bool mIsLRSwitch;
        int32 mUseSpecificMic;
        bool mForceMagiASR;
        bool mForceAECRec;

        Mutex mHDRInfoLock;
        AudioMTKStreamOut *mStreamOut;

        //for HDRec tunning
        bool mHDRecTunningEnable;
        char mVMFileName[128];
#ifndef DMNR_TUNNING_AT_MODEMSIDE
        bool mAPDMNRTuningEnable;
        int mAPTuningMode;
#endif

        bool mEnableNormalModeVoIP;
        KeyedVector<AudioMTKStreamIn *, SPELayer *> mSPELayerVector; // vector to save current recording client

        voip_sph_enh_mask_struct_t mVoIPSpeechEnhancementMask;

#ifdef EXTCODEC_ECHO_REFERENCE_SUPPORT
        status_t RequesetRecordclock();
        status_t ReleaseRecordclock();
        void NeedEchoRefResync(void);
        bool mEchoRefThreadCreated;
        AudioStreamAttribute mEchoRefAttribute;
        AudioMTKStreamInManager *mStreamInManager;
        AudioMTKStreamInClient *mEchoRefClient;
        AudioResourceManagerInterface *mAudioResourceManager;
        uint32_t mDLDevice;
        Mutex mInputInfoLock;

        //for android AEC
        Mutex   mEffectLock;
        struct echo_reference_itfe *mEcho_reference;
        // BLI_SRC
        MtkAudioSrc *mBliHandlerAndroidAEC;
        char       *mBliBufferAndroidAEC;
        uint32 mBliOutBufferSizeAndroidAEC;
#endif

};

}

#endif
